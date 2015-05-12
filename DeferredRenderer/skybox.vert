#version 420 core

in vec2 position;

uniform mat4 inverseVP;

out vec3 fragPos;

void main() {
  vec4 temp = inverseVP * vec4(position, 1.0, 1.0);
  gl_Position = vec4(position, 1.0, 1.0);
  //fragPos = temp.xyz / temp.w;
  fragPos = temp.xyz;
}
