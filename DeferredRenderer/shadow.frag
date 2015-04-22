#version 420 core

in vec2 coord;

uniform mat4 inverseMView;
uniform mat4 lightMView;

uniform sampler2D positionMap;

void main() {
	gl_FragDepth = (lightMView * (inverseMView * vec4(texture(positionMap, coord).xyz, 1))).z;
	//gl_FragDepth = texture(positionMap, coord).z;
}