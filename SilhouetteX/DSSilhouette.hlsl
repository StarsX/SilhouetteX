
struct DSOut
{
	float4	Pos		: SV_POSITION;
	float3	WSPos	: POSWORLD;
	float3	Nrm		: NORMAL;
	float4	Color	: COLOR;
};

// Output control point
struct HSOut
{
	float4	Pos		: SV_POSITION;
	float3	WSPos	: POSWORLD;
	float3	Nrm		: NORMAL;
};

// Output patch constant data.
struct HSConstDataOut
{
	float EdgeTessFactor[2]	: SV_TessFactor;
};

#define NUM_CONTROL_POINTS	2
#define Z_BIAS				0.5

[domain("isoline")]
DSOut main(
	HSConstDataOut input,
	float domain : SV_DomainLocation,
	const OutputPatch<HSOut, NUM_CONTROL_POINTS> patch)
{
	DSOut output;

	output.Pos = lerp(patch[0].Pos, patch[1].Pos, domain);
	output.WSPos = lerp(patch[0].WSPos, patch[1].WSPos, domain);
	output.Nrm = lerp(patch[0].Nrm, patch[1].Nrm, domain);

	output.Pos.z -= Z_BIAS * (1.0 - output.Pos.z / output.Pos.w);
	output.Color = float4(1.0, 0.0.xx, 0.5 / input.EdgeTessFactor[0]);

	return output;
}
