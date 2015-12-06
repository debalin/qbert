#version 330 core

out vec3 color;
in vec3 normalCam;
uniform vec3 lightLa[5];
uniform vec3 lightLd[5];
uniform vec3 lightLs[5];
uniform sampler2D textureID;
uniform sampler2D fontID;
uniform int texturePresent;
uniform float NVar;
uniform vec3 kaVar;
uniform vec3 kdVar;
uniform vec3 ksVar;
uniform vec3 colorTextVar;
in vec3 lightVectorCam[5];
in vec3 halfVectorCam[5];
in vec2 textureUVVar;
in float numOfLightsVar;
in float fontVar;
in float lightSwitchFS;

void main() {

	vec3 normal = normalize(normalCam);
	vec3 lightVector[5];
	vec3 halfVector[5];
	vec3 ka = kaVar;
	vec3 kd = kdVar;
	vec3 ks = ksVar;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	if (texturePresent == 1) {
		ka = ka * (texture2D(textureID, textureUVVar).rgb);
		kd = kd * (texture2D(textureID, textureUVVar).rgb);
		ks = ks * (texture2D(textureID, textureUVVar).rgb);
	}

	if (lightSwitchFS == 1) {
		for (int i = 0; i < numOfLightsVar; i++) {
			lightVector[i] = normalize(lightVectorCam[i]);
			halfVector[i] = normalize(halfVectorCam[i]);
			ambient += lightLa[i] * ka;
			diffuse += lightLd[i] * kd * clamp(dot(lightVector[i], normal), 0, 1);  
			specular += lightLs[i] * ks * pow(clamp(dot(normal, halfVector[i]), 0, 1), ((NVar / 1000) * 128));
		}
	}
	else {
		ambient += ka / 3.2;
		diffuse += kd / 3.2;
		specular += ks / 3.2;
	}

	color = ambient + diffuse + specular;
	if (fontVar == 1) {
		color = texture2D(fontID, textureUVVar).rgb * colorTextVar;
	}

}