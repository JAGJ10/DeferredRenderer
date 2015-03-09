#include "AABB.h"

using namespace std;

AABB::AABB(glm::vec3 min, glm::vec3 max) : min(min), max(max) {
	calcCenter();
	calcHalfDims();
}

bool AABB::operator==(const AABB &bb) {
	return min == bb.min && max == bb.max;
}

bool AABB::operator!=(const AABB &bb) {
	return min != bb.min || max != bb.max;
}

glm::vec3 AABB::getCenter() const {
	return center;
}

void AABB::setCenter(glm::vec3 center) {
	this->center = center;
}

glm::vec3 AABB::getMin() const {
	return min;
}

void AABB::setMin(glm::vec3 min) {
	this->min = min;
}

glm::vec3 AABB::getMax() const {
	return max;
}

void AABB::setMax(glm::vec3 max) {
	this->max = max;
}

glm::vec3 AABB::getHalfDims() const {
	return halfDims;
}

bool AABB::intersects(AABB bb) const {
	if (max.x < bb.min.x)
		return false;
	if (max.y < bb.min.y)
		return false;
	if (max.z < bb.min.z)
		return false;
	if (min.x > bb.max.x)
		return false;
	if (min.y > bb.max.y)
		return false;
	if (min.z > bb.max.z)
		return false;

	return true;
}

bool AABB::intersects(glm::vec3 p1, glm::vec3 p2) const {
	glm::vec3 d = (p2 - p1) * 0.5f;
	glm::vec3 e = (max - min) * 0.5f;
	glm::vec3 c = p1 + d - (min + max) * 0.5f;
	glm::vec3 ad = glm::vec3(fabs(d.x), fabs(d.y), fabs(d.z));

	if (fabs(c.x) > e.x + ad.x)
		return false;
	if (fabs(c.y) > e.y + ad.y)
		return false;
	if (fabs(c.z) > e.z + ad.z)
		return false;

	float epsilon = numeric_limits<float>::epsilon();

	if (fabs(d.y * c.z - d.z * c.y) > e.y * ad.z + e.z * ad.y + epsilon)
		return false;
	if (fabs(d.z * c.x - d.x * c.z) > e.z * ad.x + e.x * ad.z + epsilon)
		return false;
	if (fabs(d.x * c.y - d.y * c.x) > e.x * ad.y + e.y * ad.x + epsilon)
		return false;

	return true;
}

bool AABB::contains(AABB bb) const {
	if (bb.min.x >= min.x && bb.max.x <= max.x && bb.min.y >= min.y && 
		bb.max.y <= max.y && bb.min.z >= min.z && bb.max.z <= max.z)
		return true;

	return false;
}

bool AABB::contains(glm::vec3 p) const {
	if (p.x >= min.x && p.x <= max.x && p.y >= min.y &&
		p.y <= max.y && p.z >= min.z && p.z <= max.z)
		return true;

	return false;
}

void AABB::calcCenter() {
	center = ((max - min) * 0.5f) + min;
}

void AABB::calcHalfDims() {
	halfDims = (max - min) * 0.5f;
}