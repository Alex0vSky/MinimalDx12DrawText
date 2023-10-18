// src\Tool\Uploader.h - refactor ResourceUploadBatch.h
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_3d::MinimalDx12DrawText::Tool {
// Only for D3D12_COMMAND_LIST_TYPE_DIRECT and stateBefore = D3D12_RESOURCE_STATE_COPY_DEST
struct Uploader {
	static CPtr<ID3D12Resource> uploadAndTransition(
		ID3D12Device *m_device
		, ID3D12GraphicsCommandList *m_commandList
		, ID3D12Resource* resource
		, const D3D12_SUBRESOURCE_DATA* subRes
		, ID3D12Resource* resourceTransition
		, D3D12_RESOURCE_STATES stateAfter
	) {
		uint32_t subresourceIndexStart = 0;
		uint32_t numSubresources = 1;

		const UINT64 uploadSize = GetRequiredIntermediateSize(
				resource
				, subresourceIndexStart
				, numSubresources
			);
		const CD3DX12_HEAP_PROPERTIES heapProps( D3D12_HEAP_TYPE_UPLOAD );
		auto const resDesc = CD3DX12_RESOURCE_DESC::Buffer( uploadSize );
		// Create a temporary buffer
		CPtr< ID3D12Resource > scratchResource;
		ThrowIfFailed(m_device->CreateCommittedResource(
				&heapProps
				, D3D12_HEAP_FLAG_NONE
				, &resDesc
				, D3D12_RESOURCE_STATE_GENERIC_READ
				, nullptr // D3D12_CLEAR_VALUE* pOptimizedClearValue
				, IID_PPV_ARGS( scratchResource.GetAddressOf()) )
			);
		SetDebugObjectName( scratchResource.Get(), L"ResourceUpload Temporary" );
		// Submit resource copy to command list
		UpdateSubresources( m_commandList, resource, scratchResource.Get(), 0, subresourceIndexStart, numSubresources, subRes );

		ID3D12GraphicsCommandList* commandList = m_commandList;
		D3D12_RESOURCE_BARRIER descTransitionResource = {};
		descTransitionResource.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		descTransitionResource.Transition.pResource = resourceTransition;
		descTransitionResource.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		descTransitionResource.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		descTransitionResource.Transition.StateAfter = stateAfter;
		commandList->ResourceBarrier(1, &descTransitionResource);
		return scratchResource;
	}
};
} // namespace prj_3d::MinimalDx12DrawText::Tool
