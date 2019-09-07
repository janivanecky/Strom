#pragma once
#include "Math.h"

#ifndef CUBE_MESH
Vector3 normal = Math::Normalize(Vector3(0, 0.5f * 1.0f / Math::Sqrt(3.0f), Math::Sqrt(2.0f / 3.0f)));
Vector3 normal2 = Math::Normalize(Vector3(-Math::Sqrt(2.0f / 3.0f) * Math::Sqrt(3) / 2.0f,
										  0.5f * 1.0f / Math::Sqrt(3.0f), -0.5f * Math::Sqrt(2.0f / 3.0f)));

float a = Math::Sqrt(3.0f) / 2.0f - 0.5f;
float b = -a + 1.0f / Math::Sqrt(3.0f);
float y = Math::Sqrt(2.0f / 3.0f) - 0.5f;
float d = a - 1.0f / Math::Sqrt(3.0f);
Vector3 check = Math::Normalize(Math::CrossProduct(Vector3(0.5f, -0.5f, 0.5f) - Vector3(-0.5f, -0.5f, 0.5f),
												   Vector3(0.0f, y, b) - Vector3(-0.5f, -0.5f, 0.5f)));
Vector3 check2 = Math::Normalize(Math::CrossProduct(Vector3(-0.5f, -0.5f, 0.5f) - Vector3(0.0f, -0.5f, -a),
													Vector3(0.0f, y, b) - Vector3(0.0f, -0.5f, -a)));
float x = 12;

float vertices[] = {
	// down
	-0.5f, -0.5f, 0.5f + d, 1.0f,
	0.0f, -1.0f, 0.0f, 0.0f,
	0.5f, -0.5f, 0.5f + d, 1.0f,
	0.0f, -1.0f, 0.0f, 0.0f,
	0.0f, -0.5f, -a + d, 1.0f,
	0.0f, -1.0f, 0.0f, 0.0f,

	// front
	-0.5f, -0.5f, 0.5f + d, 1.0f,
	normal.x, normal.y, normal.z, 0.0f,
	0.0f, y, b + d, 1.0f,
	normal.x, normal.y, normal.z, 0.0f,
	0.5f, -0.5f, 0.5f + d, 1.0f,
	normal.x, normal.y, normal.z, 0.0f,

	// left
	-0.5f, -0.5f, 0.5f + d, 1.0f,
	normal2.x, normal2.y, normal2.z, 0.0f,
	0.0f, -0.5f, -a + d, 1.0f,
	normal2.x, normal2.y, normal2.z, 0.0f,
	0.0f, y, b + d, 1.0f,
	normal2.x, normal2.y, normal2.z, 0.0f,

	// right
	0.5f, -0.5f, 0.5f + d, 1.0f,
	-normal2.x, normal2.y, normal2.z, 0.0f,
	0.0f, y, b + d, 1.0f,
	-normal2.x, normal2.y, normal2.z, 0.0f,
	0.0f, -0.5f, -a + d, 1.0f,
	-normal2.x, normal2.y, normal2.z, 0.0f,
};

uint32_t stride = sizeof(float) * 4 * 2, vertexCount = 12;

uint16_t indices[] = {
	0, 1, 2,
	3, 4, 5,
	6, 7, 8,
	9, 10, 11
};
uint32_t indexCount = ARRAYSIZE(indices);

float quadVertices[] =
{
	-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	-1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 
	1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f
};

int16_t quadIndices[] =
{
	2,1,0,
	0,3,2,
};

float rectangleVertices[] =
{
	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f
};

int16_t rectangleIndices[] =
{
	0,1,2,
	2,3,0,
};

float size = 400.0f;
float floorVertices[] =
{
	-1.0f * size, 0.0f, -1.0f * size, 1.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	-1.0f * size, 0.0f, 1.0f * size, 1.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	1.0f * size, 0.0f, 1.0f * size, 1.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	1.0f * size, 0.0f, -1.0f * size, 1.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
};

int16_t floorIndices[] =
{
	0,1,2,
	2,3,0,
};

float dodecahedronVertices_[] =
{
	-0.57735f, -0.57735f,  0.57735f, 1.0f,
	0.934172f,  0.356822f,  0.0f, 1.0f,
	0.934172f, -0.356822f,  0.0f, 1.0f,
	-0.934172f,  0.356822f,  0.0f, 1.0f,
	-0.934172f, -0.356822f,  0.0f, 1.0f,
	0.0f,  0.934172f,  0.356822f, 1.0f,
	0.0f,  0.934172f, -0.356822f, 1.0f,
	0.356822f,  0.0f, -0.934172f, 1.0f,
	-0.356822f,  0.0f, -0.934172f, 1.0f,
	0.0f, -0.934172f, -0.356822f, 1.0f,
	0.0f, -0.934172f,  0.356822f, 1.0f,
	0.356822f,  0.0f,  0.934172f, 1.0f,
	-0.356822f,  0.0f,  0.934172f, 1.0f,
	0.57735f,  0.57735f, -0.57735f, 1.0f,
	0.57735f,  0.57735f,  0.57735f, 1.0f,
	-0.57735f,  0.57735f, -0.57735f, 1.0f,
	-0.57735f,  0.57735f,  0.57735f, 1.0f,
	0.57735f, -0.57735f, -0.57735f, 1.0f,
	0.57735f, -0.57735f,  0.57735f, 1.0f,
	-0.57735f, -0.57735f, -0.57735f, 1.0f,
};

uint16_t dodecahedronIndices_[] =
{
	19,  3 , 2	,
	12,  19,  2	,
	15,  12,  2	,
	8 , 14 , 2	,
	18,  8 , 2	,
	3 , 18 , 2	,
	20,  5 , 4	,
	9 , 20 , 4	,
	16,  9 , 4	,
	13,  17,  4	,
	1 , 13 , 4	,
	5 , 1  ,4	,
	7 , 16 , 4	,
	6 , 7  ,4	,
	17,  6 , 4	,
	6 , 15 , 2	,
	7 , 6  ,2	,
	14,  7 , 2	,
	10,  18,  3	,
	11,  10,  3	,
	19,  11,  3	,
	11,  1 , 5	,
	10,  11,  5	,
	20,  10,  5	,
	20,  9 , 8	,
	10,  20,  8	,
	18,  10,  8	,
	9 , 16 , 7	,
	8 , 9  ,7	,
	14,  8 , 7	,
	12,  15,  6	,
	13,  12,  6	,
	17,  13,  6	,
	13,  1 , 11	,
	12,  13,  11,
	19,  12,  11,
};

void GetDodecahedron(float **vertices, uint16_t **indices)
{
	*vertices = (float*)malloc(sizeof(float) * 8 * 36 * 3);
	*indices = (uint16_t *)malloc(sizeof(uint16_t) * 3 * 36);
	float *currentVertices = *vertices;
	uint16_t *currentIndices = *indices;
	for (uint32_t i = 0; i < ARRAYSIZE(dodecahedronIndices_); i += 3)
	{
		uint16_t index1 = dodecahedronIndices_[i + 2] - 1;
		uint16_t index2 = dodecahedronIndices_[i + 1] - 1;
		uint16_t index3 = dodecahedronIndices_[i] - 1;
		float *vertex1 = &dodecahedronVertices_[index1 * 4];
		float *vertex2 = &dodecahedronVertices_[index2 * 4];
		float *vertex3 = &dodecahedronVertices_[index3 * 4];

		Vector3 v1 = Vector3(vertex1[0], vertex1[1], vertex1[2]);
		Vector3 v2 = Vector3(vertex2[0], vertex2[1], vertex2[2]);
		Vector3 v3 = Vector3(vertex3[0], vertex3[1], vertex3[2]);

		Vector3 normal_ = Math::Normalize(Math::CrossProduct(v3 - v1, v2 - v1));
		float normal[4] = { normal_.x, normal_.y, normal_.z, 0.0f };

		memcpy(currentVertices, vertex1, sizeof(float) * 4);
		currentVertices += 4;
		memcpy(currentVertices, normal, sizeof(float) * 4);
		currentVertices += 4;
		memcpy(currentVertices, vertex2, sizeof(float) * 4);
		currentVertices += 4;
		memcpy(currentVertices, normal, sizeof(float) * 4);
		currentVertices += 4;
		memcpy(currentVertices, vertex3, sizeof(float) * 4);
		currentVertices += 4;
		memcpy(currentVertices, normal, sizeof(float) * 4);
		currentVertices += 4;

		*(currentIndices++) = i;
		*(currentIndices++) = i + 1;
		*(currentIndices++) = i + 2;
	}
	int x = 12;
};

float *dodecahedronVertices;
uint16_t *dodecahedronIndices;
uint32_t dodecahedronVertexCount = 36 * 3, dodecahedronIndexCount = 108;

#else 
float cubeVertices[] =
{
	// FRONT
	-0.5f, -0.5f, 0.5f, 1.0f,
	0,0,1,0,
	-0.5f, 0.5f, 0.5f, 1.0f,
	0,0,1,0,
	0.5f, 0.5f, 0.5f, 1.0f,
	0,0,1,0,
	0.5f, -0.5f, 0.5f, 1.0f,
	0,0,1,0,
	// LEFT
	-0.5f, -0.5f, -0.5f, 1.0f,
	-1,0,0,0,
	-0.5f, 0.5f, -0.5f, 1.0f,
	-1,0,0,0,
	-0.5f, 0.5f, 0.5f, 1.0f,
	-1,0,0,0,
	-0.5f, -0.5f, 0.5f, 1.0f,
	-1,0,0,0,
	// RIGHT
	0.5f, -0.5f, 0.5f, 1.0f,
	1,0,0,0,
	0.5f, 0.5f, 0.5f, 1.0f,
	1,0,0,0,
	0.5f, 0.5f, -0.5f, 1.0f,
	1,0,0,0,
	0.5f, -0.5f, -0.5f, 1.0f,
	1,0,0,0,
	// TOP
	-0.5f, 0.5f, 0.5f, 1.0f,
	0,1,0,0,
	-0.5f, 0.5f, -0.5f, 1.0f,
	0,1,0,0,
	0.5f, 0.5f, -0.5f, 1.0f,
	0,1,0,0,
	0.5f, 0.5f, 0.5f, 1.0f,
	0,1,0,0,
	// BOTTOM
	-0.5f, -0.5f, -0.5f, 1.0f,
	-0,-1,0,0,
	-0.5f, -0.5f, 0.5f, 1.0f,
	-0,-1,0,0,
	0.5f, -0.5f, 0.5f, 1.0f,
	-0,-1,0,0,
	0.5f, -0.5f, -0.5f, 1.0f,
	-0,-1,0,0,
	// BACK
	0.5f, -0.5f, -0.5f, 1.0f,
	0,0,-1,0,
	0.5f, 0.5f, -0.5f, 1.0f,
	0,0,-1,0,
	-0.5f, 0.5f, -0.5f, 1.0f,
	0,0,-1,0,
	-0.5f, -0.5f, -0.5f, 1.0f,
	0,0,-1,0,
};

uint32_t cubeStride = sizeof(float) * 8;
uint32_t cubeVertexCount = 24;

uint16_t cubeIndices[] =
{
	2,1,0,
	3,2,0,

	6,5,4,
	7,6,4,

	10,9,8,
	11,10,8,

	14,13,12,
	15,14,12,

	18,17,16,
	19,18,16,

	22,21,20,
	23,22,20
};

uint32_t cubeIndexCount = 36;
#endif