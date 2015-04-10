#ifndef SCENE_H
#define SCENE_H

#include "common.h"
#include "SceneObject.h"
#include "Octree.h"
#include "GBuffer.h"
#include "AABB.h"
#include "Mesh.h"
#include "FullscreenQuad.h"
#include "Camera.hpp"
#include "Shader.h"
#include "tiny_obj_loader.h"
#include <time.h>

class Scene {
public:
	Scene(int width, int height);
	~Scene();

	void loadMeshes();
	void renderScene(Camera &cam);
	std::pair<std::vector<tinyobj::shape_t>, std::vector<tinyobj::material_t>> read(std::istream& stream);

private:
	int width, height;

	GLuint noiseTex;
	glm::vec2 noiseScale;

	Shader firstPass;
	Shader ssao;
	Shader blur;
	Shader lightPass;
	FullscreenQuad fsQuad;
	GBuffer gBuffer;
	//Octree octree;
	//std::vector<SceneObject*> visibleObjects;
	std::vector<Mesh> meshes;
	std::vector<float> kernel;

	void initKernel();
};

#endif