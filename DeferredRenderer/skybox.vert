#version 420 core

in vec2 position;

uniform mat4 inverseVP;

out vec3 fragPos;

void main() {
  fragPos = normalize((inverseVP * vec4(position, 1.0, 1.0)).xyz);
  gl_Position = vec4(position, 1.0, 1.0);
}
