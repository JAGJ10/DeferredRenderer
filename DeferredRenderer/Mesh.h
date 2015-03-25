#ifndef MESH_H
#define MESH_H

#include "common.h"
#include "VBO.h"

class Mesh {
public:
	int numIndices;

	Mesh();
	~Mesh();

	void create();
	void updateBuffers(std::vector<float>& positions, std::vector<GLuint>& indices, std::vector<float>& normals);
	void clear();
	void renderFromBuffers();
	void setAttributes();
	bool hasBuffer() const;

private:
	GLuint vao;
	VBO positionBuffer;
	VBO indexBuffer;
	VBO normalBuffer;
};

#endif