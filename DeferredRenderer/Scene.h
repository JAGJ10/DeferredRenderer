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
#include "PointLightShadowMap.h"
#include "tiny_obj_loader.h"
#include <time.h>
#include <SOIL.h>

class Scene {
public:
	Scene(int width, int height, Camera& cam);
	~Scene();

	void loadMeshes();
	void renderScene(Camera &cam);
	void setType(int type);

private:
	int type;
	int width, height;
	float aspectRatio;

	glm::mat4 mView, projection, dLightMView, dLightProjection;
	glm::mat3 normalMatrix;

	CameraDireciton directions[6];

	GLuint noiseTex, skyboxTexture;
	glm::vec2 noiseScale;

	GBuffer gBuffer;
	ShadowMap dLightShadow;
	PointLightShadowMap pLightShadow;
	Shader geometry;
	Shader shadow;
	Shader plShadow;
	Shader stencil;
	Shader lightPass;
	Shader ssao;
	Shader blur;
	Shader finalPass;
	Shader skybox;
	FullscreenQuad fsQuad;
	//Octree octree;
	//std::vector<SceneObject*> visibleObjects;
	Mesh sphere;
	std::vector<PointLight> lights;
	std::vector<Mesh> meshes;
	std::vector<float> kernel;

	void loadCubemap();
	void initKernel();
	void geometryPass();
	void shadowPass();
	void plShadowPass(PointLight pl);
	void stencilPass(PointLight pl);
	void pointLightPass(PointLight pl);
	void ssaoPass();
	void blurPass();
	void compositePass();
	void skyboxPass();
};

#endif