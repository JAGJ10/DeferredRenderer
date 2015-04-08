#version 420 core

in vec2 coord;

uniform mat4 mView;
uniform mat3 mNormal;
uniform mat4 projection;

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D colorMap;
uniform sampler2D depthMap;
uniform sampler2D noiseMap;

out vec4 fragColor;

const vec3 lightPos = vec3(0, 1000, 0);

float linearizeDepth(float depth) {
    float near = 0.1; 
    float far = 1000.0; 
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near) / (far + near - z * (far - near));	
}

void main() {
    vec3 norm = texture(normalMap, coord).xyz;
	vec3 pos = texture(positionMap, coord).xyz;
	vec3 s = normalize(lightPos - pos);
	vec3 diffuse = vec3(0.38, 0.36, 0.34) + max(dot(s, norm), 0.0);

	float depth = texture(depthMap, coord).x;

	//fragColor = vec4(diffuse, 1);
	//fragColor = vec4(normalize(norm * 2 - 1), 1);
	//fragColor = vec4(linearizeDepth(depth));
	fragColor = texture(noiseMap, coord);
}