struct ColorType
{
   float4 color;
};
ConstantBuffer <ColorType> color: register(b1);

float4 main() : SV_Target
{
   return color.color;
}