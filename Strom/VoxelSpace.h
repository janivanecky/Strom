#pragma once
#include "Def.h"

/*
	Voxels namespace contains functions that manipulate voxel space (voxel grid),
	which is used for computing lightning conditions for the tree growth.
*/
namespace Voxels
{
	// VoxelSpace initializer
	VoxelSpace GetVoxelSpace(WorldConstants *constants);

	// Cast shadow on Voxels from specified position
	void CastVoxelShadow(VoxelSpace *voxelSpace, Vector3 position);

	// Clear shadow values from all the voxels
	void ClearVoxelShadows(VoxelSpace *voxelSpace);

	// Retrieve shadow value from voxel occupying specified tree space position
	float ShadowValueFromPosition(VoxelSpace *voxelSpace, Vector3 position);

	// Retrieve shadow value from voxel at specified coordinate
	float ShadowValueFromCoordinate(VoxelSpace *voxelSpace, Vector3i coordinate);
	
	// Compute voxel space coordinate from tree space position
	Vector3i CoordinateFromPosition(VoxelSpace *voxelSpace, Vector3 position);
	
	// Get position of center of the voxel at specified coordinate
	Vector3 VoxelCenterPositionFromCoordinate(VoxelSpace *voxelSpace, Vector3i coordinate);
	
	// Check if specified coordinate is within voxel space
	bool IsCoordinateInVoxelSpace(VoxelSpace *voxelSpace, Vector3i coordinate);

	// Check if specified tree space position is within voxel space
	bool IsPositionInVoxelSpace(VoxelSpace *voxelSpace, Vector3 position);

	// Release voxel space
	void Release(VoxelSpace *voxelSpace);
}