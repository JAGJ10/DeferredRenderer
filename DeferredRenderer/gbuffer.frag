#version 420 core

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragColor;

out vec4 position;
out vec4 normal;
out vec4 color;

void main() {
	position = vec4(fragPos, 1.0);
	normal = vec4(fragNormal * 0.5 + 0.5, 1.0);
	color = vec4(fragColor, 1.0);
}