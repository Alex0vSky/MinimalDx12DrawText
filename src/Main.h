// src\Main.h - code main class, for unittests code coverage
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
#include "Tool/Hr.h"
using namespace DirectX;
#include "Elem/VertexTypes.h"
#include "Elem/Glyph.h"
#include "Elem/PipelineStateDescription.h"
#include "Elem/SpriteQueue.h"
#include "common.h"
#include "Tool/BinaryReader.h"
#include "Tool/Uploader.h"
#include "Tool/MappedMem.h"
#include "Tool/DrawOnSprite.h"
#include "Tool/SpritesToRender.h"
#include "plain_fontWorks.h"
#include "plain_drawWorks.h"

const int WIDTH = 640;
const int HEIGHT = 480;
const int FRAMES = 2;

IDXGISwapChain3* mSwapChain;
ID3D12Device* mDevice;
ID3D12Resource* mRenderTarget[ FRAMES ];
ID3D12CommandAllocator* mCommandAllocator;
ID3D12CommandQueue* mCommandQueue;
ID3D12DescriptorHeap* mDescriptorHeap;
ID3D12PipelineState* mPSO;
ID3D12GraphicsCommandList* mCommandList;
ID3D12RootSignature* mRootSignature;
HANDLE mfenceEvent;
ID3D12Fence* mFence;
UINT64 mFenceValue;
UINT mframeIndex;
ID3D12Resource* buffer;

namespace prj_3d::MinimalDx12DrawText {
struct Main {
	static void WaitForPreviousFrame()
	{
		const UINT64 fence = mFenceValue;
		mCommandQueue->Signal( mFence, fence);
		mFenceValue++;
		if (mFence->GetCompletedValue( ) < fence)
		{
			mFence->SetEventOnCompletion( fence, mfenceEvent);
			::WaitForSingleObject(mfenceEvent, INFINITE);
		}
		mframeIndex = mSwapChain->GetCurrentBackBufferIndex( );
	}
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if ((uMsg == WM_KEYUP && wParam == VK_ESCAPE) || uMsg==WM_CLOSE || uMsg==WM_DESTROY)
		{
			PostQuitMessage(0);
			return 0;
		}
		else
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}
	static UINT run(HINSTANCE hInst) {

		int exit = 0;
		MSG msg;
		WNDCLASS win;
		ZeroMemory( &win, sizeof(WNDCLASS) );
		win.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
		win.lpfnWndProc = WindowProc;
		win.hInstance = 0;
		win.lpszClassName = "DirectX 12 Demo";
		win.hbrBackground =(HBRUSH)(COLOR_WINDOW+1);
		RegisterClass(&win);
		HWND hWnd = CreateWindowEx(0, win.lpszClassName, "DirectX 12 Demo", WS_VISIBLE|WS_OVERLAPPEDWINDOW, 0, 0, WIDTH, HEIGHT, 0, 0, 0, 0);
		HDC hdc = GetDC(hWnd);

		{
		CPtr< IDXGIFactory4> cpFactory;
		HRESULT hr;
		UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
		{
			CPtr< ID3D12Debug > pcD3D12Debug;
			hr = ::D3D12GetDebugInterface( IID_PPV_ARGS( pcD3D12Debug.ReleaseAndGetAddressOf( ) ) );
			if ( SUCCEEDED( hr ) ) {
				pcD3D12Debug ->EnableDebugLayer( );
				// Enable additional debug layers.
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
#endif // _DEBUG
		hr = CreateDXGIFactory2( dxgiFactoryFlags, IID_PPV_ARGS(cpFactory.ReleaseAndGetAddressOf( )) );
		CPtr< IDXGIAdapter1 > cpDxgiAdapter;
		hr = cpFactory ->EnumAdapters1( 0, cpDxgiAdapter.ReleaseAndGetAddressOf( ) );
		DXGI_ADAPTER_DESC1 struAdapterDesc;
		hr = cpDxgiAdapter ->GetDesc1( &struAdapterDesc );
		OutputDebugStringW( struAdapterDesc.Description );
		OutputDebugStringW( L"\n" );
		hr = D3D12CreateDevice( (IUnknown *)cpDxgiAdapter.Get( ), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));

		D3D12_COMMAND_QUEUE_DESC queueDesc = {D3D12_COMMAND_LIST_TYPE_DIRECT, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0};
		mDevice->CreateCommandQueue( &queueDesc, IID_PPV_ARGS(&mCommandQueue));

		// Prepare draw text context
		Elem::Context ctx = Plain::fontWorks( mDevice, mCommandQueue );

		DXGI_SWAP_CHAIN_DESC descSwapChain = {
			DXGI_MODE_DESC{
				WIDTH
				,HEIGHT
				,DXGI_RATIONAL{0,0}
				,DXGI_FORMAT_R8G8B8A8_UNORM
				,(DXGI_MODE_SCANLINE_ORDER)0,(DXGI_MODE_SCALING)0}
			,DXGI_SAMPLE_DESC{1,0}
			,1L << (1 + 4)
			,FRAMES
			,hWnd
			,1
			,(DXGI_SWAP_EFFECT)3
			,0};
		IDXGISwapChain* SwapChain;
		cpFactory->CreateSwapChain( (IUnknown *)mCommandQueue, &descSwapChain, &SwapChain);
		SwapChain->QueryInterface( IID_PPV_ARGS(&mSwapChain));
		SwapChain->Release( );
		D3D12_INPUT_ELEMENT_DESC layout[] ={{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }};	
		ID3DBlob* blob;
		D3D12_ROOT_PARAMETER timeParam;
		ZeroMemory(&timeParam, sizeof(timeParam));
		timeParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		timeParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		timeParam.Constants = D3D12_ROOT_CONSTANTS{ 0,0, 1  };
		D3D12_ROOT_SIGNATURE_DESC descRootSignature = {1, &timeParam, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};
		D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &blob, 0);
		mDevice->CreateRootSignature( 0, blob->GetBufferPointer( ), blob->GetBufferSize( ), IID_PPV_ARGS( &mRootSignature));
		D3D12_RASTERIZER_DESC rasterizer ={D3D12_FILL_MODE_SOLID,D3D12_CULL_MODE_BACK,0,D3D12_DEFAULT_DEPTH_BIAS,D3D12_DEFAULT_DEPTH_BIAS_CLAMP,0.0f,1,0,0,0,(D3D12_CONSERVATIVE_RASTERIZATION_MODE)0};
		D3D12_BLEND_DESC blendstate = { 
			0
			, 0
			, D3D12_RENDER_TARGET_BLEND_DESC{
				0
				, 0
				, (D3D12_BLEND)1, (D3D12_BLEND)0, D3D12_BLEND_OP_ADD, (D3D12_BLEND)1, (D3D12_BLEND)0, D3D12_BLEND_OP_ADD, D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL} 
		};
		static D3D12_GRAPHICS_PIPELINE_STATE_DESC pDesc;
		pDesc.pRootSignature = mRootSignature;	
		pDesc.VS = D3D12_SHADER_BYTECODE{VertexShader,sizeof(VertexShader)};
		pDesc.PS = D3D12_SHADER_BYTECODE{PixelShader,sizeof(PixelShader)};
		pDesc.InputLayout = D3D12_INPUT_LAYOUT_DESC{layout, _countof(layout)};	
		pDesc.RasterizerState = rasterizer;
		pDesc.BlendState = blendstate;
		pDesc.DepthStencilState = { };
		pDesc.SampleMask = UINT_MAX;
		pDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pDesc.NumRenderTargets = 1;
		pDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		pDesc.SampleDesc.Count = 1;
		hr = mDevice->CreateGraphicsPipelineState(&pDesc, IID_PPV_ARGS(&mPSO));
		static D3D12_DESCRIPTOR_HEAP_DESC descHeap = {D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FRAMES, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0};
		mDevice->CreateDescriptorHeap(&descHeap, IID_PPV_ARGS(&mDescriptorHeap));
		UINT mrtvDescriptorIncrSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
		rtvHandle = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart( );
		for (UINT i = 0; i < FRAMES; i++)
		{
			mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTarget[i])); 
			mDevice->CreateRenderTargetView(mRenderTarget[i], NULL, rtvHandle);
			rtvHandle.ptr += mrtvDescriptorIncrSize;
		}
		mDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator));
		mDevice->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator, mPSO, IID_PPV_ARGS(&mCommandList));
		D3D12_VIEWPORT mViewport = { 0.0f, 0.0f, (float)(WIDTH), (float)(HEIGHT), 0.0f, 1.0f };
		D3D12_RECT mRectScissor = { 0, 0, (LONG)(WIDTH), (LONG)(HEIGHT) };
		float vertices[] ={ -1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 1.0, 0.0f, 1.0f, 1.0f, 0.0f};
		static D3D12_HEAP_PROPERTIES heapProperties ={D3D12_HEAP_TYPE_UPLOAD,D3D12_CPU_PAGE_PROPERTY_UNKNOWN,D3D12_MEMORY_POOL_UNKNOWN,1,1};
		static D3D12_RESOURCE_DESC VertexBufferDesc ={
			D3D12_RESOURCE_DIMENSION_BUFFER,0,_countof(vertices) * 12,1,1,1,DXGI_FORMAT_UNKNOWN,{1,0},(D3D12_TEXTURE_LAYOUT)1,(D3D12_RESOURCE_FLAGS)0};
		mDevice->CreateCommittedResource( &heapProperties,(D3D12_HEAP_FLAGS)0,&VertexBufferDesc,D3D12_RESOURCE_STATE_GENERIC_READ,NULL,IID_PPV_ARGS(&buffer));
		float* data;
		buffer->Map( 0, NULL, (void**)(&data));
		memcpy(data, vertices, sizeof(vertices));
		buffer->Unmap( 0, NULL);
		D3D12_VERTEX_BUFFER_VIEW mDescViewBufVert = {buffer->GetGPUVirtualAddress( ), sizeof(vertices), 12};
		mCommandList->Close( );
		mDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
		mFenceValue = 1;
		mfenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);

		while (!exit)
		{
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT) exit = 1;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			hr = mCommandAllocator->Reset( );
			hr = mCommandList->Reset( mCommandAllocator, mPSO);
			mCommandList->SetGraphicsRootSignature( mRootSignature);

			// Time from start
			static float timerStart = GetTickCount() * 0.001f;
			float timer = GetTickCount() * 0.001f - timerStart;

			mCommandList->SetGraphicsRoot32BitConstants( 0,1,&timer,0);
			mCommandList->RSSetViewports( 1, &mViewport);
			mCommandList->RSSetScissorRects( 1, &mRectScissor);
			D3D12_RESOURCE_BARRIER barrierRTAsTexture = {(D3D12_RESOURCE_BARRIER_TYPE)0, (D3D12_RESOURCE_BARRIER_FLAGS)0,{ mRenderTarget[mframeIndex], D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET }};
			mCommandList->ResourceBarrier( 1, &barrierRTAsTexture);
			rtvHandle = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart( );

			rtvHandle.ptr += mframeIndex * mrtvDescriptorIncrSize;

			float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			mCommandList->ClearRenderTargetView( rtvHandle, clearColor, 0, NULL);
			mCommandList->OMSetRenderTargets( 1, &rtvHandle, TRUE, NULL);
			mCommandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			mCommandList->IASetVertexBuffers( 0, 1, &mDescViewBufVert);
			mCommandList->DrawInstanced( 6, 1, 0, 0);

			// Text will be: FPS count
			ctx.mCommandList = mCommandList;
			ctx.mViewport = mViewport;
			static float lastTime = (float)GetTickCount( );
			static int nbFrames = 0;
			nbFrames++;
			wchar_t buf64[ 64 + 1 ] = { };
			if ( float diff = GetTickCount( ) - lastTime ) 
				 _itow_s( int( nbFrames / ( diff / 1000.f ) ), buf64, 10 );
			// Draw text
			Tool::MappedMem memHolder = Plain::drawWorks( ctx, buf64 );

			D3D12_RESOURCE_BARRIER barrierRTForPresent = {(D3D12_RESOURCE_BARRIER_TYPE)0, (D3D12_RESOURCE_BARRIER_FLAGS)0,{ mRenderTarget[mframeIndex], D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT }};
			mCommandList->ResourceBarrier( 1, &barrierRTForPresent);
			hr = mCommandList->Close( );
			ID3D12CommandList* ppCommandLists[] = { (ID3D12CommandList *) mCommandList };
			mCommandQueue->ExecuteCommandLists( _countof(ppCommandLists), ppCommandLists );
			hr = mSwapChain->Present( 0, 0 );
			WaitForPreviousFrame( );
		}
		CloseHandle(mfenceEvent);
		mDevice->Release( );
		mSwapChain->Release( );
		buffer->Release( );
		for (UINT n = 0; n < FRAMES; n++) mRenderTarget[n]->Release( );
		mCommandAllocator->Release( );
		mCommandQueue->Release( );
		mDescriptorHeap->Release( );
		mCommandList->Release( );
		mPSO->Release( );
		mFence->Release( );
		mRootSignature->Release( );
		}
		
#ifdef _DEBUG
		IDXGIDebug1* pDebug0 = NULL;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug0))))
			pDebug0->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY), pDebug0->Release();
#endif // _DEBUG

		return static_cast< UINT >( msg.wParam );
	}
};
} // namespace prj_3d::MinimalDx12DrawText
