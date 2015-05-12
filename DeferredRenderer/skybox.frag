#version 420 core

in vec3 fragPos;

uniform samplerCube skybox;

out vec4 fragColor;

void main() {
	fragColor = texture(skybox, fragPos);
}
