#version 420 core

in vec2 coord;

uniform mat4 mView;

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D colorMap;
uniform sampler2D ssaoMap;

out vec4 fragColor;

const vec4 lightDir = vec4(0, 1, 0, 1);
const float specularPower = 16.0f;
const vec3 lightColor = vec3(1);

void main() {
    vec3 n = normalize(texture(normalMap, coord).xyz);
	float s = texture(normalMap, coord).w;
	vec3 pos = texture(positionMap, coord).xyz;
	vec3 color = texture(colorMap, coord).xyz;
	float ssao = texture(ssaoMap, coord).x;

	vec3 l = normalize((mView * -lightDir).xyz);
	vec3 v = -normalize(pos);
	vec3 h = normalize(v + l);

	float ndotl = dot(n, l);
	vec3 diffuse = max(0.0f, ndotl) * color;

	vec3 specular = vec3(0);
	if (ndotl >= 0) specular = pow(max(0.0f, dot(n, h)), specularPower) * vec3(s);

	fragColor = vec4(lightColor * (diffuse * ssao + specular), 1);
	//fragColor = vec4(n, 1);
	//fragColor = vec4(pos, 1);
}