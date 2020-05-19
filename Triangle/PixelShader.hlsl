struct VSOutput
{
	float4 Position : POSITION;
	float4 Color : COLOR;
};

float4 main(VSOutput In) : SV_TARGET
{
	return In.Color;
}