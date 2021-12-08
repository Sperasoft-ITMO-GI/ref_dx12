#include "Utils.h"

using Microsoft::WRL::ComPtr;

DxException::DxException(HRESULT hr, const char* functionName, const char* filename, int lineNumber) :
    ErrorCode(hr),
    LineNumber(lineNumber)
{
    memset(FunctionName, 0, 256);
    memset(Filename, 0, 256);
    strcpy(FunctionName, functionName);
    strcpy(Filename, filename);
}

char* DxException::ToString() const {
    // Get the string description of the error code.
    _com_error err(ErrorCode);
    char msg[256];
    memset(msg, 0, 256);
    strcpy(msg, err.ErrorMessage());

    char lineNumberToChar[16];
    _itoa(LineNumber, lineNumberToChar, 10);

    char result[1024];
    memset(result, 0, 1024);
    strcpy(result, FunctionName);
    strcat(result, " failed in ");
    strcat(result, Filename);
    strcat(result, "; line ");
    strcat(result, lineNumberToChar);
    strcat(result, "; error: ");
    strcat(result, msg);

    return result;
}

ComPtr<ID3DBlob> CompileShader(
    const wchar_t* filename,
    const D3D_SHADER_MACRO* defines,
    const char* entrypoint,
    const char* target)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors;

	hr = D3DCompileFromFile(filename, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint, target, compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
		printf("[DX12][ERROR]:%s", (char*)errors->GetBufferPointer());

	ThrowIfFailed(hr);

	return byteCode;
}

ComPtr<ID3D12Resource> CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
    ComPtr<ID3D12Resource> defaultBuffer;

    // Create the actual default buffer resource.
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

    // In order to copy CPU memory data into our default buffer, we need to create
    // an intermediate upload heap. 
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


    // Describe the data we want to copy into the default buffer.
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
    // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
    // the intermediate upload heap data will be copied to mBuffer.
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

    // Note: uploadBuffer has to be kept alive after the above function calls because
    // the command list has not been executed yet that performs the actual copy.
    // The caller can Release the uploadBuffer after it knows the copy has been executed.


    return defaultBuffer;
}

