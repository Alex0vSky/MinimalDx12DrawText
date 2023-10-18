// src\common.h - common for plain fontWorks and drawWorks
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_3d::MinimalDx12DrawText {
// Helper utility converts D3D API failures into exceptions.
static inline void ThrowIfFailed(HRESULT hr) noexcept(false) {
	// Helper class for COM exceptions
	class com_exception : public std::exception {
		HRESULT result;
	public:
		explicit com_exception(HRESULT hr) : result(hr) {}
		const char* what() const noexcept override {
			static char s_str[64] = {};
			sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
			return s_str;
		}
		HRESULT get_result() const noexcept { return result; }
	};
	if (FAILED(hr))
		throw com_exception(hr);
}

namespace Tool {
	static inline void SetDebugObjectName(ID3D12DeviceChild* resource, const wchar_t *name) {
#ifdef _DEBUG
		resource->SetName(name);
#endif // _DEBUG
	}
} // namespace Tool

namespace Const {
    static constexpr size_t VerticesPerSprite = 4;
    static constexpr size_t IndicesPerSprite = 6;
	static constexpr size_t MaxBatchSize = 2048;
} // namespace Const

namespace Enum {
enum RootParameterIndex
{
    TextureSRV,
    ConstantBuffer,
    TextureSampler,
    RootParameterCount
};
} // namespace Enum

namespace Elem {
struct FontTexture {
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle;
	XMUINT2 textureSize;
};

struct Context {
	CPtr< ID3D12Device > mDevice;
	CPtr< ID3D12DescriptorHeap > mHeap;
	Elem::FontTexture fontTexture;
	CPtr< ID3D12RootSignature > mRootSignature;
	CPtr< ID3D12PipelineState > mPSO;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	Elem::Glyph::uptr_t puGlyph;
	struct {
		CPtr< ID3D12Resource > indexBufferHolder;
		CPtr< ID3D12Resource > textureResourceHolder;
	} detail_;
	CPtr< ID3D12GraphicsCommandList > mCommandList;
	D3D12_VIEWPORT mViewport;
};
} // namespace Elem
} // namespace prj_3d::MinimalDx12DrawText
