// src\Tool\SpritesToRender.h - refactor SpriteBatch.h/*.cpp and SpriteFont.h/*.cpp
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_3d::MinimalDx12DrawText::Tool { class SpritesToRender {
	// Generates vertex data for drawing a single sprite.
	static void XM_CALLCONV RenderSprite(
		Elem::SpriteQueue::Info const* sprite
		, Elem::VertexPositionColorTexture* vertices
		, FXMVECTOR textureSize
		, FXMVECTOR inverseTextureSize
	) {
		// Load sprite parameters into SIMD registers.
		XMVECTOR source = XMLoadFloat4A(&sprite->source);
		const XMVECTOR destination = XMLoadFloat4A(&sprite->destination);
		const XMVECTOR originRotationDepth = XMLoadFloat4A(&sprite->originRotationDepth);

		const float rotation = sprite->originRotationDepth.z;
		const unsigned int flags = sprite->flags;

		// Extract the source and destination sizes into separate vectors.
		XMVECTOR sourceSize = XMVectorSwizzle<2, 3, 2, 3>(source);
		XMVECTOR destinationSize = XMVectorSwizzle<2, 3, 2, 3>(destination);
		// Scale the origin offset by source size, taking care to avoid overflow if the source region is zero.
		const XMVECTOR isZeroMask = XMVectorEqual(sourceSize, XMVectorZero());
		const XMVECTOR nonZeroSourceSize = XMVectorSelect(sourceSize, DirectX::g_XMEpsilon, isZeroMask);

		XMVECTOR origin = XMVectorDivide(originRotationDepth, nonZeroSourceSize);
		// Convert the source region from texels to mod-1 texture coordinate format.
		source = XMVectorMultiply(source, inverseTextureSize);
		sourceSize = XMVectorMultiply(sourceSize, inverseTextureSize);
		// Compute a 2x2 rotation matrix.
		XMVECTOR rotationMatrix1, rotationMatrix2;
		rotationMatrix1 = g_XMIdentityR0;
		rotationMatrix2 = g_XMIdentityR1;

		// The four corner vertices are computed by transforming these unit-square positions.
		static XMVECTORF32 cornerOffsets[Const::VerticesPerSprite] =
		{
			{ { { 0, 0, 0, 0 } } },
			{ { { 1, 0, 0, 0 } } },
			{ { { 0, 1, 0, 0 } } },
			{ { { 1, 1, 0, 0 } } },
		};

		// Tricksy alert! Texture coordinates are computed from the same cornerOffsets
		// table as vertex positions, but if the sprite is mirrored, this table
		// must be indexed in a different order. This is done as follows:
		//
		//    position = cornerOffsets[i]
		//    texcoord = cornerOffsets[i ^ SpriteEffects]
		const unsigned int mirrorBits = flags & 3u;

		// Generate the four output vertices.
		for (size_t i = 0; i < Const::VerticesPerSprite; i++) {
			// Calculate position.
			const XMVECTOR cornerOffset = XMVectorMultiply(XMVectorSubtract(cornerOffsets[i], origin), destinationSize);
			// Apply 2x2 rotation matrix.
			const XMVECTOR position1 = XMVectorMultiplyAdd(XMVectorSplatX(cornerOffset), rotationMatrix1, destination);
			const XMVECTOR position2 = XMVectorMultiplyAdd(XMVectorSplatY(cornerOffset), rotationMatrix2, position1);
			// Set z = depth.
			const XMVECTOR position = XMVectorPermute<0, 1, 7, 6>(position2, originRotationDepth);
			// Write position as a Float4, even though VertexPositionColor::position is an XMFLOAT3.
			// This is faster, and harmless as we are just clobbering the first element of the
			// following color field, which will immediately be overwritten with its correct value.
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&vertices[i].position), position);
			// Write the color.
			XMStoreFloat4( &vertices[i].color, XMLoadFloat4A(&sprite->color) );
			// Compute and write the texture coordinate.
			const XMVECTOR textureCoordinate = XMVectorMultiplyAdd(cornerOffsets[static_cast<unsigned int>(i) ^ mirrorBits], sourceSize, source);
			XMStoreFloat2(&vertices[i].textureCoordinate, textureCoordinate);
		}
	}

 public:	
	struct PreparedVertices {
		D3D12_VERTEX_BUFFER_VIEW vbv;
		size_t batchSize;
	};
	static PreparedVertices makeVertices(
		const MappedMem::Page &page
		, Elem::SpriteQueue *spriteQueue
	) {
		size_t count = spriteQueue ->count( );
		if ( !count )
			return { };
		// Fill the mSortedSprites vector.
		std::vector< Elem::SpriteQueue::Info const* > plainSprites = spriteQueue ->toPlain( );
		// Flush
		XMVECTOR textureSize = plainSprites[ 0 ] ->textureSize;
		Elem::SpriteQueue::Info const* const* pSprites = plainSprites.data( );

		Elem::VertexPositionColorTexture* vertices = static_cast<Elem::VertexPositionColorTexture*>( page.m_cpuAddress );
		// Convert to vector format.
		const XMVECTOR inverseTextureSize = XMVectorReciprocal( textureSize );
		// Generate sprite vertex data.
		for (size_t i = 0; i < count; i++) {
			RenderSprite( pSprites[ i ], vertices, textureSize, inverseTextureSize );
			vertices += Const::VerticesPerSprite;
		}
		// Set the vertex buffer view
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = page.m_gpuAddress;
		vbv.StrideInBytes = sizeof( Elem::VertexPositionColorTexture );
		vbv.SizeInBytes = static_cast<UINT>
			( count * sizeof( Elem::VertexPositionColorTexture ) * Const::VerticesPerSprite );
		return { vbv, count };
	}
};} // namespace prj_3d::MinimalDx12DrawText::Tool