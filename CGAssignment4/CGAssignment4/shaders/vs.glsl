#version 330 core

// input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normalInput;
layout (location = 2) in vec2 textureUV;
layout (location = 4) in vec2 fontPosition;
out vec3 normalCam;
uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightLocation[5];
uniform int numOfLights;
uniform int fontPresent;
uniform int lightSwitch;
out vec3 lightVectorCam[5];
out vec3 halfVectorCam[5];
out vec2 textureUVVar;
out float numOfLightsVar;
out float fontVar;
out float lightSwitchFS;

void main() {

	gl_Position = projection * view * transform * vec4(position, 1.0f);
	if (fontPresent == 1) {
		gl_Position = vec4(fontPosition, 0.8, 1);
	}

	if (lightSwitch == 1) {
		vec3 positionCam = (view * transform * vec4(position, 1.0f)).xyz;

		vec4 normalTemp = view * transform * vec4(normalInput, 0.0f);
		normalCam = normalize(normalTemp.xyz);

		vec3 lightLocationCam;

		for (int i = 0; i < numOfLights; i++) {
			lightLocationCam = (view * vec4(lightLocation[i], 1.0f)).xyz;
			lightVectorCam[i] = normalize(lightLocationCam - positionCam);

			halfVectorCam[i] = normalize((lightLocationCam - positionCam) + (vec3(0, 0, 0) - positionCam));
		}
	}

	textureUVVar = vec2(textureUV.x, 1.0f - textureUV.y);
	numOfLightsVar = numOfLights;
	fontVar = fontPresent;
	lightSwitchFS = lightSwitch;

}