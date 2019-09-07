struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 texcoord : TEXCOORD;
	uint instanceID : SV_InstanceID;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float4 color: COLOR;
};

cbuffer ProjectionMatrix: register(b5)
{
	matrix projection;
};

cbuffer RectData: register(b7)
{
	matrix model[500];
	float4 color[500];
}

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	uint id = input.instanceID;
	output.pos = mul(projection, mul(model[id], float4(input.pos, 1.0f)));
	output.color = color[id];
	return output;
}