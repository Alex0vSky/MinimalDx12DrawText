// src\plain_drawWorks.h - refactor ... .h
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_3d::MinimalDx12DrawText::Plain {
	Tool::MappedMem drawWorks(
		const Elem::Context &input
		, const wchar_t *text
		, FXMVECTOR color = DirectX::Colors::White
		, FXMVECTOR position = { 0, 0 }
	) {
		if ( !wcsnlen_s( text, Const::MaxBatchSize ) )
			return { };

		ID3D12DescriptorHeap* heaps[] = { input.mHeap.Get( ) };
		input.mCommandList ->SetDescriptorHeaps(
				static_cast< UINT >( std::size( heaps ) )
				, heaps
			);

		float rotation = 0.f;
		FXMVECTOR baseOffset = DirectX::g_XMZero;
		GXMVECTOR scale = DirectX::XMVectorReplicate( 1 );
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
		input.mCommandList ->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		input.mCommandList ->SetGraphicsRootSignature( input.mRootSignature.Get( ) );
		input.mCommandList ->SetPipelineState( input.mPSO.Get( ) );
		input.mCommandList ->IASetIndexBuffer( &input.indexBufferView );
		// Compute the matrix.
		const float xScale = (input.mViewport.Width > 0) ? 2.0f / input.mViewport.Width : 0.0f;
		const float yScale = (input.mViewport.Height > 0) ? 2.0f / input.mViewport.Height : 0.0f;
		// Set the transform matrix, DXGI_MODE_ROTATION_IDENTITY
		XMMATRIX transformMatrix = XMMATRIX
		{
			xScale, 0, 0, 0,
			0, -yScale, 0, 0,
			0, 0, 1, 0,
			-1, 1, 0, 1
		};

		Tool::MappedMem mappedMem;
		Tool::MappedMem::Page page1 = mappedMem.getNewPage( input.mDevice.Get( ), L"ConstantBuffer" );
		memcpy( page1.m_cpuAddress, &transformMatrix, sizeof( transformMatrix )  );
		input.mCommandList ->SetGraphicsRootConstantBufferView( Enum::RootParameterIndex::ConstantBuffer, page1.m_gpuAddress );
		Tool::MappedMem::Page page2 = mappedMem.getNewPage( input.mDevice.Get( ), L"Vertices" );
		Tool::SpritesToRender::PreparedVertices vertices = Tool::SpritesToRender::makeVertices( page2, &spriteQueue );

		// Draw using the specified texture. **NOTE** If D3D asserts or crashes here, you probably need to call commandList->SetDescriptorHeaps() with the required descriptor heap(s)
		input.mCommandList ->SetGraphicsRootDescriptorTable( Enum::RootParameterIndex::TextureSRV, spriteQueue.textureHandleHead( ) );
		// Set the vertex buffer view
		input.mCommandList ->IASetVertexBuffers( 0, 1, &vertices.vbv );
		// Ok lads, the time has come for us draw ourselves some sprites!
		const UINT indexCount = static_cast<UINT>( vertices.batchSize * Const::IndicesPerSprite );
		input.mCommandList ->DrawIndexedInstanced( indexCount, 1, 0, 0, 0 );

		return mappedMem;
	}
} // namespace prj_3d::MinimalDx12DrawText