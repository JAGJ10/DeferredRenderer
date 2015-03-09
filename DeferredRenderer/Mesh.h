#ifndef MESH_H
#define MESH_H

#include "common.h"
#include "VBO.h"

class Mesh {
public:
	std::vector<glm::vec3> vertices;
	std::vector<GLuint> indices;
	int numIndices;

	Mesh();
	
	void create(bool useBuffer = true);
	void updateBuffers();
	void clear();
	void renderFromBuffers();
	void setAttributes();
	void render();
	bool hasBuffer() const;

private:
	VBO positionBuffer;
	VBO indexBuffer;
};

#endif