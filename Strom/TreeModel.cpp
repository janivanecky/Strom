// Authors: Dusan Drevicky, David Pribula, Jan Ivanecky
#include "TreeModel.h"
#include "VoxelSpace.h"

#define DBG_FLT_EQ(x, y) (Math::Abs((x) - (y)) < 1.0f)

#define MAIN 0
#define TERMINAL_BUD 0
#define LATERAL_BUD 1
#define IS_TERMINAL_BUD(bud) (bud->age > 0)
#define IS_INACTIVE_BUD(bud) (bud->age == -1)
#define SINGLE_LATERAL_BUD_ON_TERMINAL_INTERNODE(bud) (bud->internode->nextInternodes[0] == NULL && bud->internode->nextLateralInternodesCount == 0 && \
													   bud->internode->buds[0] == NULL)
#define INACTIVE_BUD_AGE -1
#define GET_DIRECTION(internode) (Math::Normalize(internode->endingPosition - internode->startingPosition))

// TODO: Branch collision (especially with the main trunk which is most visible)
// TODO: Ellipse lengths should be somehow normalized so that the gravimorphic factor is scaled appropriately
// TODO: Shed threshold, resource threshold, should be relative values
// TODO: Internode min length
// TODO: MaxApicalControlOrder - lambda
// TODO: rozumna alokacia

#define CANOPY_BOOST_LAMBDA 0.65f
#define CANOPY_SHADOW_STRENGTH 0.35f

float GetCanopyShadowing(Vector3 position, float canopyHeight)
{
	float height = position.y;
	float t = Math::Clamp((height - canopyHeight + 5.0f) / 10.0f, 0, 1);
	float shadowing = (1 - t) * CANOPY_SHADOW_STRENGTH;
	return shadowing;
}

float GetCanopyBoost(Vector3 position, float canopyHeight, float lambda)
{
	float height = position.y;
	float t = Math::Clamp((height - canopyHeight + 5.0f) / 10.0f, 0, 1);
	float finalLambda = (1 - t) * CANOPY_BOOST_LAMBDA + t * lambda;
	return finalLambda;
}

Bud* CreateBud(Tree *model, Vector3 position, Vector3 direction, Vector3 lastLateralDirection, Internode *internode, uint32 age)
{
	Bud *bud = (Bud*)malloc(sizeof(Bud));
	bud->position = position;
	bud->direction = direction;
	bud->internode = internode;
	bud->lastLateralDirection = lastLateralDirection;
	bud->age = age;
	bud->lightExposure = 0.0f;

	if (age > 0 || age == -1)
	{
		internode->buds[0] = bud;
	}
	else
	{
		internode->buds[1] = bud;
	}

	return bud;
}


void ComputeVoxelShadows(Tree *model)
{
	Voxels::ClearVoxelShadows(&model->voxelSpace);
	for (uint32 i = 0; i < model->budsBuffer.count; i++)
	{
		Voxels::CastVoxelShadow(&model->voxelSpace, model->budsBuffer[i]->position);
	}
}

void SpawnLateralBud(Tree *model, Vector3 lateralBudDirection, Vector3 lastLateralDirection, Internode *internode)
{
	Bud *lateralBud = CreateBud(model, internode->endingPosition, lateralBudDirection, lastLateralDirection, internode, 0);
	Buffer::Add(&model->budsBuffer, lateralBud);
}

#include <tuple>
#include <set>
int32 GetRandomIndex(std::vector<std::tuple<Vector3i, float>> indices)
{
	assert(indices.size() > 0);

	uint32 numberOfSameVoxels = indices.size();
	float probability;
	std::set<int32> randomSetIndex;
	float sumOfCoef = 0;

	for (uint32 i = 0; i < indices.size(); i++)
	{
		sumOfCoef = (std::get<1>(indices[i]) + 1);
	}

	while (true)
	{
		int32 shuffle;

		while (randomSetIndex.size() != numberOfSameVoxels)
		{
			shuffle = rand() % numberOfSameVoxels;
			randomSetIndex.insert(shuffle);
		}

		for (std::set<int32>::iterator it = randomSetIndex.begin(); it != randomSetIndex.end(); ++it)
		{
			probability = (std::get<1>(indices[*it]) + 1) / sumOfCoef;

			uint32_t randomNumber = rand() % 100;
			if (randomNumber < probability * 100)
			{
				return *it;
			}
		}
	}

}

// Calculate the vector pointing from bud to the voxel with the lowest shadow value
Vector3 CalculateEnvironmentVector(Tree *model, Vector3 budPosition, Vector3 currentDirection)
{
	VoxelSpace *voxelSpace = &model->voxelSpace;
	uint32 voxelsPerEdge = model->voxelSpace.voxelsPerEdge;

	Vector3i budCoordinate = Voxels::CoordinateFromPosition(voxelSpace, budPosition);

	std::vector<std::tuple<Vector3i, float>> indices;

	float minShadowValue = FLT_MAX;
	Vector3i upDirection = Vector3i(budCoordinate.x, budCoordinate.y + 1, budCoordinate.z);

	int32 minX = MAX(0, budCoordinate.x - 1);
	int32 maxX = MIN(int32(voxelsPerEdge - 1), budCoordinate.x + 1);
	int32 minY = MAX(0, budCoordinate.y - 1);
	int32 maxY = MIN(int32(voxelsPerEdge - 1), budCoordinate.y + 1);
	int32 minZ = MAX(0, budCoordinate.z - 1);
	int32 maxZ = MIN(int32(voxelsPerEdge - 1), budCoordinate.z + 1);

	for (int32 z = minZ; z <= maxZ; z++)
	{
		for (int32 y = minY; y <= maxY; y++)
		{
			for (int32 x = minX; x <= maxX; x++)
			{
				if (Vector3i(x, y, z) != budCoordinate)
				{
					Vector3i currentCoordinate = Vector3i(x, y, z);
					float shadowValue = Voxels::ShadowValueFromCoordinate(voxelSpace, currentCoordinate);
					Vector3 voxelCenterPositionTemp = Voxels::VoxelCenterPositionFromCoordinate(voxelSpace, currentCoordinate);
					shadowValue += GetCanopyShadowing(voxelCenterPositionTemp, model->constants.canopyHeight);
					Vector3 environmentVectorTemp = Math::Normalize(voxelCenterPositionTemp - budPosition);
					shadowValue /= 15.0f; // constant in voxelShadow for clamping

					float coef = ((Math::Dot(currentDirection, environmentVectorTemp) * 0.5f) + 0.5f) * 0.7f + 0.3f;

					if (shadowValue < minShadowValue)
					{
						minShadowValue = shadowValue;
						indices.clear();
						indices.push_back(std::make_tuple(currentCoordinate, coef));
					}
					else if (Math::FloatEqual(shadowValue, minShadowValue) && upDirection != std::get<0>(indices[0]))
					{
						if (y > std::get<0>(indices[0]).y || currentCoordinate == upDirection)
						{
							indices.clear();
							indices.push_back(std::make_tuple(currentCoordinate, coef));
						}
						else if (y == std::get<0>(indices[0]).y)
						{
							indices.push_back(std::make_tuple(currentCoordinate, coef));
						}
					}
				}
			}
		}
	}

	Vector3 result = Vector3(0, 1, 0);
	if (indices.size() > 0)
	{
	
		int32 randomIndex = GetRandomIndex(indices);

		Vector3 voxelCenterPosition = Voxels::VoxelCenterPositionFromCoordinate(voxelSpace, std::get<0>(indices[randomIndex]));
		Vector3 environmentVector = voxelCenterPosition - budPosition;
		result = Math::Normalize(environmentVector);
	}

	return result;
}

// TODO: Angle should be restricted between 0 and PI so that the direction is not backwards
Vector3 CalculateTropismVector(float tropismAngle, Vector3 branchDirection)
{
	Vector3 refAxis = Vector3(0, 1, 0);
	if (Math::Abs(Math::Dot(branchDirection, refAxis)) > 0.95f)
	{
		refAxis = Math::Normalize(Vector3(0, 0, -1));
	}
	Vector3 left = Math::Normalize(Math::CrossProduct(refAxis, branchDirection));
	Matrix4x4 rotation = Math::GetRotation(tropismAngle, left);
	Vector3 result = rotation * Vector3(0, 1, 0);

	assert(result.x == result.x);

	return result;
}

Vector3 CalculateDirectionWithEnvironmentAndTropismVectorAndDeflection(Tree *model, Vector3 budPosition, Vector3 internodeDirection, uint32 internodeOrder,
																	   Vector3 lateralBudDirection)
{
	assert(Math::FloatEqual(Math::Length(internodeDirection), 1));
	TreeParameters *params = &model->parameters;

	// deflection angle
	Vector3 defaultOrientation = internodeDirection;
	if (!Math::FloatEqual(params->deflectionAngle, 0.0f))
	{
		Vector3 rotationAxis = Math::CrossProduct(lateralBudDirection, internodeDirection);
		rotationAxis = Math::Normalize(rotationAxis);
		Matrix4x4 deflectionRotation = Math::GetRotation(params->deflectionAngle, rotationAxis);

		defaultOrientation = deflectionRotation * defaultOrientation;
	}

	Vector3 environmentOrientation = CalculateEnvironmentVector(model, budPosition, internodeDirection);
	Vector3 finalDirection;

	float tropismWeight = 0;
	if (internodeOrder >= params->tropismMinOrder)
		tropismWeight = params->tropismOrientationWeight;
	else if (internodeOrder == params->tropismMinOrder - 1)
		tropismWeight = params->tropismOrientationWeight * 0.5f;
	Vector3 tropismOrientation = CalculateTropismVector(params->tropismAngle, internodeDirection);
	finalDirection = (defaultOrientation * params->defaultOrientationWeight + environmentOrientation * params->environmentOrientationWeight) *
		(1 - tropismWeight) + tropismOrientation * tropismWeight;

	finalDirection = Math::Normalize(finalDirection);
	finalDirection = Math::Normalize(finalDirection * 0.5f + internodeDirection * 0.5f);
	return finalDirection;
}

void CalculateBudLightExposure(Tree *model)
{
	for (uint32 i = 0; i < model->budsBuffer.count; i++)
	{
		Bud *bud = model->budsBuffer[i];
		if (IS_INACTIVE_BUD(bud))
			continue;
		float gravimorphicFactor = 1.0f;
		float budShadowValue = Voxels::ShadowValueFromPosition(&model->voxelSpace, bud->position);
		budShadowValue += GetCanopyShadowing(bud->position, model->constants.canopyHeight);
		if (bud->age > 0)
		{
			float a = model->parameters.horizontalBudPreference;
			float b = model->parameters.verticalBudPreference;// .7f;
			float c = model->parameters.upwardBudPreference;
			float cosTheta = Math::Dot(Math::Normalize(bud->direction), Vector3(0, 1, 0));
			float y = Math::Clamp((cosTheta + c), -1, 1);
			y *= y;
			float x = 1 - y;
			float gravimorphicFactor = a * x + b * y;
			float horizontalFactor = Math::Abs(Math::Dot(GET_DIRECTION(bud->internode), Vector3(0, 1, 0)));
			horizontalFactor *= horizontalFactor;
			gravimorphicFactor = horizontalFactor * 1 + (1 - horizontalFactor) * gravimorphicFactor;
		}
		bud->lightExposure = MAX(model->constants.fullLightExposure + model->constants.shadowConstantA - budShadowValue, 0.01f) * gravimorphicFactor;
	}
}

void IncrementDescendantInternodesCount(Internode *internode)
{
	Internode* tempInternode = internode->previous;
	while (tempInternode)
	{
		tempInternode->descendantInternodesCount++;
		tempInternode = tempInternode->previous;
	}
}

void DecrementDescendantInternodesCount(Internode *internode, uint32 shedInternodeCount)
{
	Internode *tmp = internode;
	while (tmp)
	{
		tmp->descendantInternodesCount -= shedInternodeCount;
		assert(tmp->descendantInternodesCount >= 0);
		tmp = tmp->previous;
	}
}

void CollectLightBudsToRoot(Tree *model)
{
	for (uint32 i = 0; i < model->budsBuffer.count; i++)
	{
		if (IS_INACTIVE_BUD(model->budsBuffer[i]))
			continue;
		Internode *tmp = model->budsBuffer[i]->internode;
		if (tmp)
		{
			float budLightExposure = model->budsBuffer[i]->lightExposure;

			assert(budLightExposure > 0);
			while (tmp)
			{
				tmp->light += budLightExposure;
				tmp = tmp->previous;
			}
		}
	}

	float totalLight = 0.0f;
	for (uint32 i = 0; i < model->budsBuffer.count; i++)
	{
		totalLight += model->budsBuffer[i]->lightExposure;
	}

	if (model->root)
		assert(totalLight == model->root->light);
}

void ShedBranch(Tree *model, Internode *internodeToShed)
{
	// Delete previous internode's reference to the shed one
	Internode *previous = internodeToShed->previous;
	for (uint32 i = 0; i < model->maxInternodesFromBud; i++)
	{
		if (previous->nextInternodes[i] == internodeToShed)
		{
			previous->nextInternodes[i] = NULL;
			if (i > 0)
			{
				--previous->nextLateralInternodesCount;
			}
			break;
		}
	}

	// Shed the internode and its descendants
	uint32 shedCount = 0;
	std::stack<Internode*> internodesToShed;
	internodesToShed.push(internodeToShed);

	while (!internodesToShed.empty())
	{
		Internode *tmp = internodesToShed.top();
		internodesToShed.pop();

		// Push decendants so they can be shed
		for (uint32 i = 0; i < model->maxInternodesFromBud; i++)
		{
			if (tmp->nextInternodes[i])
			{
				internodesToShed.push(tmp->nextInternodes[i]);
			}
		}

		// Delete bud's references to this internode
		assert(MAX_BUDS_FROM_INTERNODE == 2);
		for (uint32 i = 0; i < MAX_BUDS_FROM_INTERNODE; ++i)
		{
			if (tmp->buds[i])
			{
				tmp->buds[i]->internode = NULL;
			}
		}
		free(tmp);
		++shedCount;
	}

	// Decrement the ancestors' descendant count
	DecrementDescendantInternodesCount(previous, shedCount);
}

void CountDirectDescendants(Internode *internode)
{
	internode->accessCounter = 0;
	internode->isEnqueued = false;
	for (uint32 i = 0; i < 2; ++i)
	{
		if (internode->nextInternodes[i])
		{
			++internode->accessCounter;
		}
	}
}

void ForEachInternodeInTree(Tree *model, void(*function)(Internode *internode))
{
	std::stack<Internode*> stack;
	stack.push(model->root);

	while (!stack.empty())
	{
		Internode *tmp = stack.top();
		stack.pop();

		for (uint32 i = 0; i < model->maxInternodesFromBud; i++)
		{
			if (tmp->nextInternodes[i])
			{
				stack.push(tmp->nextInternodes[i]);
			}
		}

		function(tmp);
	}
}

void ShedBranches(Tree * model)
{
	ForEachInternodeInTree(model, CountDirectDescendants);

	std::queue<Internode*> queue;
	std::vector<Internode *> v2;
	for (uint32 i = 0; i < model->budsBuffer.count; i++)
	{
		Bud *bud = model->budsBuffer[i];
		// SINGLE_LATERAL_BUD... can happen because a lateral bud is not developed due to growing out of bounds
		if (IS_TERMINAL_BUD(bud) || IS_INACTIVE_BUD(bud) || SINGLE_LATERAL_BUD_ON_TERMINAL_INTERNODE(bud))	// add only terminal internodes
		{
			queue.push(bud->internode);

			v2.push_back(bud->internode);
			assert(bud->internode->accessCounter == 0);
		}
	}

	while (!queue.empty())
	{
		Internode* current = queue.front();
		queue.pop();
		if (current->accessCounter > 0)
		{
			queue.push(current);
			continue;
		}
		if (current->previous == NULL)
		{
			continue;
		}

		Internode* previous = current->previous;
		float branchLightToSizeRatio = current->light / (current->descendantInternodesCount + 1);
		if (branchLightToSizeRatio < model->parameters.shedThreshold)
		{
			ShedBranch(model, current);

			// Adding inactive bud if necessary
			if (!previous->nextInternodes[MAIN] && !previous->nextLateralInternodesCount &&
				(!previous->buds[TERMINAL_BUD] && !previous->buds[LATERAL_BUD]))
			{
				Buffer::Add(&model->budsBuffer, CreateBud(model, previous->endingPosition, Vector3(0, 0, 0), Vector3(0, 0, 1), previous, INACTIVE_BUD_AGE));
			}
		}

		if (previous)
		{
			--previous->accessCounter;
			if (!previous->isEnqueued)
			{
				previous->isEnqueued = true;
				queue.push(previous);
			}
		}
	}
}

#include <algorithm>
void DistributeResourceRootToBuds(Tree * model)
{
	model->root->resource = model->root->light * model->parameters.resourceToLightRatio;

	std::stack<Internode*> stack;
	stack.push(model->root);

	while (!stack.empty())
	{
		Internode *current = stack.top();
		stack.pop();

		float lambda = model->parameters.lambda;
		float resource = current->resource;
		Internode **next = current->nextInternodes;
		// TODO: For multiple lateral buds, you have to think this through a bit more
		if (current->order > model->parameters.apicalControlMaxOrder)
		{
			lambda = 0.51f;
		}

		if (current->order == 0 && current->endingPosition.y < model->constants.canopyHeight)
		{
			lambda = GetCanopyBoost(current->endingPosition, model->constants.canopyHeight, lambda);
		}

		// Calculate denominator
		float totalLight = 0.0f;
		if (next[MAIN])
			totalLight = lambda * next[MAIN]->light;
		for (uint32 i = 0; i < model->maxInternodesFromBud; i++)
		{
			if (next[i + 1])
			{
				totalLight += next[1 + i]->light * (1 - lambda);
			}
		}

		// Distribute resource
		if (next[MAIN])
		{
			next[MAIN]->resource = resource * (lambda * next[MAIN]->light) / totalLight;
			stack.push(next[MAIN]);
		}
		for (uint32 i = 0; i < model->maxInternodesFromBud; i++)
		{
			if (next[i + 1])
			{
				next[i + 1]->resource = resource * ((1.0f - lambda) * next[i + 1]->light) / totalLight;
				stack.push(next[i + 1]);
			}
		}
	}
}

//working only with 2 internodes for now
void CalculateWidth(Tree * model)
{
	for (uint32 i = 0; i < model->budsBuffer.count; i++)
	{
		Internode* current = model->budsBuffer[i]->internode;
		current->width = MAX(current->width, model->constants.basicWidth); // cause of shedding
		float currentWidth = current->width;

		Internode* previous = current->previous;

		while (previous)
		{
			if (previous->width == 0)
			{
				previous->width = currentWidth;
			}
			else
			{
				Internode **nextInternodes = previous->nextInternodes;

				if (nextInternodes[0] && previous->nextLateralInternodesCount)
				{
					float widthPower = model->parameters.widthPower;
					float calculatedWidth = Math::Pow(Math::Pow(nextInternodes[0]->width, widthPower) +
													  Math::Pow(nextInternodes[1]->width, widthPower), 1 / widthPower);
					previous->width = MAX(previous->width, calculatedWidth);
				}
				else if (nextInternodes[MAIN])
				{
					previous->width = MAX(nextInternodes[0]->width, previous->width);
				}
				else if (nextInternodes[1])
				{
					previous->width = MAX(nextInternodes[1]->width, previous->width);
				}
			}

			currentWidth = previous->width;
			previous = previous->previous;
		}
	}
}

void ClearLightAndResource(Internode *internode)
{
	internode->light = 0.0f;
	internode->resource = 0.0f;
}

void Free(Internode *internode)
{
	free(internode);
}

void ClearInternodeLightAndResource(Tree *model)
{
	ForEachInternodeInTree(model, ClearLightAndResource);
}

Vector3 CalculateLateralBudDirection(Tree *model, Vector3 mainInternodeDirection, Vector3 previousInternodeDirection, float phyllotacticAngle)
{
	assert(Math::Abs(Math::Dot(mainInternodeDirection, previousInternodeDirection)) < 1.0f);
	//{
	//	axis = Vector3(0, 0, 1);
	//}

	Vector3 right = Math::CrossProduct(mainInternodeDirection, previousInternodeDirection);
	right = Math::Normalize(right);

	Matrix4x4 branchingRotationMatrix = Math::GetRotation(model->parameters.branchingAngle, right);
	Matrix4x4 phyllotacticRotationMatrix = Math::GetRotation(phyllotacticAngle, Math::Normalize(mainInternodeDirection));
	Vector3 lateralBudDirection = Math::Normalize(branchingRotationMatrix * mainInternodeDirection);

	Vector3 rotatedLateralBudDirection = Math::Normalize(phyllotacticRotationMatrix * lateralBudDirection);

	return rotatedLateralBudDirection;
}

void TranslateBud(Bud *bud, Internode *newInternode, Internode *previousInternode)
{
	if (bud->age > 0) // was terminal
	{
		previousInternode->buds[0] = NULL;
	}
	else // was lateral
	{
		previousInternode->buds[1] = NULL;
	}

	newInternode->buds[0] = bud;

	bud->internode = newInternode;
	bud->position = newInternode->endingPosition;
	++bud->age;
}

Internode* CreateInternode(Tree *model, Vector3 startingPosition, Vector3 endingPosition, Internode *previous, uint32 order)
{
	Internode *newInternode = (Internode*)malloc(sizeof(Internode));
	(*newInternode) = {};
	newInternode->startingPosition = startingPosition;
	newInternode->endingPosition = endingPosition;
	newInternode->previous = previous;
	newInternode->width = model->constants.basicWidth;
	newInternode->resource = 0;
	newInternode->light = 0;
	newInternode->nextLateralInternodesCount = 0;
	newInternode->descendantInternodesCount = 0;
	newInternode->order = order;

	newInternode->nextInternodes[MAIN] = NULL;
	newInternode->nextInternodes[1] = NULL;

	for (uint32 i = 0; i < ARRAYSIZE(newInternode->buds); ++i)
	{
		newInternode->buds[i] = NULL;
	}

	++model->internodeCount;

	return newInternode;
}

void DeleteShedBuds(Tree *model)
{
	Bud **tmp = (Bud**)malloc(sizeof(Bud*) * model->budsBuffer.size);
	uint32 tmpCount = 0;

	for (uint32 i = 0; i < model->budsBuffer.count; i++)
	{
		if (model->budsBuffer[i]->internode == NULL)
		{
			free(model->budsBuffer[i]);
		}
		else
		{
			tmp[tmpCount++] = model->budsBuffer[i];
		}
	}

	free(model->budsBuffer.items);

	model->budsBuffer.items = tmp;
	model->budsBuffer.count = tmpCount;
}

void Grow(Tree *model)
{
	const uint32 budCount = model->budsBuffer.count;	// Buds are added during the algorithm!

	for (uint32 i = 0; i < budCount; i++)
	{
		Bud *bud = model->budsBuffer[i];
		if (!IS_INACTIVE_BUD(bud) && bud->internode->resource >= model->parameters.resourceThreshold)
		{
			// TODO: Remove clamp and tweak shadow parameters so lambda > 0.5f doesn't cause growth explosion 
			//		 Need to visualize voxelSpace first
			float totalLength = Math::Clamp(bud->internode->resource, 0.0f, 2.5f); 
			uint32_t internodeCount = uint32_t(Math::Ceil(totalLength));
			float length = totalLength / internodeCount;
			for (uint32_t ni = 0; ni < internodeCount; ++ni)
			{
				Vector3 lateralBudDirection;
				Vector3 newDirection = bud->direction;
				Internode* oldInternode = bud->internode;
				Vector3 oldDirection = bud->direction;

				if (IS_TERMINAL_BUD(bud))
				{
					lateralBudDirection = CalculateLateralBudDirection(model, GET_DIRECTION(oldInternode), bud->lastLateralDirection, model->parameters.phyllotacticAngle);
					newDirection = CalculateDirectionWithEnvironmentAndTropismVectorAndDeflection(
						model, bud->position, bud->direction, oldInternode->order, lateralBudDirection);
				}

				Vector3 newInternodeEndingPosition = bud->position + newDirection * length * model->constants.basicLength;
				if (Voxels::IsPositionInVoxelSpace(&model->voxelSpace, newInternodeEndingPosition))
				{
					bud->direction = newDirection;
					uint32 order = IS_TERMINAL_BUD(bud) ? oldInternode->order : oldInternode->order + 1;
					Internode* newInternode = CreateInternode(model,
															  oldInternode->endingPosition,
															  newInternodeEndingPosition,
															  oldInternode,
															  order);
					if (IS_TERMINAL_BUD(bud))
					{
						Vector3 newLateralBudDirection = CalculateLateralBudDirection(model, lateralBudDirection, GET_DIRECTION(oldInternode), (rand() % 1000) / 1000.f * PI2);
						SpawnLateralBud(model, lateralBudDirection, newLateralBudDirection, oldInternode);
						bud->lastLateralDirection = lateralBudDirection;
						oldInternode->nextInternodes[MAIN] = newInternode;
					}
					else
					{
						oldInternode->nextInternodes[++(oldInternode->nextLateralInternodesCount)] = newInternode;
					}
					TranslateBud(bud, newInternode, oldInternode);
					IncrementDescendantInternodesCount(newInternode);
					newInternode->previous = oldInternode;
				}
				else
				{
					break;
				}
			}
		}
	}
}

void TreeModel::Init(Tree *model, WorldConstants constants, TreeParameters parameters)
{
	model->root = (Internode*)malloc(sizeof(Internode));
	(*model->root) = {};
	model->root->startingPosition = Vector3(0, 0, 0);
	Vector3 startingDirection = Math::Normalize(Vector3(0.f, 1.f, 0.001f));
	model->root->endingPosition = model->root->startingPosition + startingDirection * constants.basicLength;
	model->root->width = 0.5f;
	model->root->order = 0;

	model->internodeCount = 1;
	model->maxInternodesFromBud = 2;

	Buffer::Init(&model->budsBuffer, 200000);
	Bud *terminalBud = CreateBud(model, model->root->endingPosition,
								 GET_DIRECTION(model->root), Vector3(0, 0, 1), model->root, 1);
	Buffer::Add(&model->budsBuffer, terminalBud);
	model->root->buds[0] = terminalBud;

	model->parameters = parameters;
	model->constants = constants;
	model->voxelSpace = Voxels::GetVoxelSpace(&constants);

	model->iteration = 0;
	model->shedTotal = 0;
}

void TreeModel::Update(Tree *model)
{
	/*

	- Light from buds
	- Resource in buds
	- Grow

	*/

	ComputeVoxelShadows(model);
	CalculateBudLightExposure(model);
	ClearInternodeLightAndResource(model);
	CollectLightBudsToRoot(model);

	ShedBranches(model);
	DeleteShedBuds(model);
	DistributeResourceRootToBuds(model);

	Grow(model);
	CalculateWidth(model);
	model->iteration++;
}

Tree TreeModel::GetLeafyBranch(WorldConstants constants, TreeParameters params)
{
	Tree result;
	constants.basicWidth *= 0.25f;
	constants.voxelsPerEdge = 0;
	constants.basicLength *= 0.5f;
	params.resourceThreshold = 0.0f;
	params.lambda = 0.5f;
	params.resourceToLightRatio *= 0.5f;
	params.tropismOrientationWeight = 0.0f;
	TreeModel::Init(&result, constants, params);
	result.root->order = 5;
	result.root->width = 0.0f;
	for (uint32_t i = 0; i < 4; ++i)
	{
		CalculateBudLightExposure(&result);
		ClearInternodeLightAndResource(&result);
		CollectLightBudsToRoot(&result);

		DistributeResourceRootToBuds(&result);

		Grow(&result);
		CalculateWidth(&result);

	}
	return result;
}

void TreeModel::Release(Tree *model)
{
	for (uint32_t b = 0; b < model->budsBuffer.count; ++b)
	{
		free(model->budsBuffer[b]);
	}
	Buffer::Release(&model->budsBuffer);
	ForEachInternodeInTree(model, Free);

	Voxels::Release(&model->voxelSpace);
}
