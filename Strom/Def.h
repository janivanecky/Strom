// Authors: Dusan Drevicky, David Pribula, Jan Ivanecky
#pragma once
#include <stdint.h>
#include "../Math.h"
#include <cassert>
#include <stack>
#include <queue>

typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

#define MAX_BUDS_FROM_INTERNODE 2
#define MAX_SHADOW_DEPTH 4

struct WorldConstants
{
	float basicLength;
	float basicWidth;

	float fullLightExposure;
	float shadowConstantA;
	float shadowConstantB;
	float canopyHeight;

	uint32 voxelsPerEdge;
	uint32 voxelSize;
};

struct TreeParameters
{
	float phyllotacticAngle;
	float branchingAngle;
	float deflectionAngle;
	float tropismAngle;
	float widthPower;
	float shedThreshold;
	float resourceThreshold;

	float resourceToLightRatio;

	uint32_t tropismMinOrder;
	uint32_t apicalControlMaxOrder;

	float defaultOrientationWeight;
	float environmentOrientationWeight;
	float tropismOrientationWeight;

	float lambda;

	float verticalBudPreference;
	float horizontalBudPreference;
	float upwardBudPreference;
};

struct Bud;
struct Internode
{
	Vector3 startingPosition;
	Vector3 endingPosition;
	float width;
	float resource;
	float light;
	uint32 descendantInternodesCount; // Number of all of the descendant internodes (variable is used in shedding)
	uint32 nextLateralInternodesCount;
	uint32 ID;
	uint32 order;
	int32 accessCounter;
	bool isEnqueued;

	Internode *previous;
	Internode *nextInternodes[5];
	Bud *buds[MAX_BUDS_FROM_INTERNODE];
};

struct Bud
{
	Vector3 position;
	Vector3 direction;
	Vector3 lastLateralDirection;
	float lightExposure;
	int32 age;
	Internode *internode;

	uint32 ID;

};

template <typename T>
struct TBuffer
{
	T *items;
	uint32 size;
	uint32 count;

	T& operator[](uint32 index)
	{
		return this->items[index];
	}
};

namespace Buffer
{
	template <typename T>
	void Init(TBuffer<T> *buffer, uint32 size)
	{
		buffer->items = (T *)malloc(sizeof(T) * size);
		buffer->count = 0;
		buffer->size = size;
	}

	template <typename T>
	void Add(TBuffer<T> *buffer, T item)
	{
		assert(buffer->count < buffer->size);
		buffer->items[buffer->count++] = item;
	}

	template <typename T>
	void Release(TBuffer<T> *buffer)
	{
		free(buffer->items);
	}
}

struct TreeShell
{
	WorldConstants constants;
	TreeParameters parameters;

	uint32_t internodeCount;
	uint32_t connectionCount;
	Internode *internodes;
	uint32_t *parentIndices;
	uint32_t *budIndices;
	float *parentBranchPart;
	float *endWidths;

	TBuffer<Bud *> budsBuffer;
};

struct TreeMesh
{
	void *vertices;
	uint32 vertexCount;
	uint32 vertexStride;
	uint32 *indices;
	uint32 indexCount;
};

struct Voxel
{
	Vector3i coordinate;	// An unsigned (if it is valid) integer coordinate into the 3D model of the VoxelSpace (positively offset)
	Vector3 worldSpaceCenterPosition;	// Position of the center of the voxel in the world shared with the tree. Used for environment vector calculation.
};

// x coordinate then y then y
// saves only in unsigned values (add half of length to X and Z (Y) always unsigned ).

struct VoxelSpace
{
	Voxel *voxels;
	float *shadowValues;

	uint32 voxelSize;
	uint32 voxelsPerEdge;

	float shadowConstantA;
	float shadowConstantB;
};

struct Tree
{
	Internode* root;
	TBuffer<Bud *> budsBuffer;
	VoxelSpace voxelSpace;
	TreeParameters parameters;
	WorldConstants constants;
	uint32 maxInternodesFromBud;
	uint32 iteration;
	uint32 shedTotal;
	uint32 internodeCount;
	float maxResourceInTerminalInternode;

};

