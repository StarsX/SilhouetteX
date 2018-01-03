struct PSIn
{
	float4	Pos		: SV_POSITION;
	float3	WSPos	: POSWORLD;
	float3	Nrm		: NORMAL;
	float4	Color	: COLOR;
};

float4 main(PSIn input) : SV_TARGET
{
	return input.Color;
}
