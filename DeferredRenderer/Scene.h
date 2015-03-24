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
#include "tiny_obj_loader.h"

class Scene {
public:
	Scene(int width, int height);
	~Scene();

	void loadMeshes();
	void renderScene(Camera &cam);
	std::vector<tinyobj::shape_t> read(std::istream& stream);

private:
	Shader firstPass;
	Shader secondPass;
	Mesh fullScreenQuad;
	GBuffer gBuffer;
	//Octree octree;
	//std::vector<SceneObject*> visibleObjects;
	std::vector<Mesh> meshes;
};

#endif