#version 420 core

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D colorMap;
uniform samplerCube shadowMap;

uniform mat4 inverseMView;
uniform vec3 lPos;
uniform vec3 worldPos;
uniform float radius;
uniform vec3 lightColor;
uniform vec3 lightAttenuation;
uniform vec2 screenSize;

out vec4 fragColor;

const float specularPower = 16.0f;

float getShadowFactor(vec3 position) {
	float sDist = texture(shadowMap, normalize(worldPos - position)).x;
	float fDist = dot(worldPos - position, worldPos - position);

	if (fDist < sDist) return 1.0;
	else return 0.5;
}

void main() {
	vec2 coord = gl_FragCoord.xy / screenSize;
	vec3 n = normalize(texture(normalMap, coord).xyz);
	float s = texture(normalMap, coord).w;
	vec3 pos = texture(positionMap, coord).xyz;
	vec3 color = texture(colorMap, coord).xyz;
	
	float shadowFactor = getShadowFactor((inverseMView * vec4(pos, 1.0)).xyz);

	float r = length(lPos - pos);
	float attenuation = dot(lightAttenuation, vec3(1, r, r*r));
	vec3 l = (lPos - pos) / r;
	vec3 v = -normalize(pos);
	vec3 h = normalize(v + l);

	float ndotl = dot(n, l);
	vec3 diffuse = max(0.0f, ndotl) * color;

	vec3 specular = vec3(0);
	if (ndotl >= 0) specular = pow(max(0.0f, dot(n, h)), specularPower) * vec3(s);

	fragColor = vec4(shadowFactor * lightColor * (diffuse + specular), 1);
	fragColor = vec4(vec3(shadowFactor), 1);
}