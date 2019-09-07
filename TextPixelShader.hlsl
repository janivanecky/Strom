SamplerState texSampler : register(s0);
SamplerState clampSampler : register(s1);
Texture2D fontTexture : register(t8);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float4 color: COLOR;
	float fontSize : SIZE;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float size = input.fontSize / 20.0f;
	float textFadeout = 0.45f / size;
	float textWidth = 0.15f * size / 1.5f + 0.15 / 3.0f;
	float4 textureColor = fontTexture.Sample(texSampler, input.texcoord);
	float distance = 1 - textureColor.a;
	float alpha = 1 - smoothstep(textWidth, textWidth + textFadeout, distance);
	return float4(input.color.xyz, input.color.a * alpha);
}