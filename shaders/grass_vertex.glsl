#version 400 core

#define M_PI 3.1415926535897932384626433832795

// out of the 8 random values given from the application, this shader uses three of them

layout(location=0) in vec4 pos;
layout(location=1) in vec4 centrePos;
layout(location=2) in vec4 texturePos;
layout(location=3) in vec4 random;

out vec3 vPos;
out vec4 vCentrePos;
out vec2 vTexturePos;
out vec4 vRandom;

uniform vec2 patchPos;
uniform vec3 fieldRect[2];
uniform mat4 objectTransform;
uniform float time;
uniform sampler2D forceMap;
uniform int windActive;

void main()
{
	vec3 newPos;

	// rotating the vertex about the blade centre. The angle pos.w is a random value from the application.
	float angle = 2*M_PI*pos.w;
	float newX = centrePos.x + cos(angle)*(pos.x - centrePos.x) - sin(angle)*(pos.z - centrePos.z);
	float newZ = centrePos.z + sin(angle)*(pos.x - centrePos.x) + cos(angle)*(pos.z - centrePos.z);

	newPos = vec3(newX, pos.y, newZ);

	// offset the upper vertices of a blade
	float maxBending = 0.03;
	vec3 offset = vec3(maxBending*(2*texturePos.z - 1.0), 0.0, maxBending*(2*texturePos.w - 1.0));

	vCentrePos = vec4((objectTransform * vec4(centrePos.xyz, 1)).xyz, centrePos.w);

	if (windActive == 1)
	{
		// adding some animation with a super basic wind function
		float c1 = 0.65;
		float wind = sin(c1*M_PI*vCentrePos.x + time);

		offset.x += wind*0.12;
		offset.z += wind*0.16;
	}

	vec3 fieldWorldPos = vec3(patchPos.x, 0.0, patchPos.y) + (pos.xyz + vec3(0.5, 0.0, 0.5));
	vec3 fieldRelativePos = fieldWorldPos - fieldRect[0];
	vec3 fieldDimensions = fieldRect[1] - fieldRect[0];

	vec2 mapPos = vec2(fieldRelativePos.x / fieldDimensions.x, fieldRelativePos.z / fieldDimensions.z);

	vec4 force = texture(forceMap, mapPos)*2 - vec4(1.0, 1.0, 1.0, 0.0);
	vec3 forceOffset = force.xyz;

	newPos += centrePos.y*offset;

	vec3 forceScale = vec3(0.4, 0.09, 0.4);
	newPos += centrePos.y*forceOffset*forceScale;

	vPos = newPos;
	vTexturePos = texturePos.xy;
	vRandom = random;
}