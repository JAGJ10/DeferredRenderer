#ifndef OCTREE_NODE_H
#define OCTREE_NODE_H

#include "common.h"
#include "SceneObject.h"
#include <memory>

class OctreeNode {
public:
	OctreeNode(AABB bb, int level, OctreeNode *parent, int maxObjects);

	std::vector<SceneObject*> getObjects();
	bool add(SceneObject &so);
	AABB getAABB() const;
	bool leafNode() const;
	std::vector<std::unique_ptr<OctreeNode>> &getChildren();

private:
	OctreeNode *parent;
	int level;
	int maxObjects;
	bool isLeaf;
	std::vector<std::unique_ptr<OctreeNode>> children;
	std::vector<SceneObject*> objects;
	AABB bb;

	//void mergeChildren(); //Unnecessary if rebuilding octree each frame
	//void destroyChildren(); //Unnecessary if rebuilding octree each frame
	bool partition();
};

#endif