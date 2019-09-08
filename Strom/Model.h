// Authors: Dusan Drevicky, David Pribula, Jan Ivanecky
#pragma once
#include "Def.h"

namespace Model
{
	void TreeToMesh(TreeShell *shell, TreeMesh *mesh, TreeMesh *leaves);
	void TreeToMesh(Tree *tree, TreeMesh *mesh, TreeMesh *leaves);
}