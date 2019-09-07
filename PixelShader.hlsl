cbuffer CamBuffer: register(b1)
{
	float4 camPos;
	float4 lightPosition;
};

struct PixelInput
{
	float4 position: SV_POSITION;
	float4 screenPos: POSITION;
	float2 texcoord: TEXCOORD;
};

static const float PI = 3.14159265359;

float4 Shlick(float4 F0, float3 l, float3 h)
{
	return F0 + (float4(1, 1, 1, 1) - F0) * pow(1 - dot(l, h), 5);
}

float TR(float alpha, float3 n, float3 h)
{
	float alpha2 = alpha * alpha;
	float nominator = alpha2;
	float denominator = dot(n, h);
	denominator = denominator * denominator * (alpha2 - 1) + 1;
	denominator = denominator * denominator * PI;
	return nominator / denominator;
}

float GGXSmith1(float3 v, float3 n, float alpha)
{
	alpha = alpha / 2.0f;
	float NV = dot(n, v);
	float nominator = (NV);
	float denominator = NV * (1 - alpha) + alpha;
	return nominator / denominator;
}

float GGXSmith(float3 l, float3 v, float3 n, float alpha)
{
	return GGXSmith1(l, n, alpha) * GGXSmith1(v, n, alpha);
}

float4 BRDF(float3 n, float3 l, float3 v, float4 specularColor, float4 diffuseColor, 
			float roughness)
{
	float nl = dot(n, l);
	float nv = dot(n, v);
	float3 h = normalize(l + v);

	float4 F = Shlick(specularColor, l, h);
	float D = TR(roughness * roughness, n, h);
	float G = GGXSmith(l, v, n, roughness);
	float4 specBRDF = F * G * D / (4 * nl * nv);

	float4 diffuseCoef = 1 - F;
	float4 diffBRDF = diffuseColor * diffuseCoef / PI;

	return diffBRDF + specBRDF;
}

cbuffer matrices: register(b0)
{
	matrix projection;
	matrix view;
	matrix projectionInverse;
};

cbuffer matrices2: register(b6)
{
	matrix shadowMVP;
	matrix inverseV;
}

cbuffer settingsBuffer: register(b8)
{
	float4 settings;
}


SamplerState texSampler : register(s0);
SamplerState clampSampler : register(s1);
Texture2D ssaoTexture: register(t0);
Texture2D normalTexture : register(t1);
Texture2D depthTexture : register(t2);
Texture2D shadowMap : register(t3);
Texture2D diffuseMap : register(t4);
Texture2D specularMap : register(t5);

float3 applyFog(float3 rgb, float distance)
{
	float fogAmount = saturate((distance - 250.0f) / 50.0f);
	//fogAmount = 1.0 - exp(-distance*0.0001f);
	float3 fogColor = float3(0.9f, 0.9f, 0.9f) * 3.0f;
	return lerp(rgb, fogColor, fogAmount);
}

float4 main(PixelInput input) : SV_TARGET
{
	float4 specularColor = specularMap.Sample(texSampler, input.texcoord);
	float4 diffuseColor = diffuseMap.Sample(texSampler, input.texcoord);
	float ao = ssaoTexture.Sample(texSampler, input.texcoord).x;
	float roughness = specularColor.w;
	specularColor.w = 1.0f;

	float depth = depthTexture.Sample(texSampler, input.texcoord).r;
	float4 ndc = float4(input.screenPos.xy, depth, 1.0f);
	float4 viewSpace = mul(projectionInverse, ndc);
	viewSpace /= viewSpace.w;
	float4 worldSpace = mul(inverseV, viewSpace);
	float4 shadowSpace = mul(shadowMVP, worldSpace);
	shadowSpace.xyz /= shadowSpace.w;
	shadowSpace.y = -shadowSpace.y;
	shadowSpace.xy = shadowSpace.xy * 0.5f + 0.5f;

	float occlusionFactor = 1.0f;

	const uint sampleCount = 16;
	static float2 poissonDisk[sampleCount] =
	{
		float2(0.2770745f, 0.6951455f),
		float2(0.1874257f, -0.02561589f),
		float2(-0.3381929f, 0.8713168f),
		float2(0.5867746f, 0.1087471f),
		float2(-0.3078699f, 0.188545f),
		float2(0.7993396f, 0.4595091f),
		float2(-0.09242552f, 0.5260149f),
		float2(0.3657553f, -0.5329605f),
		float2(-0.3829718f, -0.2476171f),
		float2(-0.01085108f, -0.6966301f),
		float2(0.8404155f, -0.3543923f),
		float2(-0.5186161f, -0.7624033f),
		float2(-0.8135794f, 0.2328489f),
		float2(-0.784665f, -0.2434929f),
		float2(0.9920505f, 0.0855163f),
		float2(-0.687256f, 0.6711345f)
	};

	if (shadowSpace.x < 1.0f && shadowSpace.y < 1.0f &&
		shadowSpace.x >= 0.0f && shadowSpace.y >= 0.0f &&
		shadowSpace.z < 1.0f && settings.y > 0)
	{
		for (uint i = 0; i < sampleCount; i++)
		{
			if (shadowMap.Sample(clampSampler, shadowSpace.xy + poissonDisk[i] / 700.0f).x <
				shadowSpace.z - 0.001f)
			{
				occlusionFactor -= 1.0f / sampleCount;
			}
		}
	}

	float3 normal = normalize(normalTexture.Sample(texSampler, input.texcoord).xyz * 2.0f - 1.0f);
	float3 camDir = normalize(camPos.xyz - worldSpace.xyz);

	float3 lightPos = lightPosition.xyz;
	float3 reflectionRay = reflect(camDir, normal);
	float3 centerToRay = dot(lightPos, reflectionRay) * reflectionRay - lightPos;
	float3 closestPoint = lightPos + centerToRay * saturate(500.0f / length(centerToRay));
	lightPos = closestPoint;

	float3 lightDir = normalize(lightPos - worldSpace.xyz);

	float lightNormalDot = clamp(dot(normal, lightDir), 0.0f, 1.0f);

	float4 lightColor = float4(1, 1, 1, 1) * 7.0f;// 15.0f;// * saturate(1500.0f / (lightPower * lightPower + 1e-8));
	float4 col = lightNormalDot * lightColor * PI * BRDF(normal, lightDir, camDir, specularColor, diffuseColor, roughness) * occlusionFactor;
	col += diffuseColor * ao * 1.0f;
	float distance = length(viewSpace.xyz);
	if (depth > 0.99999f)
	{
		distance = 100000;
	}
	col.xyz = applyFog(col.xyz, distance);
	col.xyz = 1.0f - exp(-col.xyz * 1.0f);
	//else if (input.texcoord.x > 0.6f)
	//{
	//	col.xyz = 1.0f - exp(-col.xyz * 0.5f);
	//}
	//else if (input.texcoord.x > 0.4f)
	//{
	//	col.xyz = 1.0f - exp(-col.xyz * 0.1f);
	//}
	//else if (input.texcoord.x > 0.2f)
	//{
	//	col /= col + 1;
	//}
	//
	//col.w = 1.0f;

	float4 result = col;
//	result = float4(ao, ao, ao, 1.0f);
	return result;
}