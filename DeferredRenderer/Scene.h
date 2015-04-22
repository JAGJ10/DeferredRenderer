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
#include "ShadowMap.h"
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

	glm::mat4 mView, projection, dLightMView, dLightProjection;
	glm::mat3 normalMatrix;

	GLuint noiseTex;
	glm::vec2 noiseScale;

	GBuffer gBuffer;
	ShadowMap dLightShadow;
	Shader firstPass;
	Shader shadow;
	Shader stencil;
	Shader lightPass;
	Shader ssao;
	Shader blur;
	Shader finalPass;
	FullscreenQuad fsQuad;
	//Octree octree;
	//std::vector<SceneObject*> visibleObjects;
	Mesh sphere;
	std::vector<PointLight> lights;
	std::vector<Mesh> meshes;
	std::vector<float> kernel;

	void initKernel();
	void geometryPass();
	void shadowPass();
	void stencilPass(PointLight pl);
	void pointLightPass(PointLight pl);
	void ssaoPass();
	void blurPass();
	void compositePass();
};

#endif