struct LightBuf
{
   float3 lightPos;
   float pad1;
   float3 ambient;
   float pad3;
   float3 diffuseColor;
   float pad4;
   float diffuseIntensity;
   float attConst;
   float attLin;
   float attQuad;
};
ConstantBuffer <LightBuf> light: register(b1);

struct MaterialBuf
{
   float4 materialColor;
   float specularIntensity;
   float specularPower;
   int hasNormal;
   float pad;
};
ConstantBuffer <MaterialBuf> material: register(b2);

struct TransformCBuf
{
   matrix modelView;
   matrix modelViewProj;
};
ConstantBuffer <TransformCBuf> transform: register(b0);

Texture2D tex : register(t0);
Texture2D nmap : register(t2);
SamplerState s1 : register(s0);

float4 main(float3 viewPos : Position, float3 viewNormal : Normal, float3 tan : Tangent, float3 bitan : Bitangent, float2 tc : Texcoord) : SV_Target
{
   if (material.hasNormal)
   {
      const float3x3 tanToView = float3x3(
         normalize(tan),
         normalize(bitan),
         normalize(viewNormal));

      const float3 normalSample = nmap.Sample(s1, tc).xyz;
      viewNormal = normalSample * 2.0f - 1.0f;
      viewNormal.y = -viewNormal.y;
      viewNormal.z = normalSample.z;
      viewNormal = mul(viewNormal, tanToView);
   }

   // fragment to light vector data
   const float3 vToL = light.lightPos - viewPos;
   const float distToL = length(vToL);
   const float3 dirToL = vToL / distToL;
   // attenuation
   const float att = 1.0f / (light.attConst + light.attLin * distToL + light.attQuad * (distToL * distToL));
   // diffuse intensity
   const float3 diffuse = light.diffuseColor * light.diffuseIntensity * att * max(0.0f, dot(dirToL, viewNormal));

   // reflected light vector
   const float3 w = viewNormal * dot(vToL, viewNormal);
   const float3 r = w * 2.0f - vToL;
   // calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
   const float3 specular = att * (light.diffuseColor * light.diffuseIntensity) *
         material.specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))), material.specularPower);

   // final color
   return float4(saturate((diffuse + light.ambient) * tex.Sample(s1, tc).rgb + specular), 1.0f);
   //return float4(saturate((diffuse + light.ambient) * tex2.Sample(s1, tc).rgb + specular), 1.0f);
   //return float4(saturate((diffuse + light.ambient) * nmap.Sample(s1, tc).rgb + specular), 1.0f);
}
