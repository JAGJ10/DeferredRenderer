#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

#include "common.h"

class ShadowMap {
public:
	GLuint depth;

	ShadowMap(int width, int height);
	~ShadowMap();

private:
	GLuint fbo;

	int width, height;
};

#endif