#include "SceneObject.h"

SceneObject::SceneObject() : bb(glm::vec3(0), glm::vec3(1)), toBeDestroyed(false) {}

AABB SceneObject::getAABB() const {
	return bb;
}