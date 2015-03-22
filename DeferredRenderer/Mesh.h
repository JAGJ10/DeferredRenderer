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
	VBO normalBuffer;
};

#endif