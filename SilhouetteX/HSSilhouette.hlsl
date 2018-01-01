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

	// Insert code to compute Output here
	Output.EdgeTessFactor[0] = Output.EdgeTessFactor[1] = 1;

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
	
	//if (k < NUM_CONTROL_POINTS)
	{
		const float l = ip[k].NDotV / (ip[k].NDotV - ip[n].NDotV);
		output.Pos = lerp(ip[k].Pos, ip[n].Pos, l);
		output.WSPos = lerp(ip[k].WSPos, ip[n].WSPos, l);
		output.Nrm = lerp(ip[k].Nrm, ip[n].Nrm, l);
	}

	return output;
}
