
struct VS_OUTPUT
{
   float4 position : SV_POSITION;
   float4 color : Color;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
   return input.color;
}
