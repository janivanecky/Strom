#include "Model.h"
#include "Strom.h"
#include "TreeModel.h"

#define MAIN 0
#define IS_ROOT(internode) (internode->previous == NULL)
#define IS_MAIN(internode, previous) (internode != NULL && previous != NULL && internode->order == previous->order)
#define DIRECTION(internode) (internode->endingPosition - internode->startingPosition)
#define UNIT_DIRECTION(internode) (Math::Normalize(internode->endingPosition - internode->startingPosition))

TreeMesh GetLeafModel()
{
	TreeMesh result;
	result.vertices = malloc(sizeof(float) * 12 * 12);
	result.indices = (uint32 *)malloc(sizeof(uint32) * 12);
	result.vertexStride = sizeof(float) * 12;
	result.vertexCount = 12;
	result.indexCount = 12;

	Vector4 cB = Vector4(0, 0, 0, 1);
	Vector4 cT = Vector4(0, 1, 0, 1);
	Vector4 lM = Vector4(-0.5f, 0.5f, 0, 1);
	Vector4 rM = Vector4(0.5f, 0.5f, 0, 1);

	Vector4 nF = Vector4(0, 0, 1, 0);
	Vector4 nB = Vector4(0, 0, -1, 0);
	Vector4 additional = Vector4(0, 0, 0, 0);

	Vector4 *vertices = (Vector4 *)result.vertices;
	vertices[0] = cB;
	vertices[3] = rM;
	vertices[6] = cT;
	vertices[9] = cB;
	vertices[12] = cT;
	vertices[15] = lM;
	vertices[18] = cB;
	vertices[21] = cT;
	vertices[24] = rM;
	vertices[27] = cB;
	vertices[30] = lM;
	vertices[33] = cT;

	vertices[1] = vertices[4] = vertices[7] = vertices[10] = vertices[13] = vertices[16] = nF;
	vertices[19] = vertices[22] = vertices[25] = vertices[28] = vertices[31] = vertices[34] = nB;
	vertices[2] = vertices[5] = vertices[8] = vertices[11] = vertices[14] = vertices[17] =
		vertices[20] = vertices[23] = vertices[26] = vertices[29] = vertices[32] = vertices[35] = additional;

	for (uint32_t i = 0; i < 12; ++i)
	{
		result.indices[i] = i;
	}

	return result;
}


Vector3 CreateTrunk(float **vertices, uint32 **indices, 
					Vector3 startPosition, Vector3 endPosition, Vector3 up,
					Vector3 direction, float *startWidths, float *endWidths, 
					uint32 sides, uint32 startingIndex)
{
	float angleIncrement = PI2 / (float)sides;
	Vector3 zAxis = Math::Normalize(direction);
	Vector3 yAxis = Math::Normalize(up); 
	Vector3 xAxis = Math::Normalize(Math::CrossProduct(yAxis, zAxis));
	yAxis = Math::Normalize(Math::CrossProduct(zAxis, xAxis));
	Vector4 *currentVertices = (Vector4 *)*vertices;
	uint32 *currentIndices = *indices;
	for (uint32 i = 0; i < sides; ++i)
	{
		float angle1 = angleIncrement * i;
		float angle2 = angleIncrement * ((i + 1) % sides);
		float x1 = Math::Sin(angle1);
		float y1 = Math::Cos(angle1);
		float x2 = Math::Sin(angle2);
		float y2 = Math::Cos(angle2);
		Vector4 s1Position = Vector4(startPosition + x1 * xAxis * startWidths[i] + y1 * yAxis * startWidths[i], 1.0f);
		Vector4 s2Position = Vector4(startPosition + x2 * xAxis * startWidths[(i + 1) % sides] + y2 * yAxis * startWidths[(i + 1) % sides], 1.0f);
		Vector4 e1Position = Vector4(endPosition + x1 * xAxis * endWidths[i] + y1 * yAxis * endWidths[i], 1.0f);
		Vector4 e2Position = Vector4(endPosition + x2 * xAxis * endWidths[(i + 1) % sides] + y2 * yAxis * endWidths[(i + 1) % sides], 1.0f);
		Vector4 normal = Vector4(Math::Normalize(Math::CrossProduct(
			e2Position.xyz - s2Position.xyz, s1Position.xyz - s2Position.xyz)), 0.0f);
		currentVertices[0] = s1Position;
		currentVertices[3] = s2Position;
		currentVertices[6] = e2Position;
		currentVertices[9] = e1Position;
		currentVertices[1] = currentVertices[4] = currentVertices[7] = currentVertices[10] = normal;
		currentVertices[2] = currentVertices[5] = currentVertices[8] = currentVertices[11] = Vector4(0,0,0,0);
		currentVertices += 12;

		uint32 indexBase = startingIndex + i * 4;
		currentIndices[0] = indexBase + 0;
		currentIndices[1] = indexBase + 1;
		currentIndices[2] = indexBase + 2;
		currentIndices[3] = indexBase + 0;
		currentIndices[4] = indexBase + 2;
		currentIndices[5] = indexBase + 3;
		currentIndices += 6;
	}
	*vertices = (float *)currentVertices;
	*indices = currentIndices;
	return yAxis;
}

Vector3 CreateTrunk(float **vertices, uint32 **indices,
					Vector3 startPosition, Vector3 endPosition, Vector3 up,
					Vector3 direction, float startWidth, float endWidth,
					uint32 sides, uint32 startingIndex)
{
	float *startWidths = new float[sides];
	float *endWidths = new float[sides];
	for (uint32_t i = 0; i < sides; ++i)
	{
		startWidths[i] = startWidth;
		endWidths[i] = endWidth;
	}
	Vector3 yAxis = CreateTrunk(vertices, indices, startPosition, endPosition, up, direction, startWidths, endWidths, sides, startingIndex);
	delete[] startWidths;
	delete[] endWidths;
	return yAxis;
}

void CreateNonParallelTrunk(float **vertices, uint32 **indices, 
							Vector3 startPosition, Vector3 startDirection, 
							Vector3 endPosition, Vector3 endDirection,
							Vector3 up, float startWidth, float endWidth, 
							uint32 sides, uint32 startingIndex)
{
	float angleIncrement = PI2 / (float)sides;
	Vector3 zAxisS = Math::Normalize(startDirection);
	Vector3 yAxisS = Math::Normalize(up);
	Vector3 xAxisS = Math::Normalize(Math::CrossProduct(yAxisS, zAxisS));
	yAxisS = Math::Normalize(Math::CrossProduct(zAxisS, xAxisS));
	Vector3 zAxisE = Math::Normalize(endDirection);
	Vector3 yAxisE = Math::Normalize(up);
	Vector3 xAxisE = Math::Normalize(Math::CrossProduct(yAxisE, zAxisE));
	yAxisE = Math::Normalize(Math::CrossProduct(zAxisE, xAxisE));
	Vector4 *currentVertices = (Vector4 *)*vertices;
	uint32 *currentIndices = *indices;
	for (uint32 i = 0; i < sides; ++i)
	{
		float angle1 = angleIncrement * i;
		float angle2 = angleIncrement * ((i + 1) % sides);
		float x1 = Math::Sin(angle1);
		float y1 = Math::Cos(angle1);
		float x2 = Math::Sin(angle2);
		float y2 = Math::Cos(angle2);
		Vector4 s1Position = Vector4(startPosition + x1 * xAxisS * startWidth + y1 * yAxisS * startWidth, 1.0f);
		Vector4 s2Position = Vector4(startPosition + x2 * xAxisS * startWidth + y2 * yAxisS * startWidth, 1.0f);
		Vector4 e1Position = Vector4(endPosition + x1 * xAxisE * endWidth + y1 * yAxisE * endWidth, 1.0f);
		Vector4 e2Position = Vector4(endPosition + x2 * xAxisE * endWidth + y2 * yAxisE * endWidth, 1.0f);
		Vector4 additional = Vector4(0, 0, 0, 0);
		Vector3 dir = e2Position.xyz - s2Position.xyz;
		if (Math::Dot(dir, -startDirection) < 0.0f)
			dir = -dir;

		Vector4 normal = Vector4(Math::Normalize(Math::CrossProduct(
			dir, s1Position.xyz - s2Position.xyz)), 0.0f);

		currentVertices[0] = s1Position;
		currentVertices[3] = s2Position;
		currentVertices[6] = e2Position;
		currentVertices[9] = e1Position;
		currentVertices[1] = currentVertices[4] = currentVertices[7] = currentVertices[10] = normal;
		currentVertices[2] = currentVertices[5] = currentVertices[8] = currentVertices[11] = additional;
		currentVertices += 12;

		uint32 indexBase = startingIndex + i * 4;
		currentIndices[0] = indexBase + 0;
		currentIndices[1] = indexBase + 1;
		currentIndices[2] = indexBase + 2;
		currentIndices[3] = indexBase + 0;
		currentIndices[4] = indexBase + 2;
		currentIndices[5] = indexBase + 3;
		currentIndices += 6;
	}
	*vertices = (float *)currentVertices;
	*indices = currentIndices;
}

float GetRadiusOfNRegPolygonInDirection(Vector3 fullDirection, Vector3 direction, uint32_t sides, float fullRadius)
{
	float angleCosine = Math::Clamp(Math::Dot(Math::Normalize(direction), Math::Normalize(fullDirection)), 0, 1);
	assert(angleCosine >= -1 && angleCosine <= 1.0);
	float alpha_ = PI2 / sides;
	float angle = Math::Fmod(acosf(angleCosine), alpha_);
	float beta = Math::Abs(angle - alpha_ / 2.0f);
	float y = Math::Cos(alpha_ / 2.0f) * fullRadius;
	float x = y / Math::Cos(beta);
	return x;
}

Vector3 CalculateOffset(Internode *internode, Internode *parent, float parentEndWidth, Vector3 up, uint32_t sides)
{
	if (IS_ROOT(internode) || Math::Dot(UNIT_DIRECTION(internode), UNIT_DIRECTION(parent)) >= 0.99f)
	{
		return Vector3(0, 0, 0);
	}
	//assert(parent->endingPosition == internode->startingPosition);

	Vector3 orbitDirection = Math::Normalize(
		UNIT_DIRECTION(internode) - Math::Dot(
			UNIT_DIRECTION(internode), UNIT_DIRECTION(parent)) * UNIT_DIRECTION(parent));

	float x = GetRadiusOfNRegPolygonInDirection(up, orbitDirection, sides, parentEndWidth);
	Vector3 orbitPosition = orbitDirection * x;// parent->width;

	Vector3 zAxis = Math::Normalize(-UNIT_DIRECTION(internode));
	Vector3 yAxis = Math::Normalize(up);
	Vector3 xAxis = Math::Normalize(Math::CrossProduct(yAxis, zAxis));
	yAxis = Math::Normalize(Math::CrossProduct(zAxis, xAxis));

	float internodeWidth = IS_MAIN(internode, parent) ? parentEndWidth : internode->width;
	Vector3 helpAxis = Math::Normalize(
		Math::CrossProduct(orbitDirection, UNIT_DIRECTION(parent)));
	Vector3 offsetDirection = Math::Normalize(
		Math::CrossProduct(helpAxis, DIRECTION(internode)));
	Vector3 offsetDirection_ = Math::Normalize(
		Math::CrossProduct(helpAxis, -DIRECTION(internode)));
	float mx = GetRadiusOfNRegPolygonInDirection(yAxis, offsetDirection_, sides, internodeWidth);
	Vector3 offset = orbitPosition + mx * offsetDirection + UNIT_DIRECTION(parent) * 0.25f;
	return offset;
}

const uint32 sides = 15;
const uint32 verticesPerInternode = sides * 4;
const uint32 indicesPerInternode = sides * 6;
#define INCLUDE_CONNECTIONS
#define INCLUDE_INTERNODES
TreeMesh GetTrunk(TreeShell *shell, Vector3 **upDirectionValues, Vector3 **offsetsDict, uint32_t rootParts = 1, void *preAllocatedVertices = NULL, uint32_t *preAllocatedIndices = NULL)
{
	TreeMesh mesh;

	// Counters and helper arrays
	uint32 vertexCount = 0;
	uint32 indexCount = 0;
	Vector3 *upDirections = (Vector3 *)malloc(sizeof(Vector3) * shell->internodeCount);
#ifdef INCLUDE_INTERNODES
	vertexCount += verticesPerInternode * (shell->internodeCount + rootParts - 1);
	indexCount += indicesPerInternode * (shell->internodeCount + rootParts - 1);
#endif

#ifdef INCLUDE_CONNECTIONS
	vertexCount += verticesPerInternode * shell->connectionCount;
	indexCount += indicesPerInternode * shell->connectionCount;
#endif

	// Allocate mesh
	mesh.vertexCount = vertexCount;
	mesh.indexCount = indexCount;
	mesh.vertexStride = sizeof(float) * 12;
	if (preAllocatedVertices)
	{
		mesh.vertices = preAllocatedVertices;
	}
	else
	{
		mesh.vertices = malloc(sizeof(float) * 12 * mesh.vertexCount);
	}
	
	if (preAllocatedIndices)
	{
		mesh.indices = preAllocatedIndices;
	}
	else
	{
		mesh.indices = (uint32 *)malloc(sizeof(uint32) * mesh.indexCount);
	}

	// Iterate through array of internodes and for each, create mesh
	float *vertices = (float *)mesh.vertices;
	uint32 *indices = mesh.indices;
	uint32 usedVertices = 0;
	Vector3 *offsets = (Vector3 *)malloc(sizeof(Vector3) * shell->internodeCount);

	memset(offsets, 0, sizeof(Vector3) * shell->internodeCount);

	{
		Internode *internode = &shell->internodes[0];
		Vector3 up = Vector3(0, 0, 1);

		float *startingWidths = new float[sides];
		float *endingWidths = new float[sides];
		for (uint32_t i = 0; i < sides; ++i)
		{
			float offset = MIN(-log((rand() % 1000) / 1000.0f) / 0.1f * internode->width * 0.1f, internode->width * 1.5f);
			if (rootParts == 1)
			{
				offset = 0;
			}
			startingWidths[i] = internode->width + offset;
			endingWidths[i] = shell->endWidths[0];
		}

		Vector3 direction = internode->endingPosition - internode->startingPosition;
		float offset = MAX(direction.y, internode->width * 1.5f);
		offsets[0] = Vector3(0, offset - direction.y, 0);
		direction.y = offset;
		for (uint32_t rootPart = 0; rootPart < rootParts; ++rootPart)
		{
			float start = rootPart * (1.0f / rootParts);
			float end = (rootPart + 1) * (1.0f / rootParts);
			Vector3 startingPosition = internode->startingPosition + start * direction;
			Vector3 endingPosition = internode->startingPosition + end * direction;

			float *sws = new float[sides];
			float *ews = new float[sides];
			
			for (uint32_t i = 0; i < sides; ++i)
			{
				float diff = startingWidths[i] - endingWidths[i];
				float k = exp(5) - 1;
				float f1 = log(start * k + 1) / 5.0f;
				float f2 = log(end * k + 1) / 5.0f;
				sws[i] = startingWidths[i] - diff * f1;
				ews[i] = startingWidths[i] - diff * f2;
			}

			upDirections[0] = CreateTrunk(&vertices, &indices, startingPosition, endingPosition, up, -DIRECTION(internode), sws, ews, sides, usedVertices);

			usedVertices += 4 * sides;

			delete[]sws;
			delete[]ews;
		}

		delete[]startingWidths;
		delete[]endingWidths;

	}


	for (uint32 i = 1; i < shell->internodeCount; ++i)
	{
		assert(shell->parentIndices[i] >= 0 && shell->parentIndices[i] < shell->internodeCount);

		Internode *internode = &shell->internodes[i];
		Internode *parent = &shell->internodes[shell->parentIndices[i]];
		Vector3 up = upDirections[shell->parentIndices[i]];

		float t = shell->parentBranchPart[i];
		float parentWidth = t * shell->endWidths[shell->parentIndices[i]] + (1 - t) * parent->width;

		// NOTE: A bit hacky
		if (internode->width > parentWidth)
		{
			internode->width = parentWidth;
			shell->endWidths[i] = internode->width;
		}

		float internodeWidth = IS_MAIN(internode, parent) ? parentWidth : internode->width;

		Vector3 oldOffset = offsets[shell->parentIndices[i]];
		internode->startingPosition += oldOffset;
		internode->endingPosition += oldOffset;
		Vector3 offset = CalculateOffset(internode, parent, parentWidth, up, sides);
		Vector3 startingPosition = internode->startingPosition + offset;
		Vector3 endingPosition = internode->endingPosition + offset;
		offsets[i] = oldOffset + offset;
#ifndef INCLUDE_INTERNODES
		float *vertices_ = vertices;
		uint32 *indices_ = indices;
#endif
		internode->startingPosition += offset;
		internode->endingPosition += offset;
		upDirections[i] = CreateTrunk(&vertices, &indices, startingPosition, 
									  endingPosition, up, -DIRECTION(internode),
									  internodeWidth, shell->endWidths[i], sides, usedVertices);

#ifdef INCLUDE_INTERNODES
		usedVertices += 4 * sides;
#else 
		vertices = vertices_;
		indices = indices_;
#endif
#ifdef INCLUDE_CONNECTIONS
		Vector3 parentEndingPosition = DIRECTION(parent) * shell->parentBranchPart[i] + parent->startingPosition;
		CreateNonParallelTrunk(&vertices, &indices, parentEndingPosition, 
								-DIRECTION(parent), startingPosition, 
								-DIRECTION(internode), up, 
								parentWidth, internodeWidth, sides, usedVertices);
		usedVertices += 4 * sides;
#endif
	}

	if (upDirectionValues)
	{
		*upDirectionValues = upDirections;
	}
	else
	{
		free(upDirections);
	}
	if (offsetsDict)
	{
		*offsetsDict = offsets;
	}
	else
	{
		free(offsets);
	}
	return mesh;
}

void ApplyMatrixToMesh(TreeMesh *mesh, Matrix4x4 matrix)
{
	Vector4 *vertices = (Vector4 *) mesh->vertices;
	for (uint32 v = 0; v < mesh->vertexCount; ++v)
	{
		vertices[v * 3] = matrix * vertices[v * 3];
		vertices[v * 3 + 1] = matrix * vertices[v * 3 + 1];
		vertices[v * 3 + 1].xyz = Math::Normalize(vertices[v * 3 + 1].xyz);
	}
}

TreeMesh PlaceToBuds(TreeShell *shell, TreeMesh *mesh, Vector3 *upDirections = NULL, Vector3 *offsets = NULL,
					 void *preAllocatedVertices = NULL, uint32_t *preAllocatedIndices = NULL, uint32_t indexOffset = 0)
{
	// Allocate memory for vertices and indices
	uint32_t budCount = shell->budsBuffer.count;
	uint32_t internodeCount = shell->internodeCount;

	TreeMesh result;
	result.vertexStride = mesh->vertexStride;
	result.vertexCount = mesh->vertexCount * budCount;
	result.indexCount = mesh->indexCount * budCount;

	Vector4 *vertices;
	if (preAllocatedVertices)
	{
		vertices = (Vector4 *)preAllocatedVertices;
	}
	else
	{
		vertices = (Vector4 *)malloc(sizeof(Vector4) * result.vertexCount * 3);
	}
	uint32 *indices;
	if (preAllocatedIndices)
	{
		indices = preAllocatedIndices;
	}
	else
	{
		indices = (uint32 *)malloc(sizeof(uint32 *) * result.indexCount);
	}

	result.vertices = vertices;
	result.indices = indices;

	uint32_t usedVertices = 0;
	// Place mesh onto every bud
	for (uint32 i = 0; i < budCount; ++i)
	{
		Bud *currentBud = shell->budsBuffer.items[i];

		Vector3 upDirection = upDirections[shell->budIndices[i]];
		Vector3 offset = offsets[shell->budIndices[i]];
		Matrix4x4 translation = Math::GetTranslation(currentBud->position + offset);

		Vector3 zAxis = -currentBud->direction;
		Vector3 yAxis = upDirection;
		float randomAngle = (rand() % 1000) / 1000.0f * PI;
		Matrix4x4 rotation = Math::Invert(Math::LookAt(Vector3(0, 0, 0), zAxis, yAxis)) *
			Math::GetRotation(PIHALF, Vector3(1, 0, 0)) * Math::GetRotation(randomAngle, Vector3(0,1,0));

		float t = Math::Clamp(currentBud->age / 4.0f, 0, 1);
		float s = 1.0f + t * 0.8f;
		Matrix4x4 scale = Math::GetScale(s, s, s);
		Matrix4x4 matrix = translation * rotation * scale;

		memcpy(vertices, mesh->vertices, mesh->vertexCount * sizeof(Vector4) * 3);
		memcpy(indices, mesh->indices, mesh->indexCount * sizeof(uint32));
		Vector4 *appendixVertices = (Vector4 *)vertices;
		
		for (uint32 v = 0; v < mesh->vertexCount; ++v)
		{
			appendixVertices[v * 3] = matrix * appendixVertices[v * 3];
			appendixVertices[v * 3 + 1] = matrix * appendixVertices[v * 3 + 1];
		}
		for (uint32 idx = 0; idx < mesh->indexCount; ++idx)
		{
			indices[idx] += usedVertices + indexOffset;
		}
		vertices += mesh->vertexCount * 3;
		usedVertices += mesh->vertexCount;
		indices += mesh->indexCount;
	}

	return result;
}

void Model::TreeToMesh(Tree *tree, TreeMesh *mesh, TreeMesh *leavesMesh)
{
	TreeShell shell = Strom::GetShell(tree);
	Model::TreeToMesh(&shell, mesh, leavesMesh);
	Strom::Release(&shell);
}

void Model::TreeToMesh(TreeShell *shell, TreeMesh *mesh, TreeMesh *leavesMesh)
{
	Vector3 *trunkUpDirections, *branchUpDirections, *trunkOffsets, *branchOffsets;

	uint32 vertexCount = 0;
	uint32 indexCount = 0;
	uint32_t rootParts = 5;
#ifdef INCLUDE_INTERNODES
	vertexCount += verticesPerInternode * (shell->internodeCount + rootParts - 1);
	indexCount += indicesPerInternode * (shell->internodeCount + rootParts - 1);
#endif

#ifdef INCLUDE_CONNECTIONS
	vertexCount += verticesPerInternode * shell->connectionCount;
	indexCount += indicesPerInternode * shell->connectionCount;
#endif

	float scale = 0.5f;
	Matrix4x4 scaleMini = Math::GetScale(scale, scale, scale);

	TreeShell branchShell;
	TreeMesh branchMesh = {};
	if (leavesMesh)
	{
		Tree branch = TreeModel::GetLeafyBranch(shell->constants, shell->parameters);
		branchShell = Strom::GetShell(&branch);
		Strom::Release(&branch);
		branchMesh = GetTrunk(&branchShell, &branchUpDirections, &branchOffsets);
	}

	uint32_t leafyBranchVertexCount = branchMesh.vertexCount * shell->budsBuffer.count;
	uint32_t leafyBranchIndexCount = branchMesh.indexCount * shell->budsBuffer.count;

	// Allocate mesh
	uint32_t vertexStride = sizeof(float) * 12;
	void *vertices = malloc(vertexStride * (vertexCount + leafyBranchVertexCount));
	void *indices = malloc(sizeof(uint32_t) * (indexCount + leafyBranchIndexCount));
	void *leafyBranchVertices = ((uint8_t *)vertices) + vertexStride * vertexCount;
	void *leafyBranchIndices = ((uint32_t *)indices) + indexCount;

	TreeMesh trunk = GetTrunk(shell, &trunkUpDirections, &trunkOffsets, rootParts, vertices, (uint32_t *)indices);
	if (leavesMesh)
	{
		TreeMesh leafBranches = PlaceToBuds(shell, &branchMesh, trunkUpDirections, trunkOffsets, leafyBranchVertices, (uint32_t *)leafyBranchIndices, vertexCount);
		Strom::Release(&branchMesh);
		TreeMesh leaf = GetLeafModel();
		Matrix4x4 scale = Math::GetScale(0.6f, 0.6f, 0.6f);
		ApplyMatrixToMesh(&leaf, scale);
		TreeMesh leaves = PlaceToBuds(&branchShell, &leaf, branchUpDirections, branchOffsets);
		*leavesMesh = PlaceToBuds(shell, &leaves, trunkUpDirections, trunkOffsets);
		ApplyMatrixToMesh(leavesMesh, scaleMini);
		for (uint32_t i = 0; i < leavesMesh->vertexCount;)
		{
			const float leafVariety = 0.01f;
			float randomOffsetR = Math::Random(-leafVariety, leafVariety);
			float randomOffsetG = Math::Random(-leafVariety, leafVariety);
			float randomOffsetB = Math::Random(-leafVariety, leafVariety);
			for (uint32_t j = 0; j < leaf.vertexCount; ++j, ++i)
			{
				Vector4 *additional = &((Vector4 *)leavesMesh->vertices)[i * 3 + 2];
				additional->x = randomOffsetR;
				additional->y = randomOffsetG;
				additional->z = randomOffsetB;
			}
		}
		Strom::Release(&leaf), Strom::Release(&leaves);
		Strom::Release(&branchShell);
		free(branchUpDirections);
		free(branchOffsets);
	}
	*mesh = trunk;
	mesh->indexCount = indexCount + leafyBranchIndexCount;
	mesh->vertexCount = vertexCount + leafyBranchVertexCount;

	ApplyMatrixToMesh(mesh, scaleMini);
	free(trunkUpDirections);
	free(trunkOffsets);
}

