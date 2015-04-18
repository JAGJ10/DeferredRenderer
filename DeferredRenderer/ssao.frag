#version 420 core

in vec2 coord;

uniform mat4 projection;

uniform int kernelSize;
uniform vec2 noiseScale;
uniform vec3 kernel[500];

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D noiseMap;

out vec4 fragColor;

const float radius = 10;
const int occlPower = 1;

void main() {
    vec3 normal = normalize(texture(normalMap, coord).xyz);
	vec3 pos = texture(positionMap, coord).xyz;

	vec3 origin = pos;

	vec3 rVec = texture(noiseMap, coord * noiseScale).xyz * 2.0 - 1.0;
	vec3 tangent = normalize(rVec - normal * dot(rVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bitangent, normal);

	float occlusion = 0.0f;
	for (int i = 0; i < kernelSize; i++) {
		//Get sample position
		vec3 s = tbn * kernel[i];
		s = s * radius + origin;

		//Project sample position
		vec4 offset = vec4(s, 1.0);
		offset = projection * offset;
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;

		//Get sample depth
		float sampleDepth = texture(positionMap, offset.xy).z;

		//Range check and accumulate
		float rangeCheck = abs(origin.z - sampleDepth) < radius ? 1.0 : 0.0;
		occlusion += (sampleDepth >= s.z ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion = 1.0 - (occlusion / float(kernelSize));

	fragColor = vec4(vec3(pow(occlusion, occlPower)), 1);
}