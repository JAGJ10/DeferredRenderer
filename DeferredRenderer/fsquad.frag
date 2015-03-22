#version 420 core

in vec2 coord;

uniform mat4 mView;
uniform mat3 mNormal;
uniform mat4 projection;

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D colorMap;

out vec4 fragColor;

const vec3 lightPos = vec3(0, 1000, 0);

void main() {
    vec3 norm = texture(normalMap, coord).xyz;
	vec3 pos = texture(positionMap, coord).xyz;
	vec3 s = normalize(lightPos - pos);
	vec3 diffuse = vec3(0.38, 0.36, 0.34) + max(dot(s, norm), 0.0);
	fragColor = vec4(diffuse, 1);
	//fragColor = vec4(1, 0, 0, 1);
}