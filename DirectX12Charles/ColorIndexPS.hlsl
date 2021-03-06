struct ColorType
{
   float4 face_colors[6];
};
ConstantBuffer <ColorType> buf: register(b1);

float4 main(uint tid : SV_PrimitiveID) : SV_TARGET
{
   return buf.face_colors[(tid / 2) % 6];
}
