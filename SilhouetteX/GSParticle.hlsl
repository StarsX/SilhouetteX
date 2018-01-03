struct DSOut
{
	float4	Pos		: SV_POSITION;
	float3	WSPos	: POSWORLD;
	float3	Nrm		: NORMAL;
	float4	Color	: COLOR;
};

struct GSOut
{
	float4	Pos		: SV_POSITION;
	float2	Tex		: TEXCOORD;
	float4	Color	: COLOR;
};

[maxvertexcount(4)]
void main(line DSOut input[2], inout TriangleStream<GSOut> output)
{
	GSOut element;
	element.Pos.zw = input[0].Pos.zw;
	element.Color = input[0].Color;

	for (uint i = 0; i < 4; ++i)
	{
		element.Tex = float2(i & 1, i >> 1);
		const float2 vOffset = element.Tex * float2(2.0, -2.0) + float2(-1.0, 1.0);
		element.Pos.xy = input[0].Pos.xy + vOffset * 0.2;

		output.Append(element);
	}

	output.RestartStrip();
}
