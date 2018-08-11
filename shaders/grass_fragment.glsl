#version 400 core

in vec3 teCentrePos;
in vec3 teNormal;
in vec2 teTexturePos;
in vec3 teRandom;

out vec4 colour;

uniform sampler2D alphaTexture;
uniform sampler2D diffuseTexture;
uniform vec3 cameraPos;

void main()
{
	vec4 grassColour = texture(diffuseTexture, teTexturePos);
	// modifying the diffuse colour slightly for more variance
	grassColour.r += grassColour.r*(teRandom.x*0.5 - 0.25);
	grassColour.g += grassColour.g*(teRandom.y*0.5 - 0.25);
	grassColour.b += grassColour.b*(teRandom.z*0.5 - 0.25);

	float cameraDist = length(cameraPos - teCentrePos);
	
	float distanceFalloff = 1/cameraDist;
	distanceFalloff = min(distanceFalloff, 1.0);

	//TODO(denis): temporarily disables distance falloff until I think of a better way to do it
	distanceFalloff = 1.0f;

	vec4 ambient = 0.5*grassColour*distanceFalloff;

	vec3 lightPos = vec3(0.0, 0.0, 5.0);
	vec3 lightDir = normalize(teCentrePos - lightPos);
	float intensity = dot(lightDir, teNormal);

	// We illuminate the blade whether the front or back is facing the light
	if (intensity < 0.0)
	   intensity = -intensity;

	vec4 textureColour = texture(alphaTexture, teTexturePos);
	if (textureColour == vec4(0.0, 0.0, 0.0, 1.0))
	   discard;

	// multiplied by texture coordinate so that lower areas of the blade are darker
	//(texturePos.x because the texture is horizontal)
	colour = ambient + grassColour*intensity*teTexturePos.x;
}