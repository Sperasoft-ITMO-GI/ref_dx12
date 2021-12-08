#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <comdef.h>

#include "d3dx12.h"

#include <string>

class DxException {
public:
	DxException() = default;
	DxException(HRESULT hr, const char* functionName, const char* filename, int lineNumber);

	char* ToString()const;

	HRESULT ErrorCode = S_OK;
	char FunctionName[256];
	char Filename[256];
	int LineNumber = -1;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    char* wfn = __FILE__;											  \
    if(FAILED(hr__)) { throw DxException(hr__, #x, wfn, __LINE__); } \
}
#endif

// Copy this functions from Luna's d3dUtil.h
// Need to create our own probably

static UINT CalcConstantBufferByteSize(UINT byteSize) {
    return (byteSize + 255) & ~255;
}

Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
	const std::wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint,
	const std::string& target
);

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer
);

static DirectX::XMFLOAT4X4 Identity4x4(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
);