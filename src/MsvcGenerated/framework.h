// framework.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <Windows.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <wrl/client.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>

#include <string>
#include <stdexcept>
#include <memory>
#include <vector>
#include <algorithm>
#include <system_error>
#include <iterator>


#pragma warning( disable: 26812 )
#pragma warning( disable: 26495 )
#ifdef __clang__
#	pragma clang diagnostic ignored "-Wnew-returns-null"
#	pragma clang diagnostic ignored "-Wclass-conversion"
#	pragma clang diagnostic ignored "-Wmissing-braces"
#endif // __clang__

#include <d3d12.h>
#include <D3Dcompiler.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#ifdef _DEBUG
#	include <dxgidebug.h>
#endif

//	MIT https://github.com/microsoft/DirectX-Headers/blob/main/include/directx/d3dx12.h
#	define D3DX12_NO_STATE_OBJECT_HELPERS
#	define D3DX12_NO_CHECK_FEATURE_SUPPORT_CLASS
//	because old d3dcommon.h, and dont have D3D_FORMAT_LAYOUT declatation
#	define __D3D12_PROPERTY_LAYOUT_FORMAT_TABLE_H__
#include "d3dx12.h"
