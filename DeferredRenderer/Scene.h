#ifndef SCENE_H
#define SCENE_H

#include "common.h"
#include "SceneObject.h"
#include "Octree.h"
#include "GBuffer.h"

class Scene {
public:
	Scene();
	~Scene();

	void renderScene();

private:
	GBuffer gBuffer;
	Octree octree;
	std::vector<SceneObject*> visibleObjects;
};

#endif