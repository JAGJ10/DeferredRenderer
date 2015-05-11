#version 420 core

uniform vec3 lPos;
uniform vec2 screenSize;
uniform sampler2D positionMap;

out float squaredDist;

void main(void) {
	vec2 coord = gl_FragCoord.xy / screenSize;
	vec3 pos = texture(positionMap, coord).xyz;

	vec3 dist = pos - lPos;

	squaredDist = dot(dist, dist);
} 