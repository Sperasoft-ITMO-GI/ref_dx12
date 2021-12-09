#pragma once

#include "utils.h"

#include <array>

void SetBufferResourcesToUploadHeap(Microsoft::WRL::ComPtr<ID3D12Device>& device,
						            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
	                                Microsoft::WRL::ComPtr<ID3D12Resource>& resource,
									const void* data, size_t elementsCount, size_t elementsSize, D3D12_RESOURCE_FLAGS flags);

struct MenuVertex {
	DirectX::XMFLOAT2 pos;
	DirectX::XMFLOAT3 color;
};

class MenuResource {
public:
	MenuResource() = default;
	MenuResource(const MenuResource& resource);
	MenuResource(MenuResource&& resource);

	void SetToUpload(Microsoft::WRL::ComPtr<ID3D12Device> device, 
		             Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, 
		             D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	void SetVertexes(float x, float y, float w, float h);
	void SetColor(float c0, float c1, float c2);

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();

	inline size_t GetIndexCount() {
		return kIndexesCount;
	}

private:
	static constexpr size_t kElementsCount = 4;
	static constexpr size_t kElementSize = sizeof(MenuVertex);
	static constexpr size_t kIndexesCount = 6;
	static constexpr size_t kIndexSize = sizeof(size_t);
	static constexpr std::array<size_t, kIndexesCount> indexes_ = {
		0, 1, 2,
		0, 2, 3
	};
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexes_resource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexes_resource_ = nullptr;
	std::array<MenuVertex, kElementsCount> vertexes_;
};