// Authors: Dusan Drevicky, David Pribula, Jan Ivanecky
#include "Strom.h"
#include "Model.h"
#include "TreeModel.h"

Tree Strom::Plant(TreeParameters *parameters, WorldConstants *constants)
{
	Tree tree;
	TreeModel::Init(&tree, *constants, *parameters);
	return tree;
}

void Strom::Grow(Tree *tree, uint32 age)
{
	uint32 startIteration = tree->iteration;
	uint32 endIteration = age;

	// If the desired iteration is lower than existing iteration, just reset the model and 
	// regrow it
	if (startIteration > endIteration)
	{
		startIteration = 0;
		TreeModel::Release(tree);
		TreeModel::Init(tree, tree->constants,
						  tree->parameters);
	}

	for (uint32 i = startIteration; i < endIteration; ++i)
	{
		TreeModel::Update(tree);
	}
}

TreeShell Strom::GetShell(Tree *tree, uint32_t maxOrder)
{
	uint32_t maxIntegrationCount = 1000;
	float internodeMergeWidthRatio = 10000.0f;//0.25f
	float mergeDirectionThreshold = 100000.0f;// 1.0f;

	TreeShell result;
	result.constants = tree->constants;
	result.parameters = tree->parameters;

	result.internodeCount = 0;
	result.connectionCount = 0;
	result.internodes = (Internode *)malloc(sizeof(Internode) * tree->internodeCount);
	result.parentIndices = (uint32_t *)malloc(sizeof(uint32_t) * tree->internodeCount);
	result.budIndices = (uint32_t *)malloc(sizeof(uint32_t) * tree->internodeCount);
	result.parentBranchPart = (float *)malloc(sizeof(float) * tree->internodeCount);
	result.budsBuffer.count = tree->budsBuffer.count;
	result.budsBuffer.items = (Bud **)malloc(sizeof(Bud *) * result.budsBuffer.count);
	result.endWidths = (float *)malloc(sizeof(float) * tree->internodeCount);
	result.parentIndices[0] = 0;

	// Get all the buds, but clamp them to maxOrder
	// TODO: This is a shitty way to do it, much better way is to not generate internodes
	// over maxOrder. Not generate them and then remove them...
	for (uint32_t i = 0; i < tree->budsBuffer.count; ++i)
	{
		result.budsBuffer[i] = (Bud *)malloc(sizeof(Bud));
		Bud currentBud = *tree->budsBuffer[i];
		while (currentBud.internode->order > maxOrder)
		{
			currentBud.internode = currentBud.internode->previous;
			currentBud.position = currentBud.internode->endingPosition;
		}
		*result.budsBuffer[i] = currentBud;
	}

	// Pass through the tree from root to buds, to obtain required stats
	std::queue<Internode*> queue;
	queue.push(tree->root);
	std::queue<Internode> copies;
	copies.push(*tree->root);

	float minWidth = tree->constants.basicWidth;
	while (!queue.empty())
	{
		Internode *currentInternode = queue.front();
		queue.pop();
		Internode internode = copies.front();
		copies.pop();

		for (uint32_t b = 0; b < result.budsBuffer.count; ++b)
		{
			if (result.budsBuffer[b]->internode == currentInternode)
			{
				result.budIndices[b] = result.internodeCount;
			}
		}

		result.parentBranchPart[result.internodeCount] = 1.0f;
		result.internodes[result.internodeCount] = internode;
		currentInternode = &result.internodes[result.internodeCount];
#define UNIT_DIRECTION(v1) (Math::Normalize(v1->endingPosition - v1->startingPosition))
#define DIRECTION(v1) (v1->endingPosition - v1->startingPosition)
		if (currentInternode->nextInternodes[0] && Math::Dot(UNIT_DIRECTION(currentInternode), UNIT_DIRECTION(currentInternode->nextInternodes[0])) > mergeDirectionThreshold && currentInternode->nextInternodes[0]->width / currentInternode->width >= internodeMergeWidthRatio)
		{
			std::vector<Internode> children;
			std::vector<uint32_t> childrenIndex;

			uint32_t integrationCount = 0;
			Internode *mainInternode = currentInternode;
			do
			{
				Internode *child = mainInternode->nextInternodes[0];
				currentInternode->endingPosition = child->endingPosition;
				result.endWidths[result.internodeCount] = child->width;

				for (uint32_t b = 0; b < result.budsBuffer.count; ++b)
				{
					if (result.budsBuffer[b]->internode == child)
					{
						result.budIndices[b] = result.internodeCount;
					}
				}


				for (uint32_t i = 1; i < tree->maxInternodesFromBud; i++)
				{
					if (child->nextInternodes[i] && child->nextInternodes[i]->order <= maxOrder && child->nextInternodes[i]->width >= minWidth)
					{
						Internode copy = *child->nextInternodes[i];
						result.parentIndices[result.internodeCount + queue.size() + 1] = result.internodeCount;
						queue.push(child->nextInternodes[i]);
						children.push_back(copy);
						childrenIndex.push_back(result.internodeCount + queue.size() + 1);
						result.connectionCount++;
					}
				}
				integrationCount++;
				mainInternode = child;
			} while (mainInternode->nextInternodes[0] && Math::Dot(UNIT_DIRECTION(currentInternode), UNIT_DIRECTION(mainInternode->nextInternodes[0])) > mergeDirectionThreshold && integrationCount < maxIntegrationCount && mainInternode->nextInternodes[0]->width / currentInternode->width >= internodeMergeWidthRatio);

			if (mainInternode->nextInternodes[0])
			{
				Internode copy = *mainInternode->nextInternodes[0];
				result.parentIndices[result.internodeCount + queue.size() + 1] = result.internodeCount;
				queue.push(mainInternode->nextInternodes[0]);
				children.push_back(copy);
				childrenIndex.push_back(result.internodeCount + queue.size() + 1);
				result.connectionCount++;
			}

			for (uint32_t i = 1; i < tree->maxInternodesFromBud; i++)
			{
				if (currentInternode->nextInternodes[i] && currentInternode->nextInternodes[i]->order <= maxOrder && currentInternode->nextInternodes[i]->width >= minWidth)
				{
					Internode copy = *currentInternode->nextInternodes[i];
					result.parentIndices[result.internodeCount + queue.size() + 1] = result.internodeCount;
					queue.push(currentInternode->nextInternodes[i]);
					children.push_back(copy);
					childrenIndex.push_back(result.internodeCount + queue.size() + 1);
					result.connectionCount++;
				}
			}

			for (uint32_t i = 0; i < children.size(); ++i)
			{
				Internode child = children[i];
				uint32_t index = childrenIndex[i];
				Vector3 newPosition = Math::Dot(child.startingPosition - currentInternode->startingPosition, UNIT_DIRECTION(currentInternode)) * UNIT_DIRECTION(currentInternode) + currentInternode->startingPosition;
				child.startingPosition = newPosition;

				result.parentBranchPart[index] = Math::Length(child.startingPosition - currentInternode->startingPosition) / Math::Length(DIRECTION(currentInternode));
				copies.push(child);
			}
		}
		else
		{
			result.endWidths[result.internodeCount] = currentInternode->width;
			for (uint32 i = 0; i < tree->maxInternodesFromBud; i++)
			{
				if (currentInternode->nextInternodes[i] && currentInternode->nextInternodes[i]->order <= maxOrder)
				{
					Internode copy = *currentInternode->nextInternodes[i];
					result.parentIndices[result.internodeCount + queue.size() + 1] = result.internodeCount;
					queue.push(currentInternode->nextInternodes[i]);
					copies.push(copy);
					result.connectionCount++;
				}
			}
		}
		
		result.internodeCount++;
	}
	return result;
}

TreeMesh Strom::GetTreeMesh(TreeShell *shell, TreeMesh *leavesMesh)
{
	TreeMesh treeMesh;
	Model::TreeToMesh(shell, &treeMesh, leavesMesh);
	return treeMesh;
}

TreeMesh Strom::GetTreeMesh(Tree *tree, TreeMesh *leavesMesh)
{
	TreeMesh treeMesh;
	Model::TreeToMesh(tree, &treeMesh, leavesMesh);
	return treeMesh;
}

void Strom::Release(Tree *tree)
{
	TreeModel::Release(tree);
}

void Strom::Release(TreeShell *shell)
{
	for (uint32_t b = 0; b < shell->budsBuffer.count; ++b)
	{
		free(shell->budsBuffer[b]);
	}
	Buffer::Release(&shell->budsBuffer);
	free(shell->budIndices);
	free(shell->internodes);
	free(shell->parentIndices);
}

void Strom::Release(TreeMesh *mesh)
{
	free(mesh->vertices);
	free(mesh->indices);
}
