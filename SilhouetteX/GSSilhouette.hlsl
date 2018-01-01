struct VSOut
{
	float4	Pos		: SV_POSITION;
	float3	WSPos	: POSWORLD;
	float3	Nrm		: NORMAL;
	float	NDotV	: NDOTV;
};

struct GSOut
{
	float4	Pos		: SV_POSITION;
	float3	WSPos	: POSWORLD;
	float3	Nrm		: NORMAL;
};

#define Z_BIAS	0.02

// calculating facing of a triangle relative to eye
bool silhouette(VSOut v1, VSOut v2, inout GSOut v)
{
	const float f = v1.NDotV * v2.NDotV;
	const bool r = (f <= 0);

	if (r)
	{
		const float l = v1.NDotV / (v1.NDotV - v2.NDotV);
		v.Pos = lerp(v1.Pos, v2.Pos, l);
		v.WSPos = lerp(v1.WSPos, v2.WSPos, l);
		v.Nrm = lerp(v1.Nrm, v2.Nrm, l);
	}
	
	return r;
}

[maxvertexcount(3)]
void main(triangle VSOut input[3], inout LineStream<GSOut> output)
{
	GSOut v[2] = { (GSOut)0, (GSOut)0 };
	uint c = 0;
	if (silhouette(input[0], input[1], v[c])) ++c;
	if (silhouette(input[1], input[2], v[c])) ++c;
	if (silhouette(input[2], input[0], v[c])) ++c;

	if (c > 0)
	{
		v[0].Pos.z -= Z_BIAS;
		v[1].Pos.z -= Z_BIAS;
		output.Append(v[0]);
		output.Append(v[1]);
		output.RestartStrip();
	}
}
