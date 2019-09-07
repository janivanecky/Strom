struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 texcoord : TEXCOORD;
	uint instanceID : SV_InstanceID;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float4 color: COLOR;
	float fontSize : SIZE;
};

cbuffer ProjectionMatrix: register(b5)
{
	matrix projection;
};

cbuffer TextData: register(b7)
{
	matrix model[500];
	float4 color[500];
	float4 sourceRect[500];
	float4 fontSizes[500];
}

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	uint id = input.instanceID;
	output.pos = mul(projection, mul(model[id], float4(input.pos, 1.0f)));
	float2 texcoord = input.texcoord;
	texcoord.x = texcoord.x * sourceRect[id].z + sourceRect[id].x;
	texcoord.y = texcoord.y * sourceRect[id].w + sourceRect[id].y;
	output.texCoord = texcoord;
	output.color = color[id];
	output.fontSize = fontSizes[id].x;
	return output;
}