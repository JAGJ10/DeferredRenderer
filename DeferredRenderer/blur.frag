#version 420 core

in vec2 coord;

uniform sampler2D ssaoMap;

out vec4 fragColor;

const int blurSize = 1;

void main() {
	vec4 result = vec4(0.0);
	vec2 hlim = vec2(float(-blurSize) * 0.5 + 0.5);
	for (int x = -blurSize; x < blurSize; x++) {
		for (int y = -blurSize; y < blurSize; y++) {
			vec2 offset = (hlim + vec2(float(x), float(y))) * vec2(1.0 / 1280, 1.0 / 720); //TODO: pass in;
			result += texture(ssaoMap, coord + offset);
		}
	}

	fragColor = result / float(blurSize * 4 * blurSize);
}

/*vec4 bilateralFilter() {
	vec4 color = vec4(0.0);
	vec2 center = gl_TexCoord[0].xy;
	vec2 sample;
	float sum = 0.0;
	float coefG,coefZ,finalCoef;
	float Zp = getDepth(center);

	const float epsilon = 0.01;

	for(int i = -(kernelSize-1)/2; i <= (kernelSize-1)/2; i++) {
		for(int j = -(kernelSize-1)/2; j <= (kernelSize-1)/2; j++) {
			sample = center + vec2(i,j) / texSize;
			coefG = gaussianCoef(i,j);
			float zTmp = getDepth(sample);
			coefZ = 1.0 / (epsilon + abs(Zp - zTmp));
			finalCoef = coefG * coefZ;
			sum += finalCoef;
			color += finalCoef * texture2D(AOTex,sample);
		}
	}

	return color / sum;
}*/