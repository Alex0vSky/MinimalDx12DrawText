// src\plain_fontWorks.h - refactor ResourceUploadBatch.h
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_3d::MinimalDx12DrawText::Plain {
	Elem::Context fontWorks(ID3D12Device* device, ID3D12CommandQueue* CommandQueue) {
		Tool::Hr hr;

#pragma region Spec::D12::DescriptorHeap
		CPtr< ID3D12DescriptorHeap > m_pHeap;
		D3D12_GPU_DESCRIPTOR_HANDLE handleGpu;
		D3D12_CPU_DESCRIPTOR_HANDLE handleCpu;
		{
		static const UINT FontDescriptorNum = 0;
		static const UINT FountsCount = 1;

		D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		D3D12_DESCRIPTOR_HEAP_DESC m_desc = {};
		m_desc.Flags = flags;
		m_desc.NumDescriptors = FountsCount;
		m_desc.Type = type;

		const D3D12_DESCRIPTOR_HEAP_DESC* pDesc = &m_desc;
		uint32_t m_increment;
		m_increment = device ->GetDescriptorHandleIncrementSize( pDesc->Type );

		hr = device ->CreateDescriptorHeap(
			pDesc,
			IID_PPV_ARGS( m_pHeap.ReleaseAndGetAddressOf( ) ) );
		Tool::SetDebugObjectName(m_pHeap.Get(), L"DescriptorHeap");

		D3D12_CPU_DESCRIPTOR_HANDLE m_hCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE m_hGPU;
		m_hCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart();
		if ( pDesc->Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE )
			m_hGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart();

//    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(_In_ size_t index) const {
		handleGpu.ptr = m_hGPU.ptr + UINT64( FontDescriptorNum ) * UINT64(m_increment);

//		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(_In_ size_t index) const {
		handleCpu.ptr = static_cast<SIZE_T>(m_hCPU.ptr + UINT64( FontDescriptorNum ) * UINT64(m_increment));
		}
#pragma endregion

		CPtr< ID3D12GraphicsCommandList > mList;
		D3D12_COMMAND_LIST_TYPE commandType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		{
	    CPtr<ID3D12CommandAllocator> mCmdAlloc;
        hr = device->CreateCommandAllocator( commandType, IID_PPV_ARGS( 
			mCmdAlloc.ReleaseAndGetAddressOf()));
        Tool::SetDebugObjectName(mCmdAlloc.Get(), L"ResourceUploadBatch");

        hr = device->CreateCommandList(1, commandType, mCmdAlloc.Get(), nullptr, IID_PPV_ARGS(
			mList.ReleaseAndGetAddressOf()));
        Tool::SetDebugObjectName(mList.Get(), L"ResourceUploadBatch");
		}

		Elem::Glyph::uptr_t puGlyph;
#pragma region SpriteFont::Impl
		const auto &arrayFont = Elem::DxtkFont::CompiledToBinary::getArial28( );

		Tool::BinaryReader reader( (uint8_t const* )arrayFont, sizeof( arrayFont ) );
		{

// inner	pImpl = std::make_unique<Impl>(device, upload, &reader, cpuDescriptorDest, gpuDescriptorDest, forceSRGB);
		static const char spriteFontMagic[] = "DXTKfont";

		// Validate the header.
		for (char const* magic = spriteFontMagic; *magic; magic++)
			if (reader.Read<uint8_t>() != *magic)
				throw std::runtime_error("Not a MakeSpriteFont output binary");
		// Read the glyph data.
		auto glyphCount = reader.Read<uint32_t>();
		auto glyphData = reader.ReadArray< Elem::Glyph::Character >(glyphCount);
		// Read font properties.
		float lineSpacing = reader.Read<float>();

		wchar_t character = static_cast<wchar_t>( reader.Read<uint32_t>() );
		puGlyph = std::make_unique< Elem::Glyph >( 
				glyphData, glyphCount, character, lineSpacing
			);
		}

		// Read the texture data.
		auto textureWidth = reader.Read<uint32_t>();
		auto textureHeight = reader.Read<uint32_t>();
		auto textureFormat = reader.Read<DXGI_FORMAT>();
		auto textureStride = reader.Read<uint32_t>();
		auto textureRows = reader.Read<uint32_t>();

		const uint64_t dataSize = uint64_t(textureStride) * uint64_t(textureRows);
		if (dataSize > UINT32_MAX)
			throw std::overflow_error("Invalid .spritefont file");

		auto textureData = reader.ReadArray<uint8_t>(static_cast<size_t>(dataSize));

#pragma region CreateTextureResource
		CPtr< ID3D12Resource > textureResource;
		CPtr< ID3D12Resource > uploadedTexture;
		constexpr D3D12_RESOURCE_STATES c_initialCopyTargetState = D3D12_RESOURCE_STATE_COMMON;
		{
		// Create the D3D texture (D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		uint32_t stride, rows;
		stride = textureStride;
		rows = textureRows;
		uint32_t width, height;
		width = textureWidth;
		height = textureHeight;
		DXGI_FORMAT format = textureFormat;
		const uint8_t* data = textureData;
		const uint64_t sliceBytes = uint64_t(stride) * uint64_t(rows);
		if (sliceBytes > UINT32_MAX)
			throw std::overflow_error("Invalid .spritefont file");
		D3D12_RESOURCE_DESC descSpriteFontTexture = {};
		descSpriteFontTexture.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		descSpriteFontTexture.Width = width;
		descSpriteFontTexture.Height = height;
		descSpriteFontTexture.DepthOrArraySize = 1;
		descSpriteFontTexture.MipLevels = 1;
		descSpriteFontTexture.Format = format;
		descSpriteFontTexture.SampleDesc.Count = 1;

		const CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		hr = device->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&descSpriteFontTexture,
			c_initialCopyTargetState,
			nullptr,
			IID_PPV_ARGS( textureResource.ReleaseAndGetAddressOf( ) ) );
		Tool::SetDebugObjectName( textureResource.Get(), L"SpriteFont:Texture" );

		D3D12_SUBRESOURCE_DATA initData = { data, static_cast<LONG_PTR>(stride), static_cast<LONG_PTR>(sliceBytes) };
		// Transition a resource once you're done with it
		uploadedTexture = Tool::Uploader::uploadAndTransition( 
				device
				, mList.Get( )
				, textureResource.Get( )
				, &initData
				, textureResource.Get( )
				, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE 
			);
		}
#pragma endregion // CreateTextureResource

		// Create the shader resource view
		{
		const auto desc = textureResource ->GetDesc( );
		if ( (desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) != 0 )
			throw std::runtime_error("Can't have D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE");
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		const UINT mipLevels = (desc.MipLevels) ? static_cast<UINT>(desc.MipLevels) : static_cast<UINT>(-1);
		if ( D3D12_RESOURCE_DIMENSION_TEXTURE2D != desc.Dimension )
			throw std::invalid_argument("unknown resource dimension");
		if (desc.DepthOrArraySize > 1) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.MipLevels = mipLevels;
			srvDesc.Texture2DArray.ArraySize = static_cast<UINT>(desc.DepthOrArraySize);
		} else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = mipLevels;
		}
		device ->CreateShaderResourceView( textureResource.Get( ), &srvDesc, handleCpu );
		}

		// Save off the GPU descriptor pointer and size.
		XMUINT2 textureSize = XMUINT2( textureWidth, textureHeight );
#pragma endregion // SpriteFont::Impl

        CPtr< ID3D12Resource > indexBuffer;
		CPtr< ID3D12Resource > uploadedIndexBuffer;
		CD3DX12_RESOURCE_DESC bufferDesc;
		{
//		m_spriteBatch = std::make_unique< Spec::D12::SpriteBatch >( device, resourceUpload, pd_ );
		static_assert((Const::MaxBatchSize * Const::VerticesPerSprite) < USHRT_MAX, "MaxBatchSize too large for 16-bit Indices");
		const CD3DX12_HEAP_PROPERTIES heapPropsSpriteBatch(D3D12_HEAP_TYPE_DEFAULT);
		bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(short) * Const::MaxBatchSize * Const::IndicesPerSprite);
		// Create the index buffer.
		hr = device->CreateCommittedResource(
			&heapPropsSpriteBatch,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			c_initialCopyTargetState,
			nullptr,
			IID_PPV_ARGS(indexBuffer.ReleaseAndGetAddressOf()));
		Tool::SetDebugObjectName(indexBuffer.Get(), L"SpriteBatch");

//		std::vector<short> DeviceResources::CreateIndexValues()
		std::vector<short> indexValues;
		indexValues.reserve(Const::MaxBatchSize * Const::IndicesPerSprite);
		for (size_t j = 0; j < Const::MaxBatchSize * Const::VerticesPerSprite; j += Const::VerticesPerSprite) {
			auto const i = static_cast<short>(j);
			indexValues.push_back(i);
			indexValues.push_back(i + 1);
			indexValues.push_back(i + 2);
			indexValues.push_back(i + 1);
			indexValues.push_back(i + 3);
			indexValues.push_back(i + 2);
		}
		D3D12_SUBRESOURCE_DATA indexDataDesc = {};
		indexDataDesc.pData = indexValues.data();
		indexDataDesc.RowPitch = static_cast<LONG_PTR>(bufferDesc.Width);
		indexDataDesc.SlicePitch = indexDataDesc.RowPitch;

		// Upload the resource
		uploadedIndexBuffer = Tool::Uploader::uploadAndTransition( 
				device
				, mList.Get( )
				, indexBuffer.Get( )
				, &indexDataDesc 
				, indexBuffer.Get( )
				, D3D12_RESOURCE_STATE_INDEX_BUFFER 
			);
		Tool::SetDebugObjectName( indexBuffer.Get(), L"DirectXTK:SpriteBatch Index Buffer" );
		}
        D3D12_INDEX_BUFFER_VIEW indexBufferView;
		// Create the index buffer view
		indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		indexBufferView.SizeInBytes = static_cast<UINT>(bufferDesc.Width);

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

		const CD3DX12_DESCRIPTOR_RANGE textureSRV(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        CPtr< ID3D12RootSignature > rootSignatureStatic;
		{
			// Same as CommonStates::StaticLinearClamp
			const CD3DX12_STATIC_SAMPLER_DESC sampler(
				0, // register
				D3D12_FILTER_MIN_MAG_MIP_LINEAR,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				0.f,
				16,
				D3D12_COMPARISON_FUNC_LESS_EQUAL,
				D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
				0.f,
				D3D12_FLOAT32_MAX,
				D3D12_SHADER_VISIBILITY_PIXEL);

			enum RootParameterIndex
			{
				TextureSRV,
				ConstantBuffer,
				TextureSampler,
				RootParameterCount
			};
			CD3DX12_ROOT_PARAMETER rootParameters[RootParameterIndex::RootParameterCount - 1] = {};
			rootParameters[RootParameterIndex::TextureSRV].InitAsDescriptorTable(1, &textureSRV, D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[RootParameterIndex::ConstantBuffer].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

			CD3DX12_ROOT_SIGNATURE_DESC rsigDesc;
			rsigDesc.Init(static_cast<UINT>(std::size(rootParameters)), rootParameters, 1, &sampler, rootSignatureFlags);

			CPtr< ID3DBlob > pSignature;
			CPtr< ID3DBlob > pError;
			hr = ::D3D12SerializeRootSignature( 
					&rsigDesc
					, D3D_ROOT_SIGNATURE_VERSION_1
					, pSignature.GetAddressOf( )
					, pError.GetAddressOf( )
				);
			hr = device ->CreateRootSignature(
					0
					, pSignature->GetBufferPointer( )
					, pSignature->GetBufferSize( )
					, IID_PPV_ARGS( rootSignatureStatic.ReleaseAndGetAddressOf( ) )
				);
			Tool::SetDebugObjectName( rootSignatureStatic.Get( ), L"SpriteBatch" );
		}

		CPtr<ID3D12PipelineState> mPSO;
		{
		Elem::PipelineStateDescription psoDesc( { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM } );
		const D3D12_INPUT_LAYOUT_DESC s_DefaultInputLayoutDesc = Elem::VertexPositionColorTexture::InputLayout;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dDesc = {};
		d3dDesc.InputLayout = s_DefaultInputLayoutDesc;
		d3dDesc.BlendState = psoDesc.blendDesc;
		d3dDesc.DepthStencilState = psoDesc.depthStencilDesc;
		d3dDesc.RasterizerState = psoDesc.rasterizerDesc;
		d3dDesc.DSVFormat = psoDesc.renderTargetState.dsvFormat;
		d3dDesc.NodeMask = psoDesc.renderTargetState.nodeMask;
		d3dDesc.NumRenderTargets = psoDesc.renderTargetState.numRenderTargets;
		memcpy(d3dDesc.RTVFormats, psoDesc.renderTargetState.rtvFormats, sizeof(DXGI_FORMAT) * D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);
		d3dDesc.SampleDesc = psoDesc.renderTargetState.sampleDesc;
		d3dDesc.SampleMask = psoDesc.renderTargetState.sampleMask;
		d3dDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		d3dDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		d3dDesc.pRootSignature = rootSignatureStatic.Get( );
		d3dDesc.VS = { SpriteEffect_SpriteVertexShader, sizeof(SpriteEffect_SpriteVertexShader) };
		d3dDesc.PS = { SpriteEffect_SpritePixelShader, sizeof(SpriteEffect_SpritePixelShader) };

		hr = device->CreateGraphicsPipelineState(
			&d3dDesc,
			IID_PPV_ARGS(mPSO.GetAddressOf()));
		Tool::SetDebugObjectName(mPSO.Get(), L"SpriteBatch");
		}

		ID3D12CommandQueue* commandQueue = CommandQueue;
        hr = mList->Close( );
        // Submit the job to the GPU
        commandQueue->ExecuteCommandLists(1, CommandListCast(mList.GetAddressOf()));
        // Set an event so we get notified when the GPU has completed all its work
        CPtr<ID3D12Fence> fence;
        hr = device ->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
        Tool::SetDebugObjectName(fence.Get(), L"ResourceUploadBatch");

        HANDLE gpuCompletedEvent = ::CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
        if ( !gpuCompletedEvent )
            throw std::system_error(std::error_code(static_cast<int>(GetLastError()), std::system_category()), "CreateEventEx");
        hr = commandQueue->Signal( fence.Get(), 1ULL );
        hr = fence->SetEventOnCompletion( 1ULL, gpuCompletedEvent );

		if ( WAIT_OBJECT_0 != WaitForSingleObject( gpuCompletedEvent, INFINITE ) )
			throw std::runtime_error("WaitForSingleObject");

		return { 
				device
				, m_pHeap
				, { handleGpu, textureSize }
				, rootSignatureStatic
				, mPSO
				, indexBufferView
				, std::move( puGlyph )
				, { indexBuffer , textureResource }
			};
	}
} // namespace prj_3d::MinimalDx12DrawText
