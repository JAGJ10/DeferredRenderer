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
#include "PointLight.h"
#include "MeshHelper.h"
#include "tiny_obj_loader.h"
#include <time.h>

class Scene {
public:
	Scene(int width, int height, Camera& cam);
	~Scene();

	void loadMeshes();
	void renderScene(Camera &cam);

private:
	int width, height;
	float aspectRatio;

	glm::mat4 mView, projection;
	glm::mat3 normalMatrix;

	GLuint noiseTex;
	glm::vec2 noiseScale;

	PointLight pl;

	Shader firstPass;
	Shader ssao;
	Shader blur;
	Shader stencil;
	Shader lightPass;
	Shader finalPass;
	FullscreenQuad fsQuad;
	GBuffer gBuffer;
	//Octree octree;
	//std::vector<SceneObject*> visibleObjects;
	Mesh sphere;
	std::vector<Mesh> meshes;
	std::vector<float> kernel;

	void initKernel();
	void geometryPass();
	void ssaoPass();
	void blurPass();
	void stencilPass();
	void pointLightPass();
	void compositePass();
	std::pair<std::vector<tinyobj::shape_t>, std::vector<tinyobj::material_t>> read(std::istream& stream);
};

#endif