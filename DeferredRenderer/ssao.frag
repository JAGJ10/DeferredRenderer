#version 420 core

in vec2 coord;

uniform mat4 mView;
uniform mat3 mNormal;
uniform mat4 projection;

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D colorMap;
uniform sampler2D depthMap;

out vec4 fragColor;

const vec2 poisson16[] = vec2[](vec2(-0.94201624, -0.39906216),
                                vec2(0.94558609, -0.76890725),
                                vec2(-0.094184101, -0.92938870),
                                vec2(0.34495938, 0.29387760),
                                vec2(-0.91588581, 0.45771432),
                                vec2(-0.81544232, -0.87912464),
                                vec2(-0.38277543, 0.27676845),
                                vec2(0.97484398, 0.75648379),
                                vec2(0.44323325, -0.97511554),
                                vec2(0.53742981, -0.47373420),
                                vec2(-0.26496911, -0.41893023),
                                vec2(0.79197514, 0.19090188),
                                vec2(-0.24188840, 0.99706507),
                                vec2(-0.81409955, 0.91437590),
                                vec2(0.19984126, 0.78641367),
                                vec2(0.14383161, -0.14100790)
                               );

float linearizeDepth(float depth) {
    float near = 0.1; 
    float far = 1000.0; 
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near) / (far + near - z * (far - near));	
}

void main() {
    vec3 norm = texture(normalMap, coord).xyz;
	vec3 pos = texture(positionMap, coord).xyz;
	float depth = texture(depthMap, coord).x;

	vec3 wsCoord = vec3(coord, depth);

	for (int i = 0; i < 16; i++) {
		
	}

	fragColor = vec4(linearizeDepth(depth));
}