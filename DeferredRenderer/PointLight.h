#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "common.h"

struct PointLight {
	glm::vec3 position;
	glm::vec3 color;
	float radius;
};

#endif