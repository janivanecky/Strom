// Authors: Dusan Drevicky, David Pribula, Jan Ivanecky
#pragma once
#include "Def.h"

namespace Strom
{
	Tree Plant(TreeParameters *treeParameters, WorldConstants *constants);
	void Grow(Tree *tree, uint32 age);
	TreeShell GetShell(Tree *tree, uint32_t maxOrder = 100);

	TreeMesh GetTreeMesh(TreeShell *shell, TreeMesh *leavesMesh);
	TreeMesh GetTreeMesh(Tree *tree, TreeMesh *leavesMesh);
	
	void Release(TreeMesh *mesh);
	void Release(Tree *tree);
	void Release(TreeShell *shell);
}