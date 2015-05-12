#version 420 core

uniform mat4 inverseMView;
uniform vec3 worldPos;
uniform vec2 screenSize;
uniform sampler2D positionMap;

out float squaredDist;

void main(void) {
	vec2 coord = gl_FragCoord.xy / screenSize;
	vec3 pos = (inverseMView * texture(positionMap, coord)).xyz;

	vec3 dist = worldPos - pos;

	squaredDist = dot(dist, dist);
} 