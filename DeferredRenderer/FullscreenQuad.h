#ifndef FULLSCREEN_QUAD_H
#define FULLSCREEN_QUAD_H

#include "common.h"
#include "VBO.h"

class FullscreenQuad {
public:
	int numIndices;
	FullscreenQuad();
	~FullscreenQuad();

	void create();
	void clear();
	void updateBuffers(std::vector<float>& positions, std::vector<GLuint>& indices);
	void renderFromBuffers();

private:
	GLuint vao;
	VBO positionBuffer;
	VBO indexBuffer;
};

#endif