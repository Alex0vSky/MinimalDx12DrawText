// src\Elem\PipelineStateDescription.h - from DirectXTK
#pragma once // NOLINT copyright
namespace prj_3d::MinimalDx12DrawText::Elem {
class PipelineStateDescription {
	// Encapsulates all render target state when creating pipeline state objects
	struct RenderTargetState {
		RenderTargetState(
			DXGI_FORMAT rtFormat
			, DXGI_FORMAT dsFormat
		) : 
			rtvFormats{ }
			, dsvFormat(dsFormat)
		{
			sampleDesc.Count = 1;
			rtvFormats[ 0 ] = rtFormat;
		}
		uint32_t            sampleMask = UINT_MAX;
		uint32_t            numRenderTargets = 1;
		DXGI_FORMAT         rtvFormats[ D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT ];
		DXGI_FORMAT         dsvFormat;
		DXGI_SAMPLE_DESC    sampleDesc{ };
		uint32_t            nodeMask = 0;
	};

public:
	explicit PipelineStateDescription(const RenderTargetState& renderTarget) :
		renderTargetState( renderTarget )
	{}
	// Matches CommonStates::AlphaBlend
	const D3D12_BLEND_DESC blendDesc = {
		FALSE, // AlphaToCoverageEnable
		FALSE, // IndependentBlendEnable
		{ {
			TRUE, // BlendEnable
			FALSE, // LogicOpEnable
			D3D12_BLEND_ONE, // SrcBlend
			D3D12_BLEND_INV_SRC_ALPHA, // DestBlend
			D3D12_BLEND_OP_ADD, // BlendOp
			D3D12_BLEND_ONE, // SrcBlendAlpha
			D3D12_BLEND_INV_SRC_ALPHA, // DestBlendAlpha
			D3D12_BLEND_OP_ADD, // BlendOpAlpha
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		} }
	};
	// Same as CommonStates::DepthNone
	const D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {
		FALSE, // DepthEnable
		D3D12_DEPTH_WRITE_MASK_ZERO,
		D3D12_COMPARISON_FUNC_LESS_EQUAL, // DepthFunc
		FALSE, // StencilEnable
		D3D12_DEFAULT_STENCIL_READ_MASK,
		D3D12_DEFAULT_STENCIL_WRITE_MASK,
		{
			D3D12_STENCIL_OP_KEEP, // StencilFailOp
			D3D12_STENCIL_OP_KEEP, // StencilDepthFailOp
			D3D12_STENCIL_OP_KEEP, // StencilPassOp
			D3D12_COMPARISON_FUNC_ALWAYS // StencilFunc
		}, // FrontFace
		{
			D3D12_STENCIL_OP_KEEP, // StencilFailOp
			D3D12_STENCIL_OP_KEEP, // StencilDepthFailOp
			D3D12_STENCIL_OP_KEEP, // StencilPassOp
			D3D12_COMPARISON_FUNC_ALWAYS // StencilFunc
		} // BackFace
	};
	// Same to CommonStates::CullCounterClockwise
	const D3D12_RASTERIZER_DESC rasterizerDesc = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK,
		FALSE, // FrontCounterClockwise
		D3D12_DEFAULT_DEPTH_BIAS,
		D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE, // DepthClipEnable
		TRUE, // MultisampleEnable
		FALSE, // AntialiasedLineEnable
		0, // ForcedSampleCount
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};
	const RenderTargetState renderTargetState;
};
} // namespace prj_3d::MinimalDx12DrawText::Elem
