// Input control point
struct VSOut
{
	float4	Pos		: SV_POSITION;
	float3	WSPos	: POSWORLD;
	float3	Nrm		: NORMAL;
	float	NDotV	: NDOTV;
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

#define NUM_CONTROL_POINTS 3

// Patch Constant Function
HSConstDataOut CalcHSPatchConstants(
	InputPatch<VSOut, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HSConstDataOut Output;
	
	uint2 k[2] = { uint2(0, 0), uint2(0, 0) };
	uint c = 0;
	if (ip[0].NDotV * ip[1].NDotV <= 0) k[c++] = uint2(0, 1);
	if (ip[1].NDotV * ip[2].NDotV <= 0) k[c++] = uint2(1, 2);
	if (ip[2].NDotV * ip[0].NDotV <= 0) k[c++] = uint2(2, 0);

	float4 vPos[2];
	[unroll]
	for (uint i = 0; i < 2; ++i)
	{
		const float l = ip[k[i].x].NDotV / (ip[k[i].x].NDotV - ip[k[i].y].NDotV);
		vPos[i] = lerp(ip[k[i].x].Pos, ip[k[i].y].Pos, l);
	}
	float len = distance(vPos[0].xyz / vPos[0].w, vPos[1].xyz / vPos[1].w);

	// Insert code to compute Output here
	Output.EdgeTessFactor[0] = Output.EdgeTessFactor[1] = max((uint)(len * 512.0), 1);

	return Output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(NUM_CONTROL_POINTS - 1)]
[patchconstantfunc("CalcHSPatchConstants")]
HSOut main(
	InputPatch<VSOut, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
	HSOut output = (HSOut)0;

	uint n, c = 0;
	for (uint k = 0; k < NUM_CONTROL_POINTS; ++k)
	{
		n = (k + 1) % NUM_CONTROL_POINTS;
		c += (ip[k].NDotV * ip[n].NDotV <= 0) ? 1 : 0;
		if (c >= i + 1) break;
	}

	if (k < NUM_CONTROL_POINTS)
	{
		const float l = ip[k].NDotV / (ip[k].NDotV - ip[n].NDotV);
		output.Pos = lerp(ip[k].Pos, ip[n].Pos, l);
		output.WSPos = lerp(ip[k].WSPos, ip[n].WSPos, l);
		output.Nrm = lerp(ip[k].Nrm, ip[n].Nrm, l);
	}

	return output;
}
