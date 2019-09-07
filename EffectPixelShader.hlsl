Texture2D colorTexture : register(t0);
SamplerState texSampler : register(s0);
Texture2D randomTexture: register(t5);

struct PixelInput
{
	float4 position: SV_POSITION;
	float4 screenPos: POSITION;
	float2 texcoord: TEXCOORD;
};


cbuffer settingsBuffer: register(b8)
{
	float4 settings;
}

float4 main(PixelInput input) : SV_TARGET
{
	float2 texcoord = input.texcoord;
	float4 color = float4(0,0,0,0);
	for (int y = -1; y <= 1; ++y)
	{
		for (int x = -1; x <= 1; ++x)
		{
			color += colorTexture.Sample(texSampler, texcoord + float2(0.00075f * y, 0.00075f * x) * 0.5f);
		}

	}
	color /= 9.0f;
	float2 pos = input.screenPos.xy;
	float d = length(pos * pos);
	
	color = saturate(color);
	color = pow(color, 1 / 2.2f);
	color.r -= d * 0.45f * settings.z;
	color.g -= d * 0.45f * settings.z;
	color.b -= d * 0.45f * settings.z;
	color = saturate(color);
	color.r = pow(color.r, 1.0f);
	color.g = pow(color.g, 0.9f);
	color.b = pow(color.b, 0.9f);
	return color;
}