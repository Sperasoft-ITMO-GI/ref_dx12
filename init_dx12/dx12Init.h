#pragma once

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include "d3dx12.h"

#include <string>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class DxException {
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString()const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

class InitDxApp {
public:
	~InitDxApp();

	bool Initialize(HINSTANCE hinstance, WNDPROC wndProc);

	void SetWindowSize(int width, int height);

	void Draw();

private:
	bool InitializeWindow(HINSTANCE hinstance, WNDPROC wndProc);
	bool InitializeDx();

	void OnResize();

	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();

	void FlushCommandQueue();

	inline D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const {
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			rtvHeap->GetCPUDescriptorHandleForHeapStart(),
			currentBackBuffer,
			RtvDescriptorSize
		);
	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const {
		return dsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	inline ID3D12Resource* CurrentBackBuffer() const {
		return swapChainBuffer[currentBackBuffer].Get();
	}

private:
	HWND mainWindow;

	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> dx3dDevice;
	Microsoft::WRL::ComPtr<ID3D12Fence> dx3dFence;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandListAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	
	int currentBackBuffer = 0;
	const int swapChainBufferCount = 2;
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainBuffer[2];
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;

	bool msaaState = false;

	D3D12_VIEWPORT viewPort;
	D3D12_RECT scissorRect;

	UINT RtvDescriptorSize;
	UINT DsvDescriptorSize;
	UINT CbvSrvUavDescriptorSize;
	UINT msaaQuality;

	UINT64 currentFence;

	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	int clientWidth;
	int clientHeight;

	std::wstring windowName = L"Quake 2 Dx12";
};
