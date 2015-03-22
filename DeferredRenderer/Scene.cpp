#include "Scene.h"
#include "tiny_obj_loader.h"

using namespace std;

Scene::Scene(int width, int height) : gBuffer(width, height), firstPass("gbuffer.vert", "gbuffer.frag"), secondPass("gbuffer.vert", "gbuffer.frag") {

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

	for (auto &i : shapes) {
		Mesh m = Mesh();
		m.create();
		m.indices = i.mesh.indices;
		m.positions = i.mesh.positions;
		m.normals = i.mesh.normals;
		m.updateBuffers();
		meshes.push_back(m);
	}
}

void Scene::renderScene(Camera &cam) {
	//visibleObjects = octree.findObjects(region);
	//Set camera
	glm::mat4 mView = cam.getMView();
	glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(mView));
	glm::mat4 projection = glm::perspective(cam.zoom, 1.0f, 1.0f, 100.0f);

	//Clear buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(firstPass.program);
	gBuffer.bindDraw();
	gBuffer.setDrawBuffers();
	firstPass.setUniformmat4("mView", mView);
	firstPass.setUniformmat4("projection", projection);
	firstPass.setUniformmat3("mNormal", normalMatrix);

	for (auto &i : meshes) {
		i.renderFromBuffers();
	}

	gBuffer.unbindDraw();
	
	glUseProgram(secondPass.program);

}