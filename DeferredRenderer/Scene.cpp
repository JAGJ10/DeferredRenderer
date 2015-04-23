#include "Scene.h"

using namespace std;

static const int kernelSize = 256;
static const int noiseSize = 4;
static const int blurSize = 2;
static const glm::vec4 lightDir = glm::vec4(1, 1, 1, 0);

Scene::Scene(int width, int height, Camera& cam) :
width(width), height(height), 
gBuffer(GBuffer(width, height)),
dLightShadow(ShadowMap(2048, 2048)),
firstPass(Shader("gbuffer.vert", "gbuffer.frag")),
shadow(Shader("shadow.vert", "empty.frag")),
stencil(Shader("light.vert", "empty.frag")),
ssao(Shader("ssao.vert", "ssao.frag")),
blur(Shader("blur.vert", "blur.frag")),
lightPass(Shader("light.vert", "light.frag")),
finalPass(Shader("ubershader.vert", "ubershader.frag")), 
fsQuad(FullscreenQuad()),
sphere(Mesh())
{
	aspectRatio = float(width) / float(height);
	projection = glm::infinitePerspective(cam.zoom, aspectRatio, 0.1f);
	dLightMView = glm::lookAt(glm::vec3(300.0f, 300.0f, 300.0f), glm::vec3(0), glm::vec3(0, 1, 0));
	dLightProjection = glm::ortho(-450.0f, 450.0f, -450.0f, 450.0f, 0.1f, 2000.0f);
	srand(int(time(NULL)));

	for (int i = 0; i < 50; i++) {
		float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float r3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		PointLight pl;
		pl.color = glm::vec3(r1, r2, r3);
		pl.position = glm::vec3(500 * (r1 * 2 - 1), 50 * r2, 500 * (r3 * 2 - 1));
		pl.attenuation = glm::vec3(1, 0.01f, 0.0001f);
		pl.radius = (-pl.attenuation.y + sqrtf(pow(pl.attenuation.y, 2) - (4 * pl.attenuation.z*(pl.attenuation.x - 256)))) / (2 * pl.attenuation.z);
		lights.push_back(pl);
	}
}

Scene::~Scene() {
	if (noiseTex != 0) {
		glDeleteBuffers(1, &noiseTex);
	}
}

void Scene::loadMeshes() {
	ifstream infile("scenes/rungholt.cobj", std::ifstream::binary);
	pair<vector<tinyobj::shape_t>, vector<tinyobj::material_t>> sm = read(infile);

	meshes.clear();
	meshes.resize(sm.first.size());

	for (int i = 0; i < sm.first.size(); i++) {
		meshes[i].create();
		meshes[i].updateBuffers(sm.first[i].mesh.positions, sm.first[i].mesh.indices, sm.first[i].mesh.normals);
		meshes[i].ambient = glm::vec3(sm.second[i].ambient[0], sm.second[i].ambient[1], sm.second[i].ambient[2]);
		meshes[i].diffuse = glm::vec3(sm.second[i].diffuse[0], sm.second[i].diffuse[1], sm.second[i].diffuse[2]);
		meshes[i].specular = sm.second[i].specular[0];
	}

	vector<glm::vec3> positions;
	vector<GLuint> indices;
	generatePatchedSphere(positions, indices);
	sphere.create();
	sphere.updateBuffers(positions, indices);

	initKernel();
}

void Scene::initKernel() {
	for (int i = 0; i < kernelSize; i++) {
		float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float r3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		glm::vec3 k(r1 * 2.0f - 1.0f, r2 * 2.0f - 1.0f, r3);
		k = glm::normalize(k);
		float scale = float(i) / float(kernelSize);
		scale = glm::lerp(0.1f, 1.0f, scale * scale);
		k *= scale;

		kernel.push_back(k.x);
		kernel.push_back(k.y);
		kernel.push_back(k.z);
	}

	vector<glm::vec3> noise;
	for (int i = 0; i < kernelSize; i++) {
		float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float r3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		glm::vec3 n(r1 * 2.0f - 1.0f, r2 * 2.0f - 1.0f, 0);
		n = glm::normalize(n);
		noise.push_back(n);
	}

	glGenTextures(1, &noiseTex);
	glBindTexture(GL_TEXTURE_2D, noiseTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, noiseSize, noiseSize, 0, GL_RGBA, GL_FLOAT, &noise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	noiseScale = glm::vec2(width / noiseSize, height / noiseSize);
}

void Scene::renderScene(Camera &cam) {
	//Set camera
	mView = cam.getMView();
	normalMatrix = glm::mat3(glm::inverseTranspose(mView));
	
	//Clear buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//Render geometry to gBuffer
	geometryPass();

	//Render shadowmap
	glViewport(0, 0, 2048, 2048);
	shadowPass();
	glViewport(0, 0, width, height);

	//Need to clear light buffer
	gBuffer.bindDraw();
	gBuffer.setDrawLight();
	glClear(GL_COLOR_BUFFER_BIT);
	
	//Compute stencil and then render lights
	glEnable(GL_STENCIL_TEST);
	for (auto &pl : lights) {
		stencilPass(pl);
		pointLightPass(pl);
	}
	glDisable(GL_STENCIL_TEST);
	
	//SSAO to gBuffer's effect1 texture
	ssaoPass();

	//Blur over SSAO to filter out noise (TODO: use better blur)

	//Render directional light and compute final color with light buffer
	compositePass();

	cout << glGetError() << endl;
}

void Scene::geometryPass() {
	glUseProgram(firstPass.program);
	gBuffer.bindDraw();
	gBuffer.setDrawBuffers();

	firstPass.setUniformmat4("mView", mView);
	firstPass.setUniformmat4("projection", projection);
	firstPass.setUniformmat3("mNormal", normalMatrix);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto &i : meshes) {
		firstPass.setUniformv3f("diffuse", i.diffuse);
		firstPass.setUniformf("specular", i.specular);
		i.render();
	}

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
}

void Scene::shadowPass() {
	glUseProgram(shadow.program);
	dLightShadow.bindDraw();
	
	shadow.setUniformmat4("mvp", dLightProjection * dLightMView);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	
	glClear(GL_DEPTH_BUFFER_BIT);

	for (auto &i : meshes) {
		i.render();
	}
	
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	dLightShadow.unbindDraw();
}

void Scene::stencilPass(PointLight pl) {
	glUseProgram(stencil.program);
	gBuffer.setDrawNone();

	stencil.setUniformmat4("mView", mView);
	stencil.setUniformmat4("projection", projection);
	stencil.setUniformv3f("worldPos", pl.position);
	stencil.setUniformf("radius", pl.radius);

	glEnable(GL_DEPTH_TEST);
	glStencilFunc(GL_ALWAYS, 0, 0);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

	glClear(GL_STENCIL_BUFFER_BIT);

	sphere.render();

	glDisable(GL_DEPTH_TEST);
}

void Scene::pointLightPass(PointLight pl) {
	glUseProgram(lightPass.program);
	gBuffer.setDrawLight();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.color);

	lightPass.setUniformi("positionMap", 0);
	lightPass.setUniformi("normalMap", 1);
	lightPass.setUniformi("colorMap", 2);

	lightPass.setUniformmat4("mView", mView);
	lightPass.setUniformmat4("projection", projection);
	lightPass.setUniformv3f("worldPos", pl.position);
	lightPass.setUniformf("radius", pl.radius);
	lightPass.setUniformv3f("lPos", glm::vec3(mView * glm::vec4(pl.position, 1.0)));
	lightPass.setUniformv3f("lightColor", pl.color);
	lightPass.setUniformv3f("lightAttenuation", pl.attenuation);
	lightPass.setUniformv2f("screenSize", glm::vec2(width, height));

	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_CULL_FACE);

	sphere.render();

	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void Scene::ssaoPass() {
	glUseProgram(ssao.program);
	gBuffer.setDrawEffect();

	ssao.setUniformmat4("projection", projection);
	ssao.setUniformi("kernelSize", kernelSize);
	ssao.setUniformv2f("noiseScale", noiseScale);
	ssao.setUniform3fv("kernel", kernelSize, &kernel[0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTex);

	ssao.setUniformi("positionMap", 0);
	ssao.setUniformi("normalMap", 1);
	ssao.setUniformi("noiseMap", 2);

	fsQuad.render();

	gBuffer.unbindDraw();
}

void Scene::blurPass() {

}

void Scene::compositePass() {
	//Composition pass (directional light + light buffer)
	glUseProgram(finalPass.program);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	finalPass.setUniformmat4("inverseMView", glm::inverse(mView));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.color);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gBuffer.light);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gBuffer.effect1);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, dLightShadow.depth);

	finalPass.setUniformi("positionMap", 0);
	finalPass.setUniformi("normalMap", 1);
	finalPass.setUniformi("colorMap", 2);
	finalPass.setUniformi("lightMap", 3);
	finalPass.setUniformi("ssaoMap", 4);
	finalPass.setUniformi("shadowMap", 5);

	finalPass.setUniformv3f("l", glm::vec3(mView * lightDir));
	finalPass.setUniformmat4("shadowMapMVP", dLightProjection * dLightMView);
	finalPass.setUniformi("shadowMapWidth", 2048);
	finalPass.setUniformi("shadowMapHeight", 2048);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	fsQuad.render();
}