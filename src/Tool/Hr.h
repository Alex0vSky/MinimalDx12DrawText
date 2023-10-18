// Tool/Hr.h - throws an exception when an erroneous "HRESULT" is assigned.
// @insp https://stackoverflow.com/questions/46416029/finding-the-calling-functions-address-in-visual-c-safely
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
#include <comdef.h>
namespace prj_3d::MinimalDx12DrawText::Tool {
class Hr {
    HRESULT m_hr;
    void check(void *xAddr) {
        if ( S_OK == m_hr )
            return;
		throw _com_error( m_hr );
    }

 public:
    Hr() :
		m_hr ( S_OK )
    {}
	// Its converting constructor
    Hr(HRESULT hrValue) // cppcheck-suppress noExplicitConstructor; NOLINT(runtime/explicit)
        :m_hr ( hrValue ) {
        check( _ReturnAddress( ) );
    }
    // non copy-and-swap idiom
    Hr &operator= (HRESULT hrValue) {
        m_hr = hrValue;
        check( _ReturnAddress( ) );
        return *this;
    }
	// or -fno-elide-constructor
	Hr(const Hr&& rhs) 
		: m_hr( rhs.m_hr ) 
    {}
    // NonCopyable almost -- "Hr() = default;"
    Hr(const Hr&) = delete;
    Hr & operator=(const Hr&) = delete;
};
} // namespace prj_3d::MinimalDx12DrawText::Sys
