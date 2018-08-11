#version 400 core

// out of the 5 random values from the vertex shader, this shader uses two

layout(vertices = 4) out;

in vec3 vPos[];
in vec4 vCentrePos[];
in vec2 vTexturePos[];
in vec4 vRandom[];

patch out vec3 controlPoints[2];

out vec3 tcPos[];
out vec3 tcCentrePos[];
out vec2 tcTexturePos[];
out vec3 tcRandom[];

uniform vec3 cameraPos;
uniform mat4 objectTransform;
uniform mat4 viewTransform;

vec3 calcControlPoint(vec3 lower, vec3 upper)
{
	// scaled between -1/4 & 1/4
	float rand1 = vCentrePos[gl_InvocationID].w*0.5 - 0.25;
	// scaled between 3/4 & 5/4
	float rand2 = vRandom[gl_InvocationID].w*0.5 + 0.25;

	float x = lower.x*rand1 + upper.x*(1.0 - rand1);
	float y = lower.y*rand2 + upper.y*(1.0 - rand2);
	float z = lower.z*rand1 + upper.z*(1.0 - rand1);
	return vec3(x, y, z);
}

void main()
{
	float maxDistance = 15.0;
	float maxTessellation = 8.0;

	if (gl_InvocationID == 0)
	{
		float cameraDistance = length(vCentrePos[0].xyz - cameraPos);
		float distanceRatio = cameraDistance / maxDistance;

		float tessLevel = 0;
		if (distanceRatio < 1.0)
		   tessLevel = ceil(maxTessellation*(1.0 - distanceRatio));

		//NOTE(denis): quads are defined counter clockwise, starting with 0 at bottom-left corner
		gl_TessLevelOuter[0] = tessLevel; // left edge
		gl_TessLevelOuter[1] = 1.0; // bottom edge
		gl_TessLevelOuter[2] = tessLevel; // right edge
		gl_TessLevelOuter[3] = 1.0; // top edge
		
		gl_TessLevelInner[0] = tessLevel; // top and bottom
		gl_TessLevelInner[1] = tessLevel; // left and right

		controlPoints[0] = calcControlPoint(vPos[1], vPos[0]);
		controlPoints[1] = calcControlPoint(vPos[2], vPos[3]);
	}

	tcPos[gl_InvocationID] = vPos[gl_InvocationID];
	tcCentrePos[gl_InvocationID] = vCentrePos[gl_InvocationID].xyz;
	tcTexturePos[gl_InvocationID] = vTexturePos[gl_InvocationID];
	tcRandom[gl_InvocationID] = vRandom[gl_InvocationID].xyz;
}