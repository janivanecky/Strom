struct VertexOutput
{
	float4 position: SV_POSITION;
	float4 screenPos: POSITION;
	float2 texcoord: TEXCOORD;
};

struct VertexInput
{
	float4 position: POSITION;
	float2 texcoord: TEXCOORD;
};

VertexOutput main(VertexInput input)
{
	VertexOutput result;
	result.position = input.position;
	result.screenPos = input.position;
	result.texcoord = input.texcoord;
	return result;
}