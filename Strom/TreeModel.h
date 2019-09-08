// Authors: Dusan Drevicky, David Pribula, Jan Ivanecky
#pragma once
#include "Def.h"

namespace TreeModel
{
	void Init(Tree *tree, WorldConstants constants, TreeParameters parameters);
	void Update(Tree *tree);
	void Release(Tree *tree);

	Tree GetLeafyBranch(WorldConstants constants, TreeParameters params);
}

