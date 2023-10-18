// src\Elem\VertexTypes.h - from DirectXTK
#pragma once // NOLINT copyright
namespace prj_3d::MinimalDx12DrawText::Elem {
// Vertex struct holding position, color, and texture mapping information.
struct VertexPositionColorTexture {
    XMFLOAT3 position;
    XMFLOAT4 color;
    XMFLOAT2 textureCoordinate;

 private:
    static constexpr unsigned int InputElementCount = 3;
    inline static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount] =
	{
		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

 public:
    inline static const D3D12_INPUT_LAYOUT_DESC InputLayout = 
	{
		InputElements,
		InputElementCount
	};
};
static_assert(sizeof(VertexPositionColorTexture) == 36, "Vertex struct/layout mismatch");
} // namespace prj_3d::MinimalDx12DrawText::Elem
