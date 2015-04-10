#include "Scene.h"

using namespace std;

static const int kernelSize = 64;
static const int noiseSize = 4;
static const int blurSize = 2;

Scene::Scene(int width, int height) :
width(width), height(height), 
gBuffer(GBuffer(width, height)), 
firstPass(Shader("gbuffer.vert", "gbuffer.frag")),
ssao(Shader("ssao.vert", "ssao.frag")),
blur(Shader("blur.vert", "blur.frag")),
lightPass(Shader("ubershader.vert", "ubershader.frag")), 
fsQuad(FullscreenQuad())
{
	vector<GLfloat> positions = {
		1.0f, 1.0f,	   //Top Right
		1.0f, -1.0f,   //Bottom Right
		-1.0f, -1.0f,  //Bottom Left
		-1.0f, 1.0f	   //Top Left 
	};
	vector<GLuint> indices = {
		0, 1, 3,	//First Triangle
		1, 2, 3		//Second Triangle
	};

	fsQuad.create();
	fsQuad.updateBuffers(positions, indices);

	srand(int(time(NULL)));
}

Scene::~Scene() {}

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
	glm::mat4 mView = cam.getMView();
	glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(mView));
	glm::mat4 projection = glm::infinitePerspective(cam.zoom, 1.78f, 0.1f);
	//glm::mat4 projection = glm::perspective(cam.zoom, 1.78f, 1.0f, 1000.0f);

	//Clear buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//Render to gBuffer
	glUseProgram(firstPass.program);
	gBuffer.bindDraw();
	gBuffer.setDrawBuffers();

	firstPass.setUniformmat4("mView", mView);
	firstPass.setUniformmat4("projection", projection);
	firstPass.setUniformmat3("mNormal", normalMatrix);
	
	glDepthMask(GL_TRUE);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	for (auto &i : meshes) {
		firstPass.setUniformv3f("diffuse", i.diffuse + i.ambient);
		firstPass.setUniformf("specular", i.specular);
		i.renderFromBuffers();
	}
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthMask(GL_FALSE);

	//gBuffer.unbindDraw();

	//Screen Space Ambient Occlusion pass
	glUseProgram(ssao.program);
	gBuffer.setDrawEffect();

	ssao.setUniformmat4("mView", mView);
	ssao.setUniformmat4("projection", projection);
	ssao.setUniformmat3("mNormal", normalMatrix);

	ssao.setUniformi("kernelSize", kernelSize);
	ssao.setUniformf("fov", tanf(cam.zoom * 0.5f));
	ssao.setUniformv2f("noiseScale", noiseScale);
	ssao.setUniform3fv("kernel", kernelSize, &kernel[0]);
	
	gBuffer.setGeomTextures();
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, noiseTex);

	ssao.setUniformi("positionMap", 0);
	ssao.setUniformi("normalMap", 1);
	ssao.setUniformi("colorMap", 2);
	ssao.setUniformi("depthMap", 3);
	ssao.setUniformi("noiseMap", 4);

	fsQuad.renderFromBuffers();

	gBuffer.unbindDraw();

	//Blur over SSAO to filter out noise (TODO: use better blur)
	glUseProgram(lightPass.program);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	gBuffer.setGeomTextures();
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gBuffer.effect1);
	lightPass.setUniformi("positionMap", 0);
	lightPass.setUniformi("normalMap", 1);
	lightPass.setUniformi("colorMap", 2);
	lightPass.setUniformi("depthMap", 3);
	lightPass.setUniformi("ssaoMap", 4);

	fsQuad.renderFromBuffers();

	cout << glGetError() << endl;
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