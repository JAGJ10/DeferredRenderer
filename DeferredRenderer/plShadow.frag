#version 420 core

in vec3 fragPos;

uniform mat4 inverseMView;
uniform vec3 worldPos;

out float squaredDist;

void main(void) {
	vec3 dist = worldPos - fragPos;
	squaredDist = dot(dist, dist);
} 