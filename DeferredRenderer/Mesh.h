#ifndef MESH_H
#define MESH_H

#include "common.h"
#include "VBO.h"

class Mesh {
public:
	std::vector<float> positions;
	std::vector<GLuint> indices;
	std::vector<float> normals;
	int numIndices;

	Mesh();
	
	void create();
	void updateBuffers();
	void clear();
	void renderFromBuffers();
	void setAttributes();
	bool hasBuffer() const;

	void updateFS();
	void drawFS();

private:
	GLuint vao;
	VBO positionBuffer;
	VBO indexBuffer;
	VBO normalBuffer;
};

#endif