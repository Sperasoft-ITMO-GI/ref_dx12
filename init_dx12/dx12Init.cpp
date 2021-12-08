#include "dx12Init.h"

using Microsoft::WRL::ComPtr;

InitDxApp::~InitDxApp() {
	if (dx3dDevice != nullptr) {
		FlushCommandQueue();
		if (mainWindow)
		{
			DestroyWindow(mainWindow);
			mainWindow = NULL;
		}

		UnregisterClass(windowClassName, hInstance);
		printf("%s", "[DX12]: Window destroyed\n");
	}
	printf("%s", "[DX12]: Render destoyed");
}

bool InitDxApp::Initialize(HINSTANCE hinstance, WNDPROC wndProc) {
	if (!InitializeWindow(hinstance, wndProc)) {
		return false;
	}

	if (!InitializeDx()) {
		return false;
	}

	OnResize();

	ThrowIfFailed(commandList->Reset(commandListAllocator.Get(), nullptr));

	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();

	// Execute the initialization commands.
	ThrowIfFailed(commandList->Close());
	ID3D12CommandList* cmdsLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}

void InitDxApp::SetWindowSize(int width, int height) {
	clientWidth = width;
	clientHeight = height;
}

void InitDxApp::Draw() {
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(commandListAllocator->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(commandList->Reset(commandListAllocator.Get(), pso.Get()));

	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	commandList->RSSetViewports(1, &viewPort);
	commandList->RSSetScissorRects(1, &scissorRect);
	
	// Indicate a state transition on the resource usage.
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	commandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightBlue, 0, nullptr);
	commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { cbvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	commandList->SetGraphicsRootSignature(rootSignature.Get());

	commandList->IASetVertexBuffers(0, 1, &boxGeo->VertexBufferView());
	commandList->IASetIndexBuffer(&boxGeo->IndexBufferView());
	commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootDescriptorTable(0, cbvHeap->GetGPUDescriptorHandleForHeapStart());

	commandList->DrawIndexedInstanced(boxGeo->DrawArgs["box"].IndexCount, 1, 0, 0, 0);

	// Indicate a state transition on the resource usage.
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(commandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(swapChain->Present(0, 0));
	currentBackBuffer = (currentBackBuffer + 1) % 2;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
}

bool InitDxApp::InitializeWindow(HINSTANCE hinstance_, WNDPROC wndProc_) {
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = wndProc_;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance_;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = windowClassName;

	if (!RegisterClass(&wc))
	{
		MessageBox(0, "RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, clientWidth, clientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mainWindow = CreateWindowEx(0, windowClassName, windowName,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hinstance_, 0);
	if (!mainWindow)
	{
		MessageBox(0, "CreateWindow Failed.", 0, 0);
		return false;
	}

	this->hInstance = hinstance_;
	this->wndProc = wndProc_;

	ShowWindow(mainWindow, SW_SHOW);
	UpdateWindow(mainWindow);

	printf("%s", "[DX12]: Window created\n");

	return true;
}

bool InitDxApp::InitializeDx() {
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&dx3dDevice)
	);

	if (FAILED(hardwareResult)) {
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
				pWarpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&dx3dDevice)
			)
		);
	}

	ThrowIfFailed(dx3dDevice->CreateFence(
			0, D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&dx3dFence)
		)
	);

	RtvDescriptorSize = dx3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DsvDescriptorSize = dx3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CbvSrvUavDescriptorSize = dx3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = backBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(dx3dDevice->CheckFeatureSupport(
			D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&msQualityLevels,
			sizeof(msQualityLevels)
		)
	);

	msaaQuality = msQualityLevels.NumQualityLevels;
	assert(msaaQuality > 0 && "Unexpected MSAA quality level.");

	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();

	printf("%s", "[DX12]: DirectX 12 initialized\n");

	return true;
}

void InitDxApp::OnResize() {
	assert(dx3dDevice);
	assert(swapChain);
	assert(commandListAllocator);

	// Flush before changing any resources.
	FlushCommandQueue();

	ThrowIfFailed(commandList->Reset(commandListAllocator.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < swapChainBufferCount; ++i)
		swapChainBuffer[i].Reset();
	depthStencilBuffer.Reset();

	// Resize the swap chain.
	ThrowIfFailed(swapChain->ResizeBuffers(
		swapChainBufferCount,
		clientWidth, clientHeight,
		backBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	currentBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < swapChainBufferCount; i++)
	{
		ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainBuffer[i])));
		dx3dDevice->CreateRenderTargetView(swapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, RtvDescriptorSize);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = clientWidth;
	depthStencilDesc.Height = clientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	depthStencilDesc.SampleDesc.Count = msaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = msaaState ? (msaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = depthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(dx3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(depthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = depthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	dx3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Execute the resize commands.
	ThrowIfFailed(commandList->Close());
	ID3D12CommandList* cmdsLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = static_cast<float>(clientWidth);
	viewPort.Height = static_cast<float>(clientHeight);
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	scissorRect = { 0, 0, clientWidth, clientHeight };
}

void InitDxApp::Update() {
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	using DirectX::XMVECTOR;
	using DirectX::XMMATRIX;

	// Build the view matrix.
	XMVECTOR pos = DirectX::XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = DirectX::XMVectorZero();
	XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	objectConstantBuffer->CopyData(0, objConstants);
}

void InitDxApp::CreateCommandObjects() {
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(dx3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

	ThrowIfFailed(dx3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(commandListAllocator.GetAddressOf())));

	ThrowIfFailed(dx3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandListAllocator.Get(), // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(commandList.GetAddressOf())));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	commandList->Close();

	printf("%s", "[DX12]: Command objects created\n");
}

void InitDxApp::CreateSwapChain() {
	// Release the previous swapchain we will be recreating.
	swapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = clientWidth;
	sd.BufferDesc.Height = clientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = backBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = msaaState ? 4 : 1;
	sd.SampleDesc.Quality = msaaState ? (msaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2; // swapchain buffer count
	sd.OutputWindow = mainWindow;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(dxgiFactory->CreateSwapChain(
		commandQueue.Get(),
		&sd,
		swapChain.GetAddressOf()));

	printf("%s", "[DX12]: SwapChain created\n");
}

void InitDxApp::CreateRtvAndDsvDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = 2;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(dx3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.GetAddressOf())));


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(dx3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(dsvHeap.GetAddressOf())));
}

void InitDxApp::FlushCommandQueue() {
	// Advance the fence value to mark commands up to this fence point.
	currentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(commandQueue->Signal(dx3dFence.Get(), currentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (dx3dFence->GetCompletedValue() < currentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		ThrowIfFailed(dx3dFence->SetEventOnCompletion(currentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void InitDxApp::BuildDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(dx3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap)));

	printf("%s", "[DX12]: Descriptor heap (CBV_SRV_UAV) build\n");
}

void InitDxApp::BuildConstantBuffers() {
	objectConstantBuffer = std::make_unique<UploadBuffer<ObjectConstants>>(dx3dDevice.Get(), 1, true);
	
	UINT cbByteSize = CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectConstantBuffer->Resource()->GetGPUVirtualAddress();

	int boxCBIndex = 0;
	cbAddress += boxCBIndex * cbByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = cbByteSize;

	dx3dDevice->CreateConstantBufferView(&cbvDesc, cbvHeap->GetCPUDescriptorHandleForHeapStart());

	printf("%s", "[DX12]: Constant buffer view created\n");
}

void InitDxApp::BuildRootSignature() {
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr) {
		printf("%s", errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(dx3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature)
	));

	printf("%s", "[DX12]: Root Signature to cbv created\n");
}

void InitDxApp::BuildShadersAndInputLayout() {
	HRESULT hr = S_OK;

	vsByteCode = CompileShader(L"D:/Projects/projects_C_and_C++/Computer_Graphics/SperasoftITMOGI/Quake-2/ref_dx12/init_dx12/shaders/color.hlsl", nullptr, "VS", "vs_5_0");
	psByteCode = CompileShader(L"D:/Projects/projects_C_and_C++/Computer_Graphics/SperasoftITMOGI/Quake-2/ref_dx12/init_dx12/shaders/color.hlsl", nullptr, "PS", "ps_5_0");

	inputLayout = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	printf("%s", "[DX12]: Vertex and Pixel shaders build\n");
}

void InitDxApp::BuildBoxGeometry() {
	using DirectX::XMFLOAT3;
	using DirectX::XMFLOAT4;
	using namespace DirectX::Colors;

	std::array<Vertex, 8> vertices = {
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(White) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Red) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Blue) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Yellow) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Cyan) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Magenta) })
	};

	std::array<std::uint16_t, 36> indices = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(uint16_t);

	boxGeo = std::make_unique <MeshGeometry>();
	boxGeo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &boxGeo->VertexBufferCPU));
	CopyMemory(boxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &boxGeo->IndexBufferCPU));
	CopyMemory(boxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	boxGeo->VertexBufferGPU = CreateDefaultBuffer(dx3dDevice.Get(), commandList.Get(), vertices.data(), vbByteSize, boxGeo->VertexBufferUploader);
	boxGeo->IndexBufferGPU = CreateDefaultBuffer(dx3dDevice.Get(), commandList.Get(), indices.data(), ibByteSize, boxGeo->IndexBufferUploader);

	boxGeo->VertexByteStride = sizeof(Vertex);
	boxGeo->VertexBufferByteSize = vbByteSize;
	boxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	boxGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	boxGeo->DrawArgs["box"] = submesh;

	printf("%s", "[DX12]: Box geometry build\n");
}

void InitDxApp::BuildPSO() {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(vsByteCode->GetBufferPointer()),
		vsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(psByteCode->GetBufferPointer()),
		psByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = backBufferFormat;
	psoDesc.SampleDesc.Count = msaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = msaaState ? (msaaQuality - 1) : 0;
	psoDesc.DSVFormat = depthStencilFormat;
	ThrowIfFailed(dx3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));

	printf("%s", "[DX12]: Pipeline state object build\n");
}




