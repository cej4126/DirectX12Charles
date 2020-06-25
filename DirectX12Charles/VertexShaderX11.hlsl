struct VSOut
{
	float3 color : COLOR;
	float4 pos : SV_POSITION;
};

VSOut main( float2 pos : POSITION, float3 color : COLOR )
{
	VSOut vso;
	vso.pos = float4(pos.x, pos.y, 0.0f, 1.0f);
//	vso.color = color;
	vso.color = float3(1.0f, 1.0f, 1.0f);
	return vso;
}