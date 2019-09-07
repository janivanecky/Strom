struct PixelInput
{
	float4 pos: SV_POSITION;
	float4 worldPos: POSITION;
	float4 norm: NORM;
	float4 add: ADDITIONAL;
};

cbuffer Material: register(b2)
{
	float4 ambientColor;
	float4 diffuseColor;
	float4 specularColor;
	float roughness;
};

struct PixelOutput
{
	float4 normal : SV_TARGET0;
	float4 diffuse : SV_TARGET1;
	float4 specular : SV_TARGET2;
};

static const float PI = 3.14159265359;

PixelOutput main(PixelInput input)
{
	PixelOutput output;

	output.normal = float4(input.norm.xyz * 0.5f + 0.5f, 0.0f);
	output.diffuse = diffuseColor + input.add;
	output.specular = specularColor;
	output.specular.w = roughness;
	
	return output;
}