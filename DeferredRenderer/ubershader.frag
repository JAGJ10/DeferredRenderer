#version 420 core

in vec2 coord;

uniform mat4 inverseMView;

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D colorMap;
uniform sampler2D lightMap;
uniform sampler2D ssaoMap;
uniform sampler2DShadow shadowMap;

uniform vec3 l;
uniform mat4 shadowMapMVP;
uniform int shadowMapWidth;
uniform int shadowMapHeight;

uniform int type;

out vec4 fragColor;

const float specularPower = 16.0f;
const vec3 lightColor = vec3(0.5);

float linearizeDepth(float depth) {
	float f = 2000.0;
	float n = 10.0;
	return (2 * n) / (f + n - depth * (f - n));
}

float getShadowFactor(vec3 position) {
	vec4 p = inverseMView * vec4(position, 1.0);
	p = shadowMapMVP * p;
	p /= p.w;
	p.xyz = p.xyz * 0.5 + 0.5;

	float factor = 0;

	vec2 offset = vec2(1.0 / float(shadowMapWidth), 1.0 / float(shadowMapHeight));

	for (int y = -1; y <= 1; y++) {
		for (int x = -1; x <= 1; x++) {
			vec3 uvc = vec3(p.xy + (vec2(x,y) * offset), p.z - 0.005);
			factor += texture(shadowMap, uvc);
        }
    }

	return (0.5 + (factor / 18));
}

void main() {
    vec3 n = normalize(texture(normalMap, coord).xyz);
	float s = texture(normalMap, coord).w;
	vec3 pos = texture(positionMap, coord).xyz;
	vec3 color = texture(colorMap, coord).xyz;
	vec3 light = texture(lightMap, coord).xyz;
	float ssao = texture(ssaoMap, coord).x;
	
	float shadowFactor = getShadowFactor(pos);

	vec3 ambient = color * 0.1;

	vec3 v = -normalize(pos);
	vec3 h = normalize(v + l);

	float ndotl = dot(n, l);
	vec3 diffuse = max(0.0f, ndotl) * color;

	vec3 specular = vec3(0);
	if (ndotl >= 0) specular = pow(max(0.0f, dot(n, h)), specularPower) * vec3(s);

	vec3 finalColor = lightColor * (diffuse + specular);

	if (type == 0) {
		fragColor = vec4(color, 1);
	} else if (type == 1) {
		fragColor = vec4(n, 1);
	} else if (type == 2) {
		fragColor = vec4(ssao);
	} else if (type == 3) {
		fragColor = vec4(finalColor, 1);
	} else if (type == 4) {
		fragColor = vec4(light * ssao, 1);
	} else if (type == 5) {
		fragColor = vec4((ambient + finalColor) * ssao, 1);
	} else if (type == 6) {
		fragColor = vec4((ambient + (finalColor * shadowFactor)) * ssao, 1);
	} else if (type == 7) {
		fragColor = vec4((ambient + light + (finalColor * shadowFactor)) * ssao, 1);
	}
}