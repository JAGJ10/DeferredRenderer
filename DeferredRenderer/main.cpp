#define GLEW_DYNAMIC
#include "common.h"
#include <assert.h>
#include "tiny_obj_loader.h"
#include <GLFW/glfw3.h>
#include "Camera.hpp"
#include "Scene.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

std::vector<tinyobj::shape_t> read(std::istream& stream);
void write(std::ostream& stream, const std::vector<tinyobj::shape_t>& shapes);

static const int width = 512;
static const int height = 512;
static const GLfloat lastX = (width / 2);
static const GLfloat lastY = (height / 2);
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void handleInput(GLFWwindow* window, Camera &cam);

int main() {
	//Checks for memory leaks in debug mode
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(width, height, "Deferred Renderer", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	//Set callbacks for keyboard and mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();

	// Define the viewport dimensions
	glViewport(0, 0, width, height);

	Camera cam = Camera();
	Scene scene(width, height);
	scene.loadMeshes();

	while (!glfwWindowShouldClose(window)) {
		//Set frame times
		float currentFrame = float(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		handleInput(window, cam);

		scene.renderScene(cam);

		// Swap the buffers
		glfwSwapBuffers(window);

		glfwSetCursorPos(window, lastX, lastY);
	}

	glfwTerminate();

	return 0;
}

void handleInput(GLFWwindow* window, Camera &cam) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cam.wasdMovement(FORWARD, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cam.wasdMovement(BACKWARD, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cam.wasdMovement(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cam.wasdMovement(LEFT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		cam.wasdMovement(UP, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		cam.wasdMovement(DOWN, deltaTime);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		cam.mouseMovement((float(xpos) - lastX), (lastY - float(ypos)), deltaTime);
}

void write(std::ostream& stream, const std::vector<tinyobj::shape_t>& shapes) {
	assert(sizeof(float) == sizeof(int));
	const auto sz = sizeof(int);
	const int nMeshes = static_cast<int>(shapes.size());
	const int nMatProperties = 3;

	stream.write((const char*)&nMeshes, sz);        // nMeshes
	stream.write((const char*)&nMatProperties, sz); // nMatProperties

	for (size_t i = 0; i < nMeshes; ++i) {
		const int nVertices = (int)shapes[i].mesh.positions.size();
		const int nNormals = (int)shapes[i].mesh.normals.size();
		const int nTexcoords = (int)shapes[i].mesh.texcoords.size();
		const int nIndices = (int)shapes[i].mesh.indices.size();

		// Write nVertices, nNormals,, nTexcoords, nIndices
		// Write #nVertices positions
		// Write #nVertices normals
		// Write #nVertices texcoord
		// Write #nIndices  indices
		// Write #nMatProperties material properties
		stream.write((const char*)&nVertices, sz);
		stream.write((const char*)&nNormals, sz);
		stream.write((const char*)&nTexcoords, sz);
		stream.write((const char*)&nIndices, sz);

		stream.write((const char*)&shapes[i].mesh.positions[0], nVertices  * sz);
		stream.write((const char*)&shapes[i].mesh.normals[0], nNormals   * sz);
		stream.write((const char*)&shapes[i].mesh.texcoords[0], nTexcoords * sz);
		stream.write((const char*)&shapes[i].mesh.indices[0], nIndices   * sz);
		//stream.write((const char*)&shapes[i].material.ambient[0], 3 * sz);
	}
}


std::vector<tinyobj::shape_t> read(std::istream& stream) {
	assert(sizeof(float) == sizeof(int));
	const auto sz = sizeof(int);

	std::vector<tinyobj::shape_t> shapes;

	int nMeshes = 0;
	int nMatProperties = 0;
	stream.read((char*)&nMeshes, sz);
	stream.read((char*)&nMatProperties, sz);
	shapes.resize(nMeshes);
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

		stream.read((char*)&shapes[i].mesh.positions[0], nVertices  * sz);
		stream.read((char*)&shapes[i].mesh.normals[0], nNormals   * sz);
		stream.read((char*)&shapes[i].mesh.texcoords[0], nTexcoords * sz);
		stream.read((char*)&shapes[i].mesh.indices[0], nIndices   * sz);
		//stream.read((char*)&shapes[i].material.ambient[0], 3 * sz);
	}

	return shapes;
}