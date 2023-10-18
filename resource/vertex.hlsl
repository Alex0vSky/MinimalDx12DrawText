struct VOut
{
	float4 position : SV_POSITION;
};

VOut main(float4 position : POSITION)
{
	VOut output;

	output.position = position;

	return output;
}
