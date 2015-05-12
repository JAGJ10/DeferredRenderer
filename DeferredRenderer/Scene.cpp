#include "Scene.h"

using namespace std;

static const int kernelSize = 64;
static const int noiseSize = 4;
static const int blurSize = 2;
static const glm::vec4 lightDir = glm::vec4(1, 1, 1, 0);

Scene::Scene(int width, int height, Camera& cam) :
width(width), height(height), 
gBuffer(GBuffer(width, height)),
dLightShadow(ShadowMap(2048, 2048)),
pLightShadow(PointLightShadowMap(1024, 1024)),
geometry(Shader("gbuffer.vert", "gbuffer.frag")),
shadow(Shader("shadow.vert", "empty.frag")),
plShadow(Shader("plShadow.vert", "plShadow.frag")),
stencil(Shader("light.vert", "empty.frag")),
ssao(Shader("ssao.vert", "ssao.frag")),
blur(Shader("blur.vert", "blur.frag")),
lightPass(Shader("light.vert", "light.frag")),
finalPass(Shader("ubershader.vert", "ubershader.frag")),
skybox(Shader("skybox.vert", "skybox.frag")),
fsQuad(FullscreenQuad()),
sphere(Mesh())
{
	type = 0;
	aspectRatio = float(width) / float(height);
	projection = glm::infinitePerspective(cam.zoom, aspectRatio, 0.1f);
	dLightMView = glm::lookAt(glm::vec3(500.0f, 500.0f, 500.0f), glm::vec3(0), glm::vec3(0, 1, 0));
	dLightProjection = glm::ortho(-450.0f, 450.0f, -450.0f, 450.0f, 0.1f, 2000.0f);

	directions[0] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) };
	directions[1] = { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) };
	directions[2] = { GL_TEXTURE_CUBE_MAP_POSITIVE_Y, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f) };
	directions[3] = { GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) };
	directions[4] = { GL_TEXTURE_CUBE_MAP_POSITIVE_Z, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f) };
	directions[5] = { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f) };
	
	srand(int(time(NULL)));

	for (int i = 0; i < 1; i++) {
		float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float r3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		PointLight pl;
		//pl.color = glm::vec3(r1, r2, r3);
		pl.color = glm::vec3(1);
		//pl.position = glm::vec3(500 * (r1 * 2 - 1), 50 * r2, 500 * (r3 * 2 - 1));
		pl.position = glm::vec3(100, 20, 280);
		pl.attenuation = glm::vec3(1, 0.01f, 0.001f); //radius, 0.001 = 500, 0.01 = 159
		pl.radius = (-pl.attenuation.y + sqrtf(pow(pl.attenuation.y, 2) - (4 * pl.attenuation.z*(pl.attenuation.x - 256)))) / (2 * pl.attenuation.z);
		lights.push_back(pl);
	}

	loadCubemap();
}

Scene::~Scene() {
	if (noiseTex != 0) glDeleteTextures(1, &noiseTex);
	if (skyboxTexture != 0) glDeleteTextures(1, &skyboxTexture);
}

void Scene::setType(int type) {
	this->type = type;
}

void Scene::loadCubemap() {
	vector<const GLchar*> faces{ "skybox/v1/right.jpg", "skybox/v1/left.jpg", "skybox/v1/top.jpg", "skybox/v1/bottom.jpg", "skybox/v1/back.jpg", "skybox/v1/front.jpg" };
	//vector<const GLchar*> faces{ "skybox/right.jpg", "skybox/left.jpg", "skybox/top.jpg", "skybox/bottom.jpg", "skybox/back.jpg", "skybox/front.jpg" };
	int width, height;
	unsigned char* image;

	glGenTextures(1, &skyboxTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	for (GLuint i = 0; i < faces.size(); i++) {
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,	GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	free(image);
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
	shadowPass();
	
	//Render point light shadow maps

	//Need to clear light buffer
	gBuffer.bindDraw();
	gBuffer.setDrawLight();
	glClear(GL_COLOR_BUFFER_BIT);
	gBuffer.unbindDraw();
	
	//Compute stencil and then render lights
	for (auto &pl : lights) {
		glViewport(0, 0, 1024, 1024);
		plShadowPass(pl);
		glViewport(0, 0, width, height);
		glEnable(GL_STENCIL_TEST);
		stencilPass(pl);
		pointLightPass(pl);
		glDisable(GL_STENCIL_TEST);
	}
	
	//SSAO to gBuffer's effect1 texture
	ssaoPass();

	//Blur over SSAO to filter out noise (TODO: use better blur)

	//Render directional light and compute final color with light buffer
	compositePass();

	//Render skybox last
	skyboxPass();

	GLenum err = glGetError();
	if (err != 0) cout << err << endl;
}

void Scene::geometryPass() {
	glUseProgram(geometry.program);
	gBuffer.bindDraw();
	gBuffer.setDrawBuffers();

	geometry.setUniformmat4("mView", mView);
	geometry.setUniformmat4("projection", projection);
	geometry.setUniformmat3("mNormal", normalMatrix);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto &i : meshes) {
		geometry.setUniformv3f("diffuse", i.diffuse);
		geometry.setUniformf("specular", i.specular);
		i.render();
	}

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
}

void Scene::shadowPass() {
	glViewport(0, 0, 2048, 2048);
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
	glViewport(0, 0, width, height);
}

void Scene::plShadowPass(PointLight pl) {
	//Render out depth for depth testing
	glUseProgram(stencil.program);
	pLightShadow.bindDraw();
	glDrawBuffer(GL_NONE);

	stencil.setUniformmat4("mView", mView);
	stencil.setUniformmat4("projection", projection);
	stencil.setUniformv3f("worldPos", pl.position);
	stencil.setUniformf("radius", pl.radius);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	glClear(GL_DEPTH_BUFFER_BIT);
	sphere.render();

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	pLightShadow.unbindDraw();

	//Render shadow map to cube
	glUseProgram(plShadow.program);
	pLightShadow.bindDraw();
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	plShadow.setUniformv3f("worldPos", pl.position);

	//glEnable(GL_DEPTH_TEST);
	
	for (int i = 0; i < 6; i++) {
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, directions[i].face, pLightShadow.cubeMap, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		plShadow.setUniformmat4("mvp", projection * glm::lookAt(pl.position, pl.position + directions[i].target, directions[i].up));
		for (auto &i : meshes) {
			i.render();
		}
	}

	//glDisable(GL_DEPTH_TEST);

	pLightShadow.unbindDraw();
}

void Scene::stencilPass(PointLight pl) {
	glUseProgram(stencil.program);
	gBuffer.bindDraw();
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

	gBuffer.unbindDraw();
}

void Scene::pointLightPass(PointLight pl) {
	glUseProgram(lightPass.program);
	gBuffer.bindDraw();
	gBuffer.setDrawLight();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.color);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, pLightShadow.cubeMap);

	lightPass.setUniformi("positionMap", 0);
	lightPass.setUniformi("normalMap", 1);
	lightPass.setUniformi("colorMap", 2);
	lightPass.setUniformi("shadowMap", 3);

	lightPass.setUniformmat4("inverseMView", glm::inverse(mView));
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

	gBuffer.unbindDraw();
}

void Scene::ssaoPass() {
	glUseProgram(ssao.program);
	gBuffer.bindDraw();
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

	finalPass.setUniformi("type", type);

	//glEnable(GL_FRAMEBUFFER_SRGB);

	glClear(GL_COLOR_BUFFER_BIT);

	fsQuad.render();

	//glDisable(GL_FRAMEBUFFER_SRGB);
}

void Scene::skyboxPass() {
	//Blit gbuffer's depth to main FBO for testing
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.getFBO());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glUseProgram(skybox.program);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

	skybox.setUniformi("skybox", 0);

	skybox.setUniformmat4("inverseVP", glm::inverse(projection * glm::mat4(glm::mat3(mView))));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	fsQuad.render();

	glDisable(GL_DEPTH_TEST);
}