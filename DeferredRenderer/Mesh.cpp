#include "Mesh.h"

Mesh::Mesh() : numIndices(0) {}

void Mesh::create(bool useBuffer) {
	if (useBuffer) {
		positionBuffer.create();
		indexBuffer.create();
		normalBuffer.create();
	}
}

void Mesh::updateBuffers() {
	numIndices = int(indices.size());

	positionBuffer.bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size() * 3, &positions[0], GL_STATIC_DRAW);

	normalBuffer.bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size() * 3, &normals[0], GL_STATIC_DRAW);

	indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);
}

void Mesh::clear() {
	positions.clear();
	normals.clear();
	indices.clear();
	numIndices = 0;
}

//Should be removed
void Mesh::renderFromBuffers() {
	positionBuffer.bind(GL_ARRAY_BUFFER);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float), 0);

	normalBuffer.bind(GL_ARRAY_BUFFER);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float), 0);

	indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
}

void Mesh::setAttributes() {
	if (hasBuffer()) {
		positionBuffer.bind(GL_ARRAY_BUFFER);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float), 0);

		normalBuffer.bind(GL_ARRAY_BUFFER);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float), 0);

		indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float), &positions[0]);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float), &normals[0]);
	}
}

bool Mesh::hasBuffer() const {
	return positionBuffer.created();
}