#version 420 core

layout(location = 0) in vec3 position;

uniform mat4 mvp;

out vec3 fragPos;

void main() {
	gl_Position = mvp * vec4(position, 1.0);
	fragPos = position;
}