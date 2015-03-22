#ifndef AABB_H
#define AABB_H

#include "common.h"

class AABB {
public:
	glm::vec3 upper;
	glm::vec3 lower;

	AABB(glm::vec3 lower, glm::vec3 upper);

	bool operator==(const AABB &bb);
	bool operator!=(const AABB &bb);

	glm::vec3 getCenter() const;
	void setCenter(glm::vec3 center);
	glm::vec3 getLower() const;
	void setLower(glm::vec3 lower);
	glm::vec3 getUpper() const;
	void setUpper(glm::vec3 upper);
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