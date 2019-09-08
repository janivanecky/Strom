// Authors: Dusan Drevicky, David Pribula, Jan Ivanecky
#include "VoxelSpace.h" 

// TODO(dusan): IsCoordinateInVoxelSpace sa nepouziva, nikdy nenastal pripad, ktory by to opodstatnoval

uint32 IndexFromCoordinate(VoxelSpace *voxelSpace, Vector3i coordinate)
{
	return coordinate.x + coordinate.y * voxelSpace->voxelsPerEdge + coordinate.z * voxelSpace->voxelsPerEdge * voxelSpace->voxelsPerEdge;
}

Vector3i CoordinateFromIndex(VoxelSpace *voxelSpace, uint32 i)
{
	uint32 voxelsPerEdge = voxelSpace->voxelsPerEdge;
	Vector3i coordinate;
	coordinate.x = i % voxelsPerEdge;
	coordinate.y = (i / voxelsPerEdge) % voxelsPerEdge;
	coordinate.z = (i / (voxelsPerEdge * voxelsPerEdge)) % voxelsPerEdge;
	return coordinate;
}

Vector3 PositionFromCoordinate(VoxelSpace *voxelSpace, Vector3i coordinate)
{
	float voxelSize = voxelSpace->voxelSize;
	uint32 voxelsPerEdge = voxelSpace->voxelsPerEdge;
	Vector3 position;
	position.x = coordinate.x * voxelSize - (voxelSize * voxelsPerEdge * 0.5f) + 0.5f * voxelSize;
	position.y = coordinate.y * voxelSize + 0.5f * voxelSize;
	position.z = coordinate.z * voxelSize - (voxelSize * voxelsPerEdge * 0.5f) + 0.5f * voxelSize;
	return position;
}

VoxelSpace Voxels::GetVoxelSpace(WorldConstants *constants)
{
	VoxelSpace voxelSpace;
	// Store constants
	voxelSpace.voxelsPerEdge = constants->voxelsPerEdge > 0 ? (constants->voxelsPerEdge / 2) + 1 : 0;
	voxelSpace.voxelSize = constants->voxelSize;
	voxelSpace.shadowConstantA = constants->shadowConstantA;
	voxelSpace.shadowConstantB = constants->shadowConstantB;

	if (voxelSpace.voxelsPerEdge > 0)
	{
		// Allocate necessary fields
		uint32 voxelSpaceSize = constants->voxelsPerEdge * constants->voxelsPerEdge * constants->voxelsPerEdge;
		voxelSpace.voxels = (Voxel *)malloc(sizeof(Voxel) * voxelSpaceSize);
		voxelSpace.shadowValues = (float *)malloc(sizeof(float) * voxelSpaceSize);

		// Initialize voxel values
		Voxel *voxel = voxelSpace.voxels;
		float *shadowValue = voxelSpace.shadowValues;
		for (uint32 i = 0; i < voxelSpaceSize; i++, voxel++, shadowValue++)
		{
			voxel->coordinate = CoordinateFromIndex(&voxelSpace, i);
			voxel->worldSpaceCenterPosition = PositionFromCoordinate(&voxelSpace, voxel->coordinate);
			*shadowValue = 0;
		}
	}

	return voxelSpace;
}

void Voxels::ClearVoxelShadows(VoxelSpace *voxelSpace)
{
	if (voxelSpace->voxelsPerEdge > 0)
	{
		uint32 voxelCount = voxelSpace->voxelsPerEdge * voxelSpace->voxelsPerEdge * voxelSpace->voxelsPerEdge;
		memset(voxelSpace->shadowValues, 0, voxelCount * sizeof(float));
	}
}

void Voxels::CastVoxelShadow(VoxelSpace* voxelSpace, Vector3 position)
{
	if (voxelSpace->voxelsPerEdge > 0)
	{
		uint32 voxelsPerEdge = voxelSpace->voxelsPerEdge;

		// Retrieve coordinate from tree space position and check if within voxel space
		Vector3i coordinate = CoordinateFromPosition(voxelSpace, position);
		bool isValid = IsCoordinateInVoxelSpace(voxelSpace, coordinate);
		assert(isValid);

		// Compute lowest level to which shadow will be propagated
		uint32 index = IndexFromCoordinate(voxelSpace, coordinate);
		int32 minY = MAX(coordinate.y - MAX_SHADOW_DEPTH, 0);

		for (int32 y = (int32)coordinate.y, depth = 0; y >= minY; --y, ++depth)
		{
			// Compute x-z plane bounds for the current depth level shadow propagation
			uint32 minZ = MAX(coordinate.z - depth, 0);
			uint32 maxZ = MIN(coordinate.z + depth, voxelsPerEdge - 1);
			uint32 minX = MAX(coordinate.x - depth, 0);
			uint32 maxX = MIN(coordinate.x + depth, voxelsPerEdge - 1);

			// Compute shadowing value for the current depth level
			float a = voxelSpace->shadowConstantA;
			float b = voxelSpace->shadowConstantB;
			float shadowValueIncrease = a / Math::Pow(b, float(depth));

			// Apply shadowing value to the current depth level
			for (uint32 z = minZ; z <= maxZ; ++z)
			{
				for (uint32 x = minX; x <= maxX; ++x)	
				{
					uint32 currentVoxelIndex = IndexFromCoordinate(voxelSpace, Vector3i(x, y, z));
					voxelSpace->shadowValues[currentVoxelIndex] = MIN(voxelSpace->shadowValues[currentVoxelIndex] + shadowValueIncrease, 15.f);
				}
			}
		}
	}
}

bool Voxels::IsCoordinateInVoxelSpace(VoxelSpace *voxelSpace, Vector3i coordinate)
{
	int32 voxelsPerEdge = (int32)voxelSpace->voxelsPerEdge;
	if (voxelsPerEdge == 0)
	{
		return true;
	}
	if (coordinate.x < 0 || coordinate.x >= voxelsPerEdge ||
		coordinate.y < 0 || coordinate.y >= voxelsPerEdge ||
		coordinate.z < 0 || coordinate.z >= voxelsPerEdge)
	{
		return false;
	}
	return true;
}

bool Voxels::IsPositionInVoxelSpace(VoxelSpace *voxelSpace, Vector3 position)
{
	Vector3i coordinate = Voxels::CoordinateFromPosition(voxelSpace, position);
	bool valid = Voxels::IsCoordinateInVoxelSpace(voxelSpace, coordinate);
	return valid;
}

float Voxels::ShadowValueFromCoordinate(VoxelSpace *voxelSpace, Vector3i coordinate)
{
	if (voxelSpace->voxelsPerEdge == 0)
	{
		return 0.0f;
	}
	assert(IsCoordinateInVoxelSpace(voxelSpace, coordinate));
	int32 index = IndexFromCoordinate(voxelSpace, coordinate);
	float result = voxelSpace->shadowValues[index];
	return result;
}

float Voxels::ShadowValueFromPosition(VoxelSpace *voxelSpace, Vector3 position)
{
	Vector3i coordinate = CoordinateFromPosition(voxelSpace, position);
	float result = ShadowValueFromCoordinate(voxelSpace, coordinate);
	return result;
}

Vector3 Voxels::VoxelCenterPositionFromCoordinate(VoxelSpace *voxelSpace, Vector3i coordinate)
{
	assert(IsCoordinateInVoxelSpace(voxelSpace, coordinate));
	int32 index = IndexFromCoordinate(voxelSpace, coordinate);
	Vector3 result = voxelSpace->voxels[index].worldSpaceCenterPosition;
	return result;
}

Vector3i Voxels::CoordinateFromPosition(VoxelSpace *voxelSpace, Vector3 position)
{
	uint32 voxelsPerEdge = voxelSpace->voxelsPerEdge;
	float voxelSize = float(voxelSpace->voxelSize);

	// Shift negative position to positive range
	// + voxelSize/2 so that the coordinate 0,0,0 has range [-voxelSize/2, + voxelSize/2] in the x and z directions
	// i.e., the voxelspace is centered on the point 0, 0, 0
	float floatX = position.x + voxelsPerEdge / 2 * voxelSize + voxelSize / 2;	
	float floatY = position.y;
	float floatZ = position.z + voxelsPerEdge / 2 * voxelSize + voxelSize / 2;

	// Normalize value by size of each voxel
	floatX /= voxelSize;
	floatY /= voxelSize;
	floatZ /= voxelSize;

	// Floor to obtain final values
	Vector3i coordinate;
	coordinate.x = (int32)Math::Floor(floatX);
	coordinate.y = (int32)Math::Floor(floatY);
	coordinate.z = (int32)Math::Floor(floatZ);
	return coordinate;
}

void Voxels::Release(VoxelSpace *voxelSpace)
{
	if (voxelSpace->voxelsPerEdge > 0)
	{
		free(voxelSpace->voxels);
		free(voxelSpace->shadowValues);
	}
}