struct VertexInput
{
	float4 pos: POSITION;
	float4 normal: NORMAL;
	float4 additional: ADDITIONAL;
};

struct VertexOutput
{
	float4 pos: SV_POSITION;
	float4 worldPos: POSITION;
	float4 norm: NORM;
	float4 add: ADDITIONAL;
};

cbuffer modelBuffer: register(b7)
{
	matrix model;
}

cbuffer PVBuffer : register(b0)
{
	matrix projection;
	matrix view;
	matrix inverseProjection;
};

VertexOutput main(VertexInput input)
{
	VertexOutput result;
	result.pos = mul(projection, mul(view, mul(model, input.pos)));
	result.worldPos = mul(model, input.pos);
	result.norm = mul(model, input.normal);
	result.add = input.additional;
	return result;
}