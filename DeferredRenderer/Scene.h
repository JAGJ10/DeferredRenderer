#ifndef SCENE_H
#define SCENE_H

#include "common.h"
#include "SceneObject.h"
#include "Octree.h"
#include "GBuffer.h"
#include "AABB.h"
#include "Mesh.h"
#include "Camera.hpp"
#include "Shader.h"

class Scene {
public:
	Scene(int width, int height);
	~Scene();

	void loadMeshes();
	void renderScene(Camera &cam);

private:
	Shader firstPass;
	Shader secondPass;
	GBuffer gBuffer;
	//Octree octree;
	std::vector<SceneObject*> visibleObjects;
	std::vector<Mesh> meshes;
};

#endif