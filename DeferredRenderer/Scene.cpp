#include "Scene.h"
#include "tiny_obj_loader.h"

using namespace std;

Scene::Scene(int width, int height) : gBuffer(GBuffer(width, height)), firstPass(Shader("gbuffer.vert", "gbuffer.frag")), secondPass(Shader("fsquad.vert", "fsquad.frag")), fullScreenQuad(Mesh()) {
	vector<GLfloat> positions = {
		1.0f, 1.0f,	   // Top Right
		1.0f, -1.0f,   // Bottom Right
		-1.0f, -1.0f,  // Bottom Left
		-1.0f, 1.0f	   // Top Left 
	};
	vector<GLuint> indices = {
		0, 1, 3,	// First Triangle
		1, 2, 3	// Second Triangle
	};

	fullScreenQuad.create();
	fullScreenQuad.indices = indices;
	fullScreenQuad.positions = positions;
	fullScreenQuad.updateFS();
}

Scene::~Scene() {}

void Scene::loadMeshes() {
	string inputfile = "rungholt.obj";
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;

	string err = tinyobj::LoadObj(shapes, materials, inputfile.c_str());

	if (!err.empty()) {
		std::cerr << err << std::endl;
		exit(1);
	}

	for (int i = 0; i < shapes.size(); i++) {
		meshes.push_back(Mesh());
	}

	for (int i = 0; i < shapes.size(); i++) {
		meshes[i].create();
		meshes[i].indices = shapes[i].mesh.indices;
		meshes[i].positions = shapes[i].mesh.positions;
		meshes[i].normals = shapes[i].mesh.normals;
		meshes[i].updateBuffers();
	}
}

void Scene::renderScene(Camera &cam) {
	//Set camera
	glm::mat4 mView = cam.getMView();
	glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(mView));
	glm::mat4 projection = glm::infinitePerspective(cam.zoom, 1.0f, 1.0f);

	//Clear buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(firstPass.program);
	gBuffer.bind();
	gBuffer.setDrawBuffers();
	firstPass.setUniformmat4("mView", mView);
	firstPass.setUniformmat4("projection", projection);
	firstPass.setUniformmat3("mNormal", normalMatrix);
	
	for (auto &i : meshes) {
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		i.renderFromBuffers();
	}

	gBuffer.unbind();

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glUseProgram(secondPass.program);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	secondPass.setUniformmat4("mView", mView);
	secondPass.setUniformmat4("projection", projection);
	secondPass.setUniformmat3("mNormal", normalMatrix);

	gBuffer.setTextures();
	secondPass.setUniformi("positionMap", 0);
	secondPass.setUniformi("normalMap", 1);
	secondPass.setUniformi("colorMap", 2);
	fullScreenQuad.drawFS();
}