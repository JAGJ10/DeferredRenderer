#version 420 core

in vec3 position;

uniform mat4 mView;
uniform mat4 projection;

uniform vec3 worldPos;
uniform float radius;
uniform vec3 lPos;

void main() {
	vec3 wPos = (position * radius) + worldPos;
	gl_Position = projection * mView * vec4(wPos, 1.0);
}