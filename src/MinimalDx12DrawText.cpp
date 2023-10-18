// src\MinimalDx12DrawText.cpp - entry point
// Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
// @insp https://www.shadertoy.com/view/wtB3RG PrzemyslawZaworski
#include "MsvcGenerated\stdafx.h"
#include "Main.h"

extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, PSTR, int) {
#ifdef _DEBUG
    HeapSetInformation( NULL, HeapEnableTerminationOnCorruption, NULL, NULL );
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	UINT uExitCode = prj_3d::MinimalDx12DrawText::Main::run( hInst );
	return static_cast< int >( uExitCode );
}
