#version 420 core

in vec2 coord;

uniform mat4 mView;
uniform mat3 mNormal;
uniform mat4 projection;

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D colorMap;
uniform sampler2D depthMap;
uniform sampler2D ssaoMap;

out vec4 fragColor;

const vec3 lightDir = vec3(0, -1, 0);
const float specularPower = 16.0f;
const vec3 lightColor = vec3(1, 1, 1);

float linearizeDepth(float depth) {
    float near = 0.1; 
    float far = 1000.0; 
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near) / (far + near - z * (far - near));	
}

void main() {
    vec3 n = normalize(texture(normalMap, coord).xyz);
	float s = texture(normalMap, coord).w;
	vec3 pos = texture(positionMap, coord).xyz;
	vec3 color = texture(colorMap, coord).xyz;
	float depth = texture(depthMap, coord).x;
	float ssao = texture(ssaoMap, coord).x;
	
	vec3 v = -normalize(pos);

	vec3 l = normalize(lightDir - pos);
	vec3 h = normalize(v + l);

	float r = length(lightDir - pos);
	float attenuation = 1; //don't attenuate for directional light
	float ndotl = dot(n, l);
	vec3 diffuse = max(0.0f, ndotl) * color;

	vec3 specular = vec3(0);
	if (ndotl >= 0) specular = pow(max(0.0f, dot(n, h)), specularPower) * vec3(s);

	fragColor = vec4(lightColor * (diffuse * ssao + specular) / attenuation, 1);
}