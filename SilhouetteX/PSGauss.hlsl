struct PSIn
{
	float4	Pos		: SV_POSITION;
	float2	Tex		: TEXCOORD;
	float4	Color	: COLOR;
};

float4 main(PSIn input) : SV_TARGET
{
	return input.Color;
}
