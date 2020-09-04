struct CBuf
{
   float4 color;
};
ConstantBuffer <CBuf> buf: register(b1);

float4 main() : SV_Target
{
//   return buf.color;
   return float4(1.0f, 0.0f, 0.0f, 1.0f);
}