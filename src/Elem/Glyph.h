// src\Elem\Glyph.h - refactor SpriteFont.h/SpriteFont.cpp
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_3d::MinimalDx12DrawText::Elem {
class Glyph {
 public:
	// Describes a single character glyph.
	struct Character {
		uint32_t Character;
		RECT Subrect;
		float XOffset, YOffset, XAdvance;
	};

	typedef std::unique_ptr< Glyph > uptr_t;
	Glyph(
		const Character* glyphData
		, uint32_t glyphCount
		, wchar_t character
		, float lineSpacing
	) :
		m_lineSpacing( lineSpacing )
	{
		m_glyphs.assign( glyphData, glyphData + glyphCount );
		m_glyphsIndex.reserve( m_glyphs.size( ) );
		std::transform( 
				m_glyphs.begin( ), m_glyphs.end( )
				, std::back_inserter( m_glyphsIndex ) 
				, [](const auto& glyph) { 
					return glyph.Character;
				}
			);
		if ( character )
			m_defaultGlyph = findGlyph(character);
	}
	// Looks up the requested glyph, falling back to the default character if it is not in the font.
	Character const* findGlyph(wchar_t character) {
		size_t lower = 0;
		size_t higher = m_glyphs.size() - 1;
		size_t index = higher / 2;
		const size_t size = m_glyphs.size();

		while (index < size) {
			const auto curChar = m_glyphsIndex[index];
			if (curChar == character) { return &m_glyphs[index]; }
			if (curChar < character) {
				lower = index + 1;
			} else {
				higher = index - 1;
			}
			if (higher < lower) { 
				break; 
			} else if (higher - lower <= 4) {
				for (index = lower; index <= higher; index++) {
					if (m_glyphsIndex[index] == character) {
						return &m_glyphs[index];
					}
				}
			}
			index = lower + ((higher - lower) / 2);
		}
		if (m_defaultGlyph)
			return m_defaultGlyph;
		throw std::runtime_error("Character not in font");
	}

	float getLineSpacing() {
		return m_lineSpacing;
	}

 private: 
	std::vector< Character > m_glyphs;
	std::vector< uint32_t > m_glyphsIndex;
	Character const* m_defaultGlyph = nullptr;
	float m_lineSpacing;
};
} // namespace prj_3d::MinimalDx12DrawText::Elem