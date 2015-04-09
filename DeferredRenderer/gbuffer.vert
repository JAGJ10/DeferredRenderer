#version 420 core

in vec3 position;
in vec3 normal;

uniform mat4 mView;
uniform mat4 projection;
uniform mat3 mNormal;

out vec3 fragPos;
out vec3 fragNormal;
out vec3 fragColor;

void main() {
    gl_Position = projection * mView * vec4(position, 1.0);
	fragPos = (mView * vec4(position, 1.0)).xyz;
	//fragNormal = normalize(mNormal * normal);
	fragNormal = normalize(normal);
	fragColor = vec3(0);
}