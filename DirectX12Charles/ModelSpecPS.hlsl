#include "PointLight.hlsli"
ConstantBuffer <PointLightType> light: register(b1);

#include "Material.hlsli"
ConstantBuffer <MaterialType> material: register(b2);

#include "LightVectorData.hlsli"
#include "ShaderOps.hlsli"

Texture2D tex : register(t0);
Texture2D spec : register(t1);
Texture2D nmap : register(t2);
SamplerState s1 : register(s0);

float4 main(float3 viewPos : Position, float3 viewNormal : Normal, float2 tc : Texcoord) : SV_Target
{
   // normalize the mesh normal
   viewNormal = normalize(viewNormal);

   // fragment to light vector data
   const LightVectorData lv = CalculateLightVectorData(light.viewLightPos, viewPos);

   float3 specularReflectionColor;
   float specularPower = material.specularPower;
   const float4 specularSample = spec.Sample(s1, tc);
   // calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
   specularReflectionColor = specularSample.rgb * material.specularWeight;
   if (material.hasGloss)
   {
      specularPower = pow(2.0f, specularSample.a * 13.0f);
   }

   // attenuation
   const float att = Attenuate(light.attConst, light.attLin, light.attQuad, lv.distToL);

   // diffuse intensity
   const float3 diffuse = Diffuse(light.diffuseColor, light.diffuseIntensity, att, lv.dirToL, viewNormal);

   // specular reflected
   const float3 specularReflected = Speculate(
      specularReflectionColor, 1.0f, viewNormal,
      lv.vToL, viewPos, att, specularPower);

   // final color
   return float4(saturate((diffuse + light.ambient) * tex.Sample(s1, tc).rgb + specularReflected), 1.0f);
}
