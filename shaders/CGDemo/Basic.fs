#version 430 core

layout(location=0) out vec4 out_color;
 
in vec4 vertexColor; // Now interpolated across face

in vec4 interPos;
in vec3 interNormal;

in vec2 interUV;

struct PointLight {
	vec4 pos;
	vec4 color;
};
uniform PointLight light;

uniform float metallic;
uniform float roughness;

uniform sampler2D diffuseTex;

const float PI = 3.14159265359;

vec3 getFresnelAtAngleZero(vec3 albedo, float metallic) {
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);
	return F0;
}

vec3 getFresnel(vec3 F0, vec3 L, vec3 H) {
	float cosAngle = max(0, dot(L,H));
	vec3 F = F0 + (1 - F0)*pow(1 - max(0, cosAngle), 5);
	return F;
}

float getNDF(vec3 H, vec3 N, float roughness) {
	float a = roughness*roughness;
	float a2 = a*a;
	float dotVal = max(0, dot(H,N));
	dotVal *= dotVal;
	float denom = dotVal*(a2 - 1) + 1;
	denom *= denom;
	denom *= PI;
	float NDF = a2/denom;
	return NDF;
}

float getSchlickGeo(vec3 B, vec3 N, float roughness) {
	float k = pow(roughness + 1, 2) / 8;
	float G = dot(N, B) / (dot(N, B)*(1 - k) + k);
	return G;
}

float getGF(vec3 L, vec3 V, vec3 N, float roughness) {
	float GL = getSchlickGeo(L, N, roughness);
	float GV = getSchlickGeo(V, N, roughness);
	return GL*GV;
}

void main()
{	


	vec4 texColor = texture(diffuseTex, interUV);

	vec4 baseColor = texColor; //vertexColor;

	vec3 N = normalize(interNormal);
	vec3 L = vec3(normalize(light.pos - interPos));
	float diffCoff = max(0, dot(N,L));
	vec3 diffColor = vec3(diffCoff*baseColor*light.color);

	vec3 V = normalize(-vec3(interPos));
	vec3 F0 = getFresnelAtAngleZero(vec3(baseColor), metallic);
	vec3 H = normalize(V + L);
	vec3 F = getFresnel(F0, L, H);
	vec3 kS = F;	
	vec3 kD = 1.0 - kS;

	kD *= (1.0 - metallic);
	kD *= vec3(baseColor);
	kD /= PI;
	
	float NDF = getNDF(H, N, roughness);
	float G = getGF(L, V, N, roughness);
	kS *= G;
	kS *= NDF;
	kS /= (4.0 * max(0, dot(N,L)) * max(0, dot(N,V))) + 0.0001;

	vec3 finalColor = (kD + kS)*vec3(light.color)*max(0, dot(N,L));

	finalColor = finalColor / (finalColor + 1.0);

	out_color = vec4(finalColor, 1.0);
}
