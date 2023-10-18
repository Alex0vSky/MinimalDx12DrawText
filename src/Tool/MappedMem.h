// src\Tool\MappedMem.h - refactor GraphicsMemory.h/*.cpp and LinearAllocator.h/*.cpp
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_3d::MinimalDx12DrawText::Tool {
class MappedMem {
    static constexpr size_t MinPageSize = 64 * 1024;
	std::vector< CPtr< ID3D12Resource > > m_holder;

 public:
	struct Page {
		void *m_cpuAddress;
		D3D12_GPU_VIRTUAL_ADDRESS m_gpuAddress;
	};
	// Allocate the upload heap
	Page getNewPage(ID3D12Device* device, const wchar_t *name = nullptr) {
		const CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer( MinPageSize );
		CPtr< ID3D12Resource > spResource;
		ThrowIfFailed( device ->CreateCommittedResource(
			&uploadHeapProperties
			, D3D12_HEAP_FLAG_NONE
			, &bufferDesc
			, D3D12_RESOURCE_STATE_GENERIC_READ
			, nullptr
			, IID_PPV_ARGS(spResource.ReleaseAndGetAddressOf( ))));
		m_holder.push_back( spResource );
		if ( name ) Tool::SetDebugObjectName( spResource.Get( ), name );
		// Get a pointer to the memory
		Page page = { };
		// Auto-unmap after Release
		ThrowIfFailed( spResource->Map( 0, nullptr, &page.m_cpuAddress ) );
		page.m_gpuAddress = spResource ->GetGPUVirtualAddress( );
		return page;
	}
};
} // namespace prj_3d::MinimalDx12DrawText::Tool