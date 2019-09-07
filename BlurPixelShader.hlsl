SamplerState texSampler : register(s0);
SamplerState clampSampler : register(s1);
Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D depthTexture : register(t2);

struct PixelInput
{
	float4 position: SV_POSITION;
	float4 screenPos: POSITION;
	float2 texcoord: TEXCOORD;
};

cbuffer screenBuffer: register(b3)
{
	float2 screenSize;
};

float4 main(PixelInput input) : SV_TARGET
{

	float2 texcoord = input.texcoord;
	float2 texelSize = 1.0 / float2(screenSize.x, screenSize.y);
	float4 color = colorTexture.Sample(clampSampler, texcoord);
	float depth = depthTexture.Sample(clampSampler, texcoord).r;
	float3 normal = normalTexture.Sample(clampSampler, texcoord).rgb * 2.0f - 1.0f;

	float2 hlim = float2(-4.0f * 0.5 + 0.5, -4.0f * 0.5 + 0.5);
	float sum = 1.0f;
	for (int i = -2; i <= 2; ++i) {
		for (int j = -2; j <= 2; ++j) {
			if (i == 0 && j == 0)
				continue;
			float2 offset = float2(i,j) * texelSize;

			const float BlurSigma = float(2) * 0.5;
			const float BlurFalloff = 1.0 / (2.0*BlurSigma*BlurSigma);

			float d = depthTexture.Sample(clampSampler, texcoord + offset).r;
			float ddiff = (d - depth) * 40.0f;
			float r = max(abs(i), abs(j));
			float w = exp2(-r * r *BlurFalloff - ddiff*ddiff);

			sum += w;
			color += colorTexture.Sample(clampSampler, texcoord + offset) * w;
		}
	}
	color = color / sum;
	return color;
}