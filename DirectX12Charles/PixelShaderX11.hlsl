cbuffer CBuf
{
	float4 face_colors[6];
};

float4 main(uint tid : SV_PrimitiveID) : SV_Target
//float4 main() : SV_Target
{
	return face_colors[tid / 2];
	//	return float4(1.0f, 0.0f, 0.5f, 1.0f);
}