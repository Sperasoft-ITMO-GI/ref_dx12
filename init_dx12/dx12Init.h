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

#include "d3dx12.h"

#include <string>
#include <memory>
#include <vector>
#include <array>

#include "resources.h"
#include "uploadBuffer.h"

#include "menu_resource.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class InitDxApp {
public:
	~InitDxApp();

	bool Initialize(HINSTANCE hinstance, WNDPROC wndProc);

	void SetWindowSize(int width, int height);

	void Draw();
	void Update();

	void OnResize();

public:
	inline void AddMenuResource(MenuResource& menu_resource) {
		menu_resource.SetToUpload(dx3dDevice.Get(), commandList.Get());
		menu_resources_.push_back(menu_resource);
	}

private:
	bool InitializeWindow(HINSTANCE hinstance, WNDPROC wndProc);
	bool InitializeDx();

	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();

	void FlushCommandQueue();

	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBoxGeometry();
	void BuildPSO();

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

	HINSTANCE hInstance;
	WNDPROC wndProc;

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

	char* windowName = "Quake 2 DX12";
	char* windowClassName = "Quake 2";

	// there the all data that need to draw cube
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvHeap = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> objectConstantBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> vsByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> psByteCode = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
	std::unique_ptr<MeshGeometry> boxGeo = nullptr;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;

	DirectX::XMFLOAT4X4 mWorld = Identity4x4;
	DirectX::XMFLOAT4X4 mView =  Identity4x4;
	DirectX::XMFLOAT4X4 mProj =  Identity4x4;
	DirectX::XMFLOAT4X4 mOrtho = Identity4x4;

	//float mTheta = 1.5f * DirectX::XM_PI;
	//float mPhi = DirectX::XM_PIDIV4;
	float mRadius = 5.0f;

	// data to draw menu
private: 
	std::vector<MenuResource> menu_resources_;
	MenuResource mr;
};
