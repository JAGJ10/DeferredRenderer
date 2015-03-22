#version 420 core

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragColor;

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;
layout(location = 2) out vec4 color;

void main() {
	position = vec4(fragPos, 1.0);
	normal = vec4(fragNormal, 1.0);
	color = vec4(fragColor, 1.0);
}