#include "menu_resource.h"

using Microsoft::WRL::ComPtr;

MenuResource::MenuResource(const MenuResource& resource) {
	resource.vertexes_resource_.CopyTo(&vertexes_resource_);
	resource.indexes_resource_.CopyTo(&indexes_resource_);
	std::copy(resource.vertexes_.begin(), resource.vertexes_.end(), vertexes_.begin());
}

MenuResource::MenuResource(MenuResource&& resource) {
	vertexes_resource_.Swap(resource.vertexes_resource_);
	indexes_resource_.Swap(resource.indexes_resource_);
	vertexes_.swap(resource.vertexes_);
}

void MenuResource::SetToUpload(ID3D12Device* device,
	                           ID3D12GraphicsCommandList* commandList, 
							   D3D12_RESOURCE_FLAGS flags) {

	SetBufferResourcesToUploadHeap(device, commandList, vertexes_resource_, vertexes_.data(), kElementsCount, kElementSize, flags);
	SetBufferResourcesToUploadHeap(device, commandList, indexes_resource_, indexes_.data(), kIndexesCount, kIndexSize, flags);
}

void MenuResource::SetVertexes(float x, float y, float w, float h) {
	using DirectX::XMFLOAT3;
	vertexes_[0].pos = XMFLOAT3(x, y, 2.0f);
	vertexes_[1].pos = XMFLOAT3(x + w, y, 2.0f);
	vertexes_[2].pos = XMFLOAT3(x + w, y + h, 2.0f);
	vertexes_[3].pos = XMFLOAT3(x, y + h, 2.0f);
}

void MenuResource::SetColor(float c0, float c1, float c2) {
	using DirectX::XMFLOAT4;
	for (auto vert : vertexes_) {
		vert.color = XMFLOAT4(c0, c1, c2, 1.0f);
	}
}

D3D12_VERTEX_BUFFER_VIEW MenuResource::GetVertexBufferView() {
	D3D12_VERTEX_BUFFER_VIEW view;
	view.BufferLocation = vertexes_resource_->GetGPUVirtualAddress();
	view.SizeInBytes = kElementsCount * kElementSize;
	view.StrideInBytes = sizeof(MenuVertex);

	return view;
}

D3D12_INDEX_BUFFER_VIEW MenuResource::GetIndexBufferView() {
	D3D12_INDEX_BUFFER_VIEW view;
	view.BufferLocation = indexes_resource_->GetGPUVirtualAddress();
	view.Format = DXGI_FORMAT_R16_UINT;
	view.SizeInBytes = kIndexesCount * kIndexSize;

	return view;
}

void SetBufferResourcesToUploadHeap(ID3D12Device* device, 
	                                ID3D12GraphicsCommandList* commandList, 
	                                Microsoft::WRL::ComPtr<ID3D12Resource>& resource, 
	                                const void* data, size_t elementsCount, size_t elementsSize, 
				                    D3D12_RESOURCE_FLAGS flags) {

	const auto buffer_size = elementsCount * elementsSize;

	ThrowIfFailed(
		device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(buffer_size),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(resource.GetAddressOf())
		)
	);

	ComPtr<ID3D12Resource> intermediate_resource;
	ThrowIfFailed(
		device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(buffer_size),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(intermediate_resource.GetAddressOf())
		)
	);

	D3D12_SUBRESOURCE_DATA subresource_data = {};
	subresource_data.pData = data;
	subresource_data.RowPitch = buffer_size;
	subresource_data.SlicePitch = subresource_data.RowPitch;

	commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			resource.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COPY_DEST
		)
	);

	UpdateSubresources(
		commandList,
		intermediate_resource.Get(),
		resource.Get(),
		0, 0, 1,
		&subresource_data
	);

	commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			resource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_GENERIC_READ
		)
	);
}
