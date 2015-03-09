#ifndef OCTREE_H
#define OCTREE_H

#include "common.h"
#include "OctreeNode.h"

#include <memory>

class Octree {
public:
	int maxObjects;
	int maxLevels;

	Octree(AABB rootBB);
	bool add(SceneObject &so);
	AABB getRootBB() const;
	std::vector<SceneObject*> findObjects(AABB bb);

private:
	std::unique_ptr<OctreeNode> root;
	std::vector<SceneObject*> objectsOutsideRoot;
};

#endif