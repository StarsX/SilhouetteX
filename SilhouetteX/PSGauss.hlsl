struct PSIn
{
	float4	Pos		: SV_POSITION;
	float2	Tex		: TEXCOORD;
	float4	Color	: COLOR;
};

float4 main(PSIn input) : SV_TARGET
{
	const float2 vr = (input.Tex - 0.5) * 2.0;
	const float fGauss = exp(-4.0 * dot(vr, vr));
	const float fAlpha = fGauss * input.Color.w;

	return float4(input.Color.xyz, fAlpha);
}
