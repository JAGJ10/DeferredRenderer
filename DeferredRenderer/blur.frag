#version 420 core

in vec2 coord;

uniform sampler2D ssaoTex;

out vec4 fragColor;

const int blurSize = 2;

void main() {
	float result = 0.0;
	vec2 hlim = vec2(float(-blurSize) * 0.5 + 0.5);
	for (int x = -blurSize; x < blurSize; x++) {
		for (int y = -blurSize; y < blurSize; y++) {
			vec2 offset = (hlim + vec2(float(x), float(y))) * texelSize;
			result += texture(ssaoTex, coord + offset).x;
		}
	}

	fragColor = result / float(blurSize * 4 * blurSize);
}