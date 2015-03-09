#ifndef AABB_H
#define AABB_H

#include "common.h"

class AABB {
public:
	glm::vec3 max;
	glm::vec3 min;

	AABB(glm::vec3 min, glm::vec3 max);

	bool operator==(const AABB &bb);
	bool operator!=(const AABB &bb);

	glm::vec3 getCenter() const;
	void setCenter(glm::vec3 center);
	glm::vec3 getMin() const;
	void setMin(glm::vec3 min);
	glm::vec3 getMax() const;
	void setMax(glm::vec3 max);
	glm::vec3 getHalfDims() const;

	bool intersects(AABB bb) const;
	bool intersects(glm::vec3 p1, glm::vec3 p2) const;
	bool contains(AABB bb) const;
	bool contains(glm::vec3 p) const;

private:
	glm::vec3 center;
	glm::vec3 halfDims;

	void calcCenter();
	void calcHalfDims();
};

#endif