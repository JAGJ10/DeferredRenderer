#include "Scene.h"

using namespace std;

static const int kernelSize = 64;
static const int noiseSize = 4;
static const int blurSize = 2;

Scene::Scene(int width, int height, Camera& cam) :
width(width), height(height), 
gBuffer(GBuffer(width, height)), 
firstPass(Shader("gbuffer.vert", "gbuffer.frag")),
ssao(Shader("ssao.vert", "ssao.frag")),
blur(Shader("blur.vert", "blur.frag")),
lightPass(Shader("light.vert", "light.frag")),
finalPass(Shader("ubershader.vert", "ubershader.frag")), 
fsQuad(FullscreenQuad()),
sphere(Mesh())
{
	aspectRatio = float(width) / float(height);
	projection = glm::infinitePerspective(cam.zoom, aspectRatio, 0.1f);
	srand(int(time(NULL)));
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

	gBuffer.unbindDraw();

	//Render lights to light buffer
	pointLightPass();

	//SSAO to gBuffer's effect1 texture
	//ssaoPass();

	//Blur over SSAO to filter out noise (TODO: use better blur)

	//Composition pass (directional light + light buffer)
	/*glUseProgram(finalPass.program);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	finalPass.setUniformmat4("mView", mView);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.color);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gBuffer.effect1);

	finalPass.setUniformi("positionMap", 0);
	finalPass.setUniformi("normalMap", 1);
	finalPass.setUniformi("colorMap", 2);
	finalPass.setUniformi("ssaoMap", 3);

	fsQuad.renderFromBuffers();*/

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
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	for (auto &i : meshes) {
		firstPass.setUniformv3f("diffuse", i.diffuse + i.ambient);
		firstPass.setUniformf("specular", i.specular);
		i.render();
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthMask(GL_FALSE);
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

	fsQuad.renderFromBuffers();

	gBuffer.unbindDraw();
}

void Scene::blurPass() {

}

void Scene::pointLightPass() {
	PointLight pl;
	pl.color = glm::vec3(1);
	pl.position = glm::vec3(0, 25, 0);
	pl.radius = 100;

	glUseProgram(lightPass.program);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	lightPass.setUniformmat4("mView", mView);
	lightPass.setUniformmat4("projection", projection);
	lightPass.setUniformv3f("worldPos", pl.position);
	lightPass.setUniformf("radius", pl.radius);
	lightPass.setUniformv3f("lPos", glm::vec3(mView * glm::vec4(pl.position, 1.0)));
	lightPass.setUniformv3f("lightColor", pl.color);
	lightPass.setUniformv2f("screenSize", glm::vec2(width, height));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.color);

	lightPass.setUniformi("positionMap", 0);
	lightPass.setUniformi("normalMap", 1);
	lightPass.setUniformi("colorMap", 2);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	glClear(GL_COLOR_BUFFER_BIT);

	sphere.render();
}

pair<vector<tinyobj::shape_t>, vector<tinyobj::material_t>> Scene::read(std::istream& stream) {
	assert(sizeof(float) == sizeof(int));
	const auto sz = sizeof(int);

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	int nMeshes = 0;
	int nMatProperties = 0;
	stream.read((char*)&nMeshes, sz);
	stream.read((char*)&nMatProperties, sz);
	shapes.resize(nMeshes);
	materials.resize(nMeshes);

	for (size_t i = 0; i < nMeshes; ++i) {
		int nVertices = 0, nNormals = 0, nTexcoords = 0, nIndices = 0;
		stream.read((char*)&nVertices, sz);
		stream.read((char*)&nNormals, sz);
		stream.read((char*)&nTexcoords, sz);
		stream.read((char*)&nIndices, sz);

		shapes[i].mesh.positions.resize(nVertices);
		shapes[i].mesh.normals.resize(nNormals);
		shapes[i].mesh.texcoords.resize(nTexcoords);
		shapes[i].mesh.indices.resize(nIndices);
		
		stream.read((char*)&shapes[i].mesh.positions[0], nVertices * sz);
		stream.read((char*)&shapes[i].mesh.normals[0], nNormals * sz);
		stream.read((char*)&shapes[i].mesh.texcoords[0], nTexcoords * sz);
		stream.read((char*)&shapes[i].mesh.indices[0], nIndices * sz);
		stream.read((char*)&materials[i].ambient[0], 3 * sz);
		stream.read((char*)&materials[i].diffuse[0], 3 * sz);
		stream.read((char*)&materials[i].specular[0], 3 * sz);
	}

	pair<vector<tinyobj::shape_t>, vector<tinyobj::material_t>> ret(shapes, materials);

	return ret;
}