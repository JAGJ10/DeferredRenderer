#include "Octree.h"

using namespace std;

Octree::Octree(AABB rootBB) : maxObjects(6), maxLevels(20) {
	this->root.reset(new OctreeNode(rootBB, 0, nullptr, maxObjects));
}

/**
* Adds object to the octree
* Returns true if object is within the bounds of the tree
* False otherwise
*/
bool Octree::add(SceneObject &so) {
	if (root->getAABB().contains(so.getAABB())) {
		root->add(so);
		return true;
	} else {
		objectsOutsideRoot.push_back(&so);
		return false;
	}
}

AABB Octree::getRootBB() const {
	return root->getAABB();
}

/**
* Returns a vector of all sceneobjects that intersect the specified AABB region
*/
vector<SceneObject*> Octree::findObjects(AABB bb) {
	vector<SceneObject*> objs;
	for (auto &o : objectsOutsideRoot) {
		if (bb.intersects(o->getAABB())) {
			objs.push_back(o);
		}
	}

	vector<OctreeNode*> octree;
	octree.push_back(root.get());
	while (!octree.empty()) {
		OctreeNode *cur = octree.back();
		octree.pop_back();

		if (bb.intersects(cur->getAABB())) {
			for (auto &o : cur->getObjects()) {
				if (bb.intersects(o->getAABB())) {
					objs.push_back(o);
				}
			}
		}

		if (!cur->leafNode()) {
			for (auto &c : cur->getChildren()) {
				octree.push_back(c.get());
			}
		}
	}

	return objs;
}