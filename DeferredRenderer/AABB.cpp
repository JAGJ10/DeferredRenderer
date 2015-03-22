#include "AABB.h"

using namespace std;

AABB::AABB(glm::vec3 lower, glm::vec3 upper) : lower(lower), upper(upper) {
	calcCenter();
	calcHalfDims();
}

bool AABB::operator==(const AABB &bb) {
	return lower == bb.lower && upper == bb.upper;
}

bool AABB::operator!=(const AABB &bb) {
	return lower != bb.lower || upper != bb.upper;
}

glm::vec3 AABB::getCenter() const {
	return center;
}

void AABB::setCenter(glm::vec3 center) {
	this->center = center;
}

glm::vec3 AABB::getLower() const {
	return lower;
}

void AABB::setLower(glm::vec3 lower) {
	this->lower = lower;
}

glm::vec3 AABB::getUpper() const {
	return upper;
}

void AABB::setUpper(glm::vec3 upper) {
	this->upper = upper;
}

glm::vec3 AABB::getHalfDims() const {
	return halfDims;
}

bool AABB::intersects(AABB bb) const {
	if (upper.x < bb.lower.x)
		return false;
	if (upper.y < bb.lower.y)
		return false;
	if (upper.z < bb.lower.z)
		return false;
	if (lower.x > bb.upper.x)
		return false;
	if (lower.y > bb.upper.y)
		return false;
	if (lower.z > bb.upper.z)
		return false;

	return true;
}

bool AABB::intersects(glm::vec3 p1, glm::vec3 p2) const {
	glm::vec3 d = (p2 - p1) * 0.5f;
	glm::vec3 e = (upper - lower) * 0.5f;
	glm::vec3 c = p1 + d - (lower + upper) * 0.5f;
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
	if (bb.lower.x >= lower.x && bb.upper.x <= upper.x && bb.lower.y >= lower.y && 
		bb.upper.y <= upper.y && bb.lower.z >= lower.z && bb.upper.z <= upper.z)
		return true;

	return false;
}

bool AABB::contains(glm::vec3 p) const {
	if (p.x >= lower.x && p.x <= upper.x && p.y >= lower.y &&
		p.y <= upper.y && p.z >= lower.z && p.z <= upper.z)
		return true;

	return false;
}

void AABB::calcCenter() {
	center = ((upper - lower) * 0.5f) + lower;
}

void AABB::calcHalfDims() {
	halfDims = (upper - lower) * 0.5f;
}