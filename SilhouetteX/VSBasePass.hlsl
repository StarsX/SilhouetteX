struct VSIn
{
	float3	Pos		: POSITION;
	float3	Nrm		: NORMAL;
};

struct VSOut
{
	float4	Pos		: SV_POSITION;
	float3	WSPos	: POSWORLD;
	float3	Nrm		: NORMAL;
	float	NDotV	: NDOTV;
};

cbuffer cbMatrices
{
	matrix	g_mWorldViewProj;
	matrix	g_mWorld;
	matrix	g_mWorldIT;
};

cbuffer cbPerFrame
{
	float3	g_vEyePt;
};

VSOut main(VSIn input)
{
	VSOut output;

	const float4 vPos = float4(input.Pos, 1.0);
	output.Pos = mul(vPos, g_mWorldViewProj);
	output.WSPos = mul(vPos, g_mWorld).xyz;
	output.Nrm = mul(input.Nrm, (float3x3)g_mWorldIT);
	
	const float3 vViewDir = normalize(g_vEyePt - output.WSPos);
	output.NDotV = dot(output.Nrm, vViewDir);

	return output;
}
