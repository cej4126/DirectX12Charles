#include "PointLight.hlsli"
ConstantBuffer <PointLightType> light: register(b1);

#include "Material.hlsli"
ConstantBuffer <MaterialType> material: register(b2);

#include "Transform.hlsli"
ConstantBuffer <TransformType> transform: register(b0);

Texture2D tex : register(t0);
Texture2D nmap : register(t2);
SamplerState s1 : register(s0);

#include "ShaderOps.hlsli"
#include "LightVectorData.hlsli"

float4 main(float3 viewPos : Position, float3 viewNormal : Normal, float3 viewTan : Tangent, float3 viewBitan : Bitangent, float2 tc : Texcoord) : SV_Target
{
   viewNormal = normalize(viewNormal);

   if (material.hasNormal)
   {
      viewNormal = MapNormal(normalize(viewTan), normalize(viewBitan), viewNormal, tc, nmap, s1);
   }

   // fragment to light vector data
   const LightVectorData lv = CalculateLightVectorData(light.viewLightPos, viewPos);
   // attenuation
   const float att = Attenuate(light.attConst, light.attLin, light.attQuad, lv.distToL);
   // diffuse intensity
   const float3 diffuse = Diffuse(light.diffuseColor, light.diffuseIntensity, att, lv.dirToL, viewNormal);
   // specular
   const float3 specular = Speculate(
      light.diffuseColor, light.diffuseIntensity, viewNormal,
      lv.vToL, viewPos, att, material.specularPower);

   // final color
   return float4(saturate((diffuse + light.ambient) * tex.Sample(s1, tc).rgb + specular), 1.0f);
}
