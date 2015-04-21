#ifndef MESH_HELPER_H
#define MESH_HELPER_H

#include "common.h"

inline void generatePatchedSphere(std::vector<glm::vec3> &positions, std::vector<GLuint> &indices) {
	float m = 5; //squares per face
	for (int i = 0; i <= m - 1; i += 1) {
		for (int j = 0; j <= m - 1; j += 1) {
			float sa = -1.0f + 2.0f * (i / m);
			float sb = -1.0f + 2.0f * ((i + 1) / m);
			float ta = -1.0f + 2.0f * (j / m);
			float tb = -1.0f + 2.0f * ((j + 1) / m);
			GLuint i = GLuint(positions.size());
			positions.push_back(glm::normalize(glm::vec3(sa, tb, 1.0f)));
			positions.push_back(glm::normalize(glm::vec3(sb, tb, 1.0f)));
			positions.push_back(glm::normalize(glm::vec3(sb, ta, 1.0f)));
			positions.push_back(glm::normalize(glm::vec3(sa, ta, 1.0f)));
			indices.push_back(i + 0); indices.push_back(i + 1); indices.push_back(i + 2);
			indices.push_back(i + 2); indices.push_back(i + 3); indices.push_back(i + 0);
			i += 4;
			positions.push_back(glm::normalize(glm::vec3(sa, ta, -1.0f)));
			positions.push_back(glm::normalize(glm::vec3(sb, ta, -1.0f)));
			positions.push_back(glm::normalize(glm::vec3(sb, tb, -1.0f)));
			positions.push_back(glm::normalize(glm::vec3(sa, tb, -1.0f)));
			indices.push_back(i + 0); indices.push_back(i + 1); indices.push_back(i + 2);
			indices.push_back(i + 2); indices.push_back(i + 3); indices.push_back(i + 0);
			i += 4;
			positions.push_back(glm::normalize(glm::vec3(sa, 1.0f, ta)));
			positions.push_back(glm::normalize(glm::vec3(sb, 1.0f, ta)));
			positions.push_back(glm::normalize(glm::vec3(sb, 1.0f, tb)));
			positions.push_back(glm::normalize(glm::vec3(sa, 1.0f, tb)));
			indices.push_back(i + 0); indices.push_back(i + 1); indices.push_back(i + 2);
			indices.push_back(i + 2); indices.push_back(i + 3); indices.push_back(i + 0);
			i += 4;
			positions.push_back(glm::normalize(glm::vec3(sa, -1.0f, tb)));
			positions.push_back(glm::normalize(glm::vec3(sb, -1.0f, tb)));
			positions.push_back(glm::normalize(glm::vec3(sb, -1.0f, ta)));
			positions.push_back(glm::normalize(glm::vec3(sa, -1.0f, ta)));
			indices.push_back(i + 0); indices.push_back(i + 1); indices.push_back(i + 2);
			indices.push_back(i + 2); indices.push_back(i + 3); indices.push_back(i + 0);
			i += 4;
			positions.push_back(glm::normalize(glm::vec3(-1.0f, sa, ta)));
			positions.push_back(glm::normalize(glm::vec3(-1.0f, sb, ta)));
			positions.push_back(glm::normalize(glm::vec3(-1.0f, sb, tb)));
			positions.push_back(glm::normalize(glm::vec3(-1.0f, sa, tb)));
			indices.push_back(i + 0); indices.push_back(i + 1); indices.push_back(i + 2);
			indices.push_back(i + 2); indices.push_back(i + 3); indices.push_back(i + 0);
			i += 4;
			positions.push_back(glm::normalize(glm::vec3(1.0f, sa, tb)));
			positions.push_back(glm::normalize(glm::vec3(1.0f, sb, tb)));
			positions.push_back(glm::normalize(glm::vec3(1.0f, sb, ta)));
			positions.push_back(glm::normalize(glm::vec3(1.0f, sa, ta)));
			indices.push_back(i + 0); indices.push_back(i + 1); indices.push_back(i + 2);
			indices.push_back(i + 2); indices.push_back(i + 3); indices.push_back(i + 0);
			i += 4;
		}
	}
}

#endif