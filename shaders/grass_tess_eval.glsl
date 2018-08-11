#version 400 core

// this shader uses none of the 3 random values from the TCS

layout(quads, equal_spacing, ccw) in;

patch in vec3 controlPoints[2];

in vec3 tcPos[];
in vec3 tcCentrePos[];
in vec2 tcTexturePos[];
in vec3 tcRandom[];

out vec3 teCentrePos;
out vec3 teNormal;
out vec2 teTexturePos;
out vec3 teRandom;

uniform mat4 objectTransform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;

struct SplineData
{
	vec3 pos;

	vec3 tangent;
	vec3 a;
};

// we use De Casteljau's algorithm for spline calculation
SplineData calcSplinePos(vec3 p1, vec3 p2, vec3 p3, float param)
{
	SplineData result;
	
	vec3 a = p1 + param*(p2 - p1);
	vec3 b = p2 + param*(p3 - p2);

	result.pos = a + param*(b - a);
	result.tangent = (b - a) / length(b - a);
	result.a = a;
	return result;
}

void main()
{
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec2 a = mix(tcTexturePos[0], tcTexturePos[3], u);
	vec2 b = mix(tcTexturePos[1], tcTexturePos[2], u);
	teTexturePos = mix(a, b, v);

	SplineData leftSpline = calcSplinePos(tcPos[0], controlPoints[0], tcPos[1], v);
	SplineData rightSpline = calcSplinePos(tcPos[3], controlPoints[1], tcPos[2], v);

	vec3 splinePos = leftSpline.pos*(1.0 - u) + rightSpline.pos*u;
	vec3 bitangent = (rightSpline.pos - leftSpline.pos) / length(rightSpline.pos - leftSpline.pos*leftSpline.a);
	vec3 tangent = (leftSpline.tangent*(1.0 - u) + rightSpline.tangent*u) / length(leftSpline.tangent*(1.0 - u) + rightSpline.tangent*u);
	vec3 normal = cross(tangent, bitangent) / length(cross(tangent, bitangent));

	teCentrePos = tcCentrePos[0];
	teNormal = normal;
	teRandom = tcRandom[0]; // since they are all the same, any would work

	gl_Position = projectionTransform * viewTransform * objectTransform * vec4(splinePos, 1.0);
}