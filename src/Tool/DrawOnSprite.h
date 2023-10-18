// src\Tool\DrawOnSprite.h - refactor ... .h
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_3d::MinimalDx12DrawText::Tool { class DrawOnSprite {
	Elem::SpriteQueue *m_pSpriteQueue;
	Elem::Glyph *m_pGlyph;
	Elem::FontTexture m_texture;

	inline bool valid_(wchar_t character, Elem::Glyph::Character const* glyph) {
		return false 
				|| !iswspace( character )
				|| ((glyph ->Subrect.right - glyph ->Subrect.left) > 1)
				|| ((glyph ->Subrect.bottom - glyph ->Subrect.top) > 1)
			;
	}
	// Adds a single sprite to the queue. Filling sprite data
	void glyphToTexture_(
		XMVECTOR dest
		, RECT const* sourceRectangle
		, FXMVECTOR color
		, FXMVECTOR originRotationDepth
	) {
		// Dynamically expands the array used to store pending sprite information. // void GrowSpriteQueue()
		m_pSpriteQueue ->grow( );
		// Get a pointer to the output sprite.
		Elem::SpriteQueue::Info* pSprite = m_pSpriteQueue ->current( );
		// User specified an explicit source region. // const XMVECTOR source = LoadRect(sourceRectangle);
        XMVECTOR source = XMLoadInt4(reinterpret_cast<uint32_t const*>(sourceRectangle));
        source = XMConvertVectorIntToFloat( source, 0);
        // Convert right/bottom to width/height.
        source = XMVectorSubtract( source, XMVectorPermute< 0, 1, 4, 5 >( g_XMZero, source ) );
		XMStoreFloat4A( &pSprite ->source, source );
		// destination size relative to the source region, convert it to pixels. // dest.zw *= source.zw
		dest = XMVectorPermute< 0, 1, 6, 7 >( dest, XMVectorMultiply( dest, source ) );
		// Convert texture size
		pSprite ->textureSize = XMLoadUInt2( &m_texture.textureSize );
		// Store sprite parameters.
		XMStoreFloat4A( &pSprite ->destination, dest );
		XMStoreFloat4A( &pSprite ->color, color );
		XMStoreFloat4A( &pSprite ->originRotationDepth, originRotationDepth );
		pSprite ->texture = m_texture.textureHandle;
		pSprite ->flags = Elem::SpriteQueue::Info::SourceInTexels | Elem::SpriteQueue::Info::DestSizeInPixels;
		m_pSpriteQueue ->next( );
	}

 public:
	DrawOnSprite(
		Elem::SpriteQueue *oSprite
		, Elem::Glyph *pGlyph
		, Elem::FontTexture texture
	) : 
		 m_pSpriteQueue( oSprite )
		 , m_pGlyph( pGlyph )
		 , m_texture( texture )
	 {}
	void allTextGlyphs(
		wchar_t const* text
		, FXMVECTOR position
		, FXMVECTOR color
		, float rotation
		, FXMVECTOR baseOffset
		, GXMVECTOR scale
		, float layerDepth

	) {
		float x = 0;
		float y = 0;
		for ( ; *text; text++ ) {
			const wchar_t character = *text;
			// Skip carriage returns.
			if ( '\r' == character ) 
				continue;
			// New line.
			if ( '\n' == character ) {
				x = 0;
				y += m_pGlyph ->getLineSpacing( );
				continue;
			}
			// Output this character.
			Elem::Glyph::Character const* glyph = m_pGlyph ->findGlyph( character );
			x += glyph ->XOffset;
			if ( x < 0 )
				x = 0;
			const float advance = float(glyph ->Subrect.right) - float(glyph ->Subrect.left) + glyph ->XAdvance;
			float x_for_offset = x;
			x += advance;
			if ( !valid_( character, glyph ) ) 
				continue;
			XMVECTOR offset = XMVectorMultiplyAdd(
					XMVectorSet( x_for_offset, y + glyph ->YOffset, 0, 0 )
					, { -1, -1, 0, 0 }
					, baseOffset
				);
			// x, y, scale.x, scale.y
			const XMVECTOR destination = XMVectorPermute< 0, 1, 4, 5 >( position, scale );
			const XMVECTOR rotationDepth = XMVectorMergeXY(
				XMVectorReplicate( rotation ),
				XMVectorReplicate( layerDepth ));
			const XMVECTOR originRotationDepth = XMVectorPermute< 0, 1, 4, 5 >( offset, rotationDepth );
			glyphToTexture_( 
					destination
					, &glyph ->Subrect
					, color
					, originRotationDepth
				);
		}
	}
};} // namespace prj_3d::MinimalDx12DrawText::Tool