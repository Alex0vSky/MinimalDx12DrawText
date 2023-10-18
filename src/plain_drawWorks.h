// src\plain_drawWorks.h - refactor ... .h
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_3d::MinimalDx12DrawText::Plain {
	Tool::MappedMem drawWorks(
		ID3D12Device* device
		, CPtr< ID3D12GraphicsCommandList > commandList
		, const Elem::FontData &input
		, D3D12_VIEWPORT *mViewport
		, const wchar_t *text
	) {
		ID3D12DescriptorHeap* heaps[] = { input.m_pHeap.Get( ) };
		commandList ->SetDescriptorHeaps(
				static_cast< UINT >( std::size( heaps ) )
				, heaps
			);

		FXMVECTOR position = { 0, 0 };
		FXMVECTOR color = DirectX::Colors::Blue;
		float rotation = 0.f;
		FXMVECTOR baseOffset = DirectX::g_XMZero;
		GXMVECTOR scale = DirectX::XMVectorReplicate( 2 );
		float layerDepth = 0;

		Elem::SpriteQueue spriteQueue;
		Tool::DrawOnSprite drawOnSprites( 
			&spriteQueue 
			, input.puGlyph.get( )
			, input.fontTexture
		);

		// Draw each characters in sprites
		drawOnSprites.allTextGlyphs( 
				text
				, position
				, color
				, rotation
				, baseOffset
				, scale
				, layerDepth
			);
		// Prepare
		commandList ->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		commandList ->SetGraphicsRootSignature( input.mRootSignature.Get( ) );
		commandList ->SetPipelineState( input.mPSO.Get( ) );
		commandList ->IASetIndexBuffer( &input.indexBufferView );
		if ( !mViewport )
			throw std::runtime_error( "Viewport not set." );
		// Compute the matrix.
		const float xScale = (mViewport ->Width > 0) ? 2.0f / mViewport ->Width : 0.0f;
		const float yScale = (mViewport ->Height > 0) ? 2.0f / mViewport ->Height : 0.0f;
		// Set the transform matrix, DXGI_MODE_ROTATION_IDENTITY
		XMMATRIX transformMatrix = XMMATRIX
		{
			xScale, 0, 0, 0,
			0, -yScale, 0, 0,
			0, 0, 1, 0,
			-1, 1, 0, 1
		};

		Tool::MappedMem mappedMem;
		Tool::MappedMem::Page page1 = mappedMem.getNewPage( device, L"ConstantBuffer" );
		memcpy( page1.m_cpuAddress, &transformMatrix, sizeof( transformMatrix )  );
		commandList ->SetGraphicsRootConstantBufferView( Enum::RootParameterIndex::ConstantBuffer, page1.m_gpuAddress );
		Tool::MappedMem::Page page2 = mappedMem.getNewPage( device, L"Vertices" );
		Tool::SpritesToRender::PreparedVertices vertices = Tool::SpritesToRender::makeVertices( page2, &spriteQueue );

		// Draw using the specified texture. **NOTE** If D3D asserts or crashes here, you probably need to call commandList->SetDescriptorHeaps() with the required descriptor heap(s)
		commandList ->SetGraphicsRootDescriptorTable( Enum::RootParameterIndex::TextureSRV, spriteQueue.textureHandleHead( ) );
		// Set the vertex buffer view
		commandList ->IASetVertexBuffers( 0, 1, &vertices.vbv );
		// Ok lads, the time has come for us draw ourselves some sprites!
		const UINT indexCount = static_cast<UINT>( vertices.batchSize * Const::IndicesPerSprite );
		commandList ->DrawIndexedInstanced( indexCount, 1, 0, 0, 0 );

		return std::move( mappedMem );
	}
} // namespace prj_3d::MinimalDx12DrawText