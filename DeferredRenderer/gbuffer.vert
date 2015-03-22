#version 420 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 mView;
uniform mat4 projection;
uniform mat3 mNormal;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragColor;

void main() {
    gl_Position = projection * mView * vec4(position, 1.0);
	fragPos = (mView * vec4(position, 1.0)).xyz;
	fragNormal = normalize(mNormal * normal);
	fragColor = vec3(0);
}