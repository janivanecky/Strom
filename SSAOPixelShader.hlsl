SamplerState texSampler : register(s0);
SamplerState clampSampler : register(s1);
Texture2D normalTexture : register(t1);
Texture2D depthTexture : register(t2);
Texture2D randomTexture: register(t6);
Texture3D noiseTexture: register(t7);

static const int NO_KERNELS = 64;

cbuffer randomVectors: register(b4)
{
	float4 vecs[NO_KERNELS];
};

cbuffer screenBuffer: register(b3)
{
	float2 screenSize;
};


cbuffer matrices: register(b0)
{
	matrix projection;
	matrix view;
	matrix projectionInverse;
};

cbuffer settingsBuffer: register(b8)
{
	float4 settings;
}

struct PixelInput
{
	float4 position: SV_POSITION;
	float4 screenPos: POSITION;
	float2 texcoord: TEXCOORD;
};

float4 main(PixelInput input) : SV_TARGET
{
	float2 texcoord = input.texcoord;
	float2 pos = input.screenPos.xy;

	float depth = depthTexture.Sample(clampSampler, input.texcoord).r;
	float4 ndc = float4(input.screenPos.xy, depth, 1.0f);
	float4 viewSpace = mul(projectionInverse, ndc);
	viewSpace /= viewSpace.w;

	float3 normal = normalize(normalTexture.Sample(clampSampler, texcoord).xyz * 2.0f - 1.0f);
	normal = mul(view, float4(normal, 0.0f)).xyz;
	float3 randomVec = normalize(randomTexture.Sample(texSampler, 
			texcoord * float2(screenSize.x / 4.0f, screenSize.y / 4.0f)).xyz * 2.0f - 1.0f);
	float3 tangent = normalize(randomVec - normal * dot(normal, randomVec));
	float3 bitangent = normalize(cross(normal, tangent));
	float3x3 basis = float3x3(bitangent, normal, tangent);
	basis = transpose(basis);
	
	float occlusionFactor = 1.0f;
	float ssaoRadius = 3.0f;
	for (int i = 0; i < NO_KERNELS && settings.x > 0; ++i)
	{
		float4 sampleDirection = vecs[i];//controls.y;
		sampleDirection = float4(mul(basis, sampleDirection.xyz), 0.0f) * ssaoRadius;
		float4 p = viewSpace + sampleDirection;
		p = mul(projection, p);
		p /= p.w;
		float4 ndccoords = float4(p.xy, 0.0f, 1.0f);
		p.y = -p.y;
		p.xy = p.xy * 0.5f + 0.5f;
		float pDepth = depthTexture.Sample(texSampler, p.xy).x;
		ndccoords.z = pDepth;
		float4 viewP = mul(projectionInverse, ndccoords);
		viewP /= viewP.w;
		float rangeCheck = abs(viewSpace.z - viewP.z) < ssaoRadius ? 1.0f : 0.0f;
		if (pDepth <= p.z && depth < 1.0f)
		{
			occlusionFactor -= 2.0f / (float)NO_KERNELS * rangeCheck;
		}
	}
	occlusionFactor = saturate(occlusionFactor);
	return float4(occlusionFactor, occlusionFactor, occlusionFactor, 1.0f);
}