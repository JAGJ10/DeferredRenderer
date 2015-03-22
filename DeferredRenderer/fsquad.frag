#version 420 core

in vec2 coord;

uniform mat4 mView;
uniform mat3 mNormal;
uniform mat4 projection;

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D colorMap;

out vec4 fragColor;

void main() {
    fragColor = texture(normalMap, coord);
	//fragColor = vec4(1, 0, 0, 1);
}