#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include "AABB.h"

class SceneObject {
public:
	SceneObject();
	virtual ~SceneObject() = 0;

	virtual void update(float deltaT) = 0;
	virtual void destroy() = 0;

	AABB getAABB() const;

protected:
	AABB bb;

private:
	bool toBeDestroyed;
};

#endif