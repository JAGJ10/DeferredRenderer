#include "OctreeNode.h"

using namespace std;

OctreeNode::OctreeNode(AABB bb, int level, OctreeNode *parent, int maxObjects) : isLeaf(true), bb(bb), level(level), parent(parent), maxObjects(maxObjects) {}

vector<SceneObject*> OctreeNode::getObjects() {
	return objects;
}

bool OctreeNode::add(SceneObject &so) {
	bool inserted = false;
	if (bb.contains(so.getAABB())) {
		if (isLeaf) {
			if (objects.size() < maxObjects) objects.push_back(&so);
			else {
				partition();
				for (auto &c : children) {
					inserted = c->add(so);
				}

				if (!inserted) objects.push_back(&so); //Over maxObject limit
			}
			return true;
		} else {
			for (auto &c : children) {
				c->add(so);
			}
			return true;
		}
	} else {
		return false;
	}
}

/**
* Returns true if the Node is successfully partitioned
* Returns false if the Node is not a leaf
*/
bool OctreeNode::partition() {
	if (!isLeaf) return false;

	glm::vec3 halfDims = bb.getHalfDims();
	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			for (int z = 0; z < 2; z++) {
				glm::vec3 offset = glm::vec3(x * halfDims.x, y * halfDims.y, z * halfDims.z);
				AABB childBB(bb.getLower() + offset, bb.getCenter() + offset);

				children[x + y * 2 + z * 4].reset(new OctreeNode(childBB, level + 1, this, maxObjects));
			}
		}
	}

	isLeaf = false;
	return true;
}

AABB OctreeNode::getAABB() const {
	return bb;
}

bool OctreeNode::leafNode() const {
	return isLeaf;
}

vector<unique_ptr<OctreeNode>> &OctreeNode::getChildren() {
	return children;
}