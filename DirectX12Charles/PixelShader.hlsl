
cbuffer CBuf : register(b1)
{
   float4 face_color[6];
};

float4 main(uint tid : SV_PrimitiveID) : SV_TARGET
{
   //return float4(1.0f, 0.0f, 0.5f, 1.0f);
   return face_color[tid /2];
}
