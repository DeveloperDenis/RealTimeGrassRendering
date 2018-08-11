#include "platform_layer.h"

#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "main.h"

// returns a random number in range [0.0, 1.0]
static f32 getRandom()
{
	return (f32)rand() / (f32)RAND_MAX;
}

static void drawGrassField(Matrix4f transform, u32 transformUniform, u32 patchPosUniform, u32 numElements, u32 type)
{
    v3f origin = transform.getTranslation();
	Matrix4f newTransform = transform;
	
	//TODO(denis): will need to not be hardcoded if want to support over 9 patches
	for (s32 row = -1; row <= 1; ++row)
	{
		for (s32 col = -1; col <= 1; ++col)
		{
			v2f patchPos = {-0.5f + col*1.0f, -0.5f + row*1.0f};
			
			newTransform.setTranslation(origin.x + col, origin.y, origin.z + row);
			glUniformMatrix4fv(transformUniform, 1, GL_TRUE,  (f32*)newTransform.elements);
			glUniform2fv(patchPosUniform, 1, (f32*)patchPos.e);

			if (type == GL_PATCHES)
				glDrawArrays(GL_PATCHES, 0, numElements);
			else if (type == GL_TRIANGLES)
				glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
		}
	}
}

//TODO(denis): implement density map
// returns the number of vertices generated
static u32 generateGrassPatch(v3f grassPlane[4], std::vector<GrassBlade>* blades)
{
	// these values were played around with until something that looked "right" was found
	// ideally, we would read these values from the density map (or at least the height)
	f32 minWidth = 0.0025f;
	f32 maxWidth = 0.0075f;

	f32 minHeight = 0.05f;
	f32 maxHeight = 0.125f;

	//TODO(denis): read from density map
	u32 numBladesToGenerate = NUM_BLADES_TO_GENERATE;
	for (u32 i = 0; i < numBladesToGenerate; ++i)
	{
		// these are passed to the GPU to give variety to grass blades
		f32 randomValues[8];
		for (u32 randIndex = 0; randIndex < 8; ++randIndex)
			randomValues[randIndex] = getRandom();
			
		f32 width = minWidth + getRandom()*(maxWidth - minWidth);
		f32 height = minHeight + getRandom()*(maxHeight - minHeight); //TODO(denis): multiply by density map value

		//TODO(denis): use a better random distribution, some kind of noise function?
		f32 x = getRandom() - 0.5f;
		f32 z = getRandom() - 0.5f;
		v4f bladeCentre = V4f(x, grassPlane[0].y, z, randomValues[0]);

		GrassBlade blade;

		// setting quad vertices
		blade[0] = bladeCentre - V4f(0.5f*width, 0.0f, 0.0f, 0.0f);
		blade[4] = bladeCentre - V4f(0.5f*width, -height, 0.0f, 0.0f);
		blade[8] = bladeCentre + V4f(0.5f*width, height, 0.0f, 0.0f);
		blade[12] = bladeCentre + V4f(0.5f*width, 0.0f, 0.0f, 0.0f);

		// setting centre positions for each vertex
		{
			v4f centreLower = V4f(bladeCentre.x, 0, bladeCentre.z, randomValues[1]);
			v4f centreUpper = centreLower;
			centreUpper.y = 1;
		    
			blade[1] = centreLower;
			blade[5] = centreUpper;
			blade[9] = centreUpper;
			blade[13] = centreLower;
		}

		// setting texture coords for each vertex
		blade[2] = V4f(0.0f, 1.0f, randomValues[2], randomValues[3]);
		blade[6] = V4f(1.0f, 1.0f, randomValues[2], randomValues[3]);
		blade[10] = V4f(1.0f, 0.0f, randomValues[2], randomValues[3]);
		blade[14] = V4f(0.0f, 0.0f, randomValues[2], randomValues[3]);

		// setting an extra vector filled with only random values for some randomness in the shaders
		v4f randomVector = V4f(randomValues[4], randomValues[5], randomValues[6], randomValues[7]);
		blade[3] = randomVector;
		blade[7] = randomVector;
		blade[11] = randomVector;
		blade[15] = randomVector;

	    blades->push_back(blade);		
	}

	return (u32)blades->size()*4;
}

static Matrix4f calculateProjectionMatrix(f32 near, f32 far, f32 fov, f32 aspectRatioX, f32 aspectRatioY)
{
	Matrix4f projectionMatrix = M4f();

	//set new w to -z to perform our perspective projection automatically through
	// the matrix multiplication
	projectionMatrix[3][2] = -1;
	projectionMatrix[3][3] = 0;

	//clipping plane stuff
	projectionMatrix[2][2] = -far / (far - near);
	projectionMatrix[2][3] = -((far*near)/(far - near));

	f32 fovScaleFactor = 1/tan(fov/2);
	projectionMatrix[0][0] = fovScaleFactor*aspectRatioX;
	projectionMatrix[1][1] = fovScaleFactor*aspectRatioY;

   return projectionMatrix;
}

static Matrix4f calculateViewMatrix(Camera* camera)
{
	Matrix4f viewMatrix = M4f();

	v3f zAxis = normalize(camera->pos - camera->target);
	v3f xAxis = normalize(cross(camera->upDir, zAxis));
	v3f yAxis = cross(zAxis, xAxis);

	Matrix4f orientation = M4f();
	orientation.setRow(0, xAxis);
	orientation.setRow(1, yAxis);
	orientation.setRow(2, zAxis);

	Matrix4f translation = M4f();
	translation.setTranslation(-camera->pos);

	viewMatrix = orientation*translation;
	return viewMatrix;
}

static Matrix4f getProjectionTransform(Camera* camera)
{
	Matrix4f result;
	
	s32 params[4];
	glGetIntegerv(GL_VIEWPORT, params);

	u32 screenWidth = params[2];
	u32 screenHeight = params[3];

	f32 aspectRatioX = 1.0f;
	f32 aspectRatioY = 1.0f;
	if (screenWidth > screenHeight)
		aspectRatioX = (f32)screenHeight / (f32)screenWidth;
	else
		aspectRatioY = (f32)screenWidth / (f32)screenHeight;

	result = calculateProjectionMatrix(camera->near, camera->far, camera->fov, aspectRatioX, aspectRatioY);
	return result;
}

static void updateShaderTransforms(Matrix4f projection, Matrix4f view, Matrix4f object, Camera* camera,
								   ShaderInfo* shaderInfo)
{
	Matrix4f worldTransform = projection*view*object;

	glUseProgram(shaderInfo->groundProgram);
	//TODO(denis): this is the exact same as grass, simplify!
	glUniformMatrix4fv(shaderInfo->groundObjectTransform, 1, GL_TRUE, (f32*)object.elements);
	glUniformMatrix4fv(shaderInfo->groundViewTransform, 1, GL_TRUE, (f32*)view.elements);
	glUniformMatrix4fv(shaderInfo->groundProjectionTransform, 1, GL_TRUE, (f32*)projection.elements);

	glUseProgram(shaderInfo->grassProgram);
	
	glUniformMatrix4fv(shaderInfo->grassObjectTransform, 1, GL_TRUE, (f32*)object.elements);
	glUniformMatrix4fv(shaderInfo->grassViewTransform, 1, GL_TRUE, (f32*)view.elements);
	glUniformMatrix4fv(shaderInfo->grassProjectionTransform, 1, GL_TRUE, (f32*)projection.elements);

	glUniform3fv(shaderInfo->cameraPos, 1, (f32*)&camera->pos);
}

static u32 createTexture(char* textureFile, u32 textureUnit)
{
	u32 textureID = 0;
	
	s32 width, height, numComponents;
	u8* textureData = stbi_load(textureFile, &width, &height, &numComponents, 4);
	ASSERT(textureData);
	
	glGenTextures(1, &textureID);
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);

	stbi_image_free(textureData);

	return textureID;
}

APP_INIT_CALL(appInit)
{
	ShaderInfo* shaderInfo = &memory->shaderInfo;
	shaderInfo->groundProgram = initShaders(platform, "../shaders/ground_vertex.glsl", "../shaders/ground_fragment.glsl", 0, 0);

	// the plane on which all of the grass blades are drawn
	v3f grassPlane[4];
    grassPlane[0] = V3f(-0.5f, 0.0f, -0.5f);
	grassPlane[1] = V3f(0.5f, 0.0f, -0.5f);
	grassPlane[2] = V3f(0.5f, 0.0f, 0.5f);
	grassPlane[3] = V3f(-0.5f, 0.0f, 0.5f);

	v3 planeTriangles[2];
	planeTriangles[0] = V3(2, 1, 0);
	planeTriangles[1] = V3(2, 0, 3);

	glGenVertexArrays(1, &memory->groundVAO);
	glBindVertexArray(memory->groundVAO);

	createVertexBuffer(grassPlane, 4);
	createElementBuffer(planeTriangles, 2);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3f), 0);
	glEnableVertexAttribArray(0);

	Camera* camera = &memory->camera;
	camera->fov= CAMERA_FOV;
	camera->pos = V3f(0.0f, 3.0f, 5.0f);
	camera->target = V3f(0.0f, 0.0f, 0.0f);
	camera->upDir = V3f(0.0f, 1.0f, 0.0f);
	camera->near = NEAR_PLANE;
	camera->far = FAR_PLANE;

	memory->viewTransform = calculateViewMatrix(camera);
	memory->projectionTransform = getProjectionTransform(camera);

	memory->objectTransform = M4f();

	//TODO(denis): these are now the exact same as the grass transforms, so there is probably a way to simplify this
	shaderInfo->groundObjectTransform = glGetUniformLocation(shaderInfo->groundProgram, "object");
	shaderInfo->groundViewTransform = glGetUniformLocation(shaderInfo->groundProgram, "view");
	shaderInfo->groundProjectionTransform = glGetUniformLocation(shaderInfo->groundProgram, "projection");

	glUniformMatrix4fv(shaderInfo->groundObjectTransform, 1, GL_TRUE, (f32*)memory->objectTransform.elements);
	glUniformMatrix4fv(shaderInfo->groundViewTransform, 1, GL_TRUE, (f32*)memory->viewTransform.elements);
	glUniformMatrix4fv(shaderInfo->groundProjectionTransform, 1, GL_TRUE, (f32*)memory->projectionTransform.elements);

	std::vector<GrassBlade> blades;
	memory->numBladeVertices = generateGrassPatch(grassPlane, &blades);
	
	shaderInfo->grassProgram = initShaders(platform, "../shaders/grass_vertex.glsl", "../shaders/grass_fragment.glsl",
										   "../shaders/grass_tess_control.glsl", "../shaders/grass_tess_eval.glsl");

	// each quad is a patch
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	
	glGenVertexArrays(1, &memory->grassVAO);
	glBindVertexArray(memory->grassVAO);

	u32 componentsPerBlade = sizeof(GrassBlade) / sizeof(v4f);
	createVertexBuffer((v4f*)(&blades[0]), (u32)blades.size() * componentsPerBlade);

	u32 vertexStride = sizeof(v4f)*4;
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertexStride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, vertexStride, (void*)sizeof(v4f));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, vertexStride, (void*)(sizeof(v4f)*2));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, vertexStride, (void*)(sizeof(v4f)*3));
	glEnableVertexAttribArray(3);

	shaderInfo->grassObjectTransform = glGetUniformLocation(shaderInfo->grassProgram, "objectTransform");
	shaderInfo->grassViewTransform = glGetUniformLocation(shaderInfo->grassProgram, "viewTransform");
	shaderInfo->grassProjectionTransform = glGetUniformLocation(shaderInfo->grassProgram, "projectionTransform");

	glUniformMatrix4fv(shaderInfo->grassObjectTransform, 1, GL_TRUE, (f32*)memory->objectTransform.elements);
	glUniformMatrix4fv(shaderInfo->grassViewTransform, 1, GL_TRUE, (f32*)memory->viewTransform.elements);
	glUniformMatrix4fv(shaderInfo->grassProjectionTransform, 1, GL_TRUE, (f32*)memory->projectionTransform.elements);

	shaderInfo->cameraPos = glGetUniformLocation(shaderInfo->grassProgram, "cameraPos");
	glUniform3fv(shaderInfo->cameraPos, 1, (f32*)&camera->pos);

	shaderInfo->time = glGetUniformLocation(shaderInfo->grassProgram, "time");
	shaderInfo->windActive = glGetUniformLocation(shaderInfo->grassProgram, "windActive");
	shaderInfo->patchPos = glGetUniformLocation(shaderInfo->grassProgram, "patchPos");

	u32 fieldOrigin = glGetUniformLocation(shaderInfo->grassProgram, "fieldRect");
	//TODO(denis): assumes the 9 square system is how the patches are organized
	v3f fieldRect[2] = {grassPlane[0] - V3f(1.0f, 0.0f, 1.0f),
						grassPlane[2] + V3f(1.0f, 0.0f, 1.0f)};
	glUniform3fv(fieldOrigin, 2, (f32*)&fieldRect[0]);

	glEnable(GL_DEPTH_TEST);

	memory->oldController = {};
	memory->lastMousePos = V2(-1, -1);
	memory->windActive = 0;

	// setting up textures
	memory->alphaTexture = createTexture("grass_alpha_texture.png", GL_TEXTURE0);
	u32 textureUniform = glGetUniformLocation(shaderInfo->grassProgram, "alphaTexture");
	glUniform1i(textureUniform, 0);

	memory->diffuseTexture = createTexture("grass_diffuse_texture.jpg", GL_TEXTURE1);
	textureUniform = glGetUniformLocation(shaderInfo->grassProgram, "diffuseTexture");
	glUniform1i(textureUniform, 1);

	memory->forceMap = createTexture(forceMapFile, GL_TEXTURE2);
	textureUniform = glGetUniformLocation(shaderInfo->grassProgram, "forceMap");
	glUniform1i(textureUniform, 2);
}

APP_UPDATE_CALL(appUpdate)
{
	static f32 time = 0.0f;
	static f32 cameraRotation = 0.0f;
	
	if (input->mouse.leftPressed)
	{
		f32 cameraDist = magnitude(memory->camera.pos - memory->camera.target);

		f32 panFactor = MAX_PAN_SPEED*(cameraDist/MAX_ZOOM);
		v2 diff = memory->lastMousePos - input->mouse.pos;

		f32 xPanAmount = -diff.x*panFactor;
		f32 yPanAmount = -diff.y*panFactor;

		v3f cameraDir = normalize(memory->camera.pos - memory->camera.target);
		v3f cameraLeft = cross(V3f(0.0f, 1.0f, 0.0f), cameraDir);
		v3f cameraBack = cross(cameraLeft, V3f(0.0f, -1.0f, 0.0f));

		v3f translateX = cameraLeft*xPanAmount;
		v3f translateY = cameraBack*(-yPanAmount);
		
		memory->objectTransform.translate(translateX);
		memory->objectTransform.translate(translateY);
	}

	if (input->mouse.rightPressed)
	{
	    s32 rotateDiff = input->mouse.pos.x - memory->lastMousePos.x;

		Matrix4f rotationMatrix = getYRotationMatrix(-rotateDiff*0.005f);
	    memory->camera.pos = rotationMatrix*memory->camera.pos;

		cameraRotation += rotateDiff*0.005f;
		if (cameraRotation > 2.0f*M_PI)
			cameraRotation /= 2.0f*(f32)M_PI;
	   	
		s32 yDiff = memory->lastMousePos.y - input->mouse.pos.y;
		f32 cameraDist = magnitude(memory->camera.pos - memory->camera.target);

		f32 zoomRatio = 0.01f;
		if (cameraDist < MAX_ZOOM/5.0f)
			zoomRatio = 0.006f;
		
		f32 zoomAmount = yDiff*zoomRatio;

		cameraDist -= zoomAmount;

		if (cameraDist < MIN_ZOOM)
			cameraDist = MIN_ZOOM;
		else if (cameraDist > MAX_ZOOM)
			cameraDist = MAX_ZOOM;

		v3f cameraDir = normalize(memory->camera.pos);
		memory->camera.pos = cameraDir*cameraDist;

		memory->viewTransform = calculateViewMatrix(&memory->camera);
	}

	if (memory->oldController.actionPressed && !input->controller.actionPressed)
	{
		memory->windActive = (memory->windActive + 1) % 2;
		glUniform1i(memory->shaderInfo.windActive, memory->windActive);
		time = 0.0f;
	}

	//TODO(denis): these cause weird behaviour with the zooming function
	if (input->controller.upPressed && memory->camera.pos.y < MAX_CAMERA_HEIGHT)
	{
		memory->camera.pos.y += CAMERA_HEIGHT_MOVEMENT;

		if (memory->camera.pos.y > MAX_CAMERA_HEIGHT)
			memory->camera.pos.y = MAX_CAMERA_HEIGHT;
		
		memory->viewTransform = calculateViewMatrix(&memory->camera);
	}
	else if (input->controller.downPressed && memory->camera.pos.y > MIN_CAMERA_HEIGHT)
	{
		memory->camera.pos.y -= CAMERA_HEIGHT_MOVEMENT;

		if (memory->camera.pos.y < MIN_CAMERA_HEIGHT)
			memory->camera.pos.y = MIN_CAMERA_HEIGHT;
		
		memory->viewTransform = calculateViewMatrix(&memory->camera);
	}

	// these have to be done every frame because we never know when the user will resize the window
	memory->projectionTransform = getProjectionTransform(&memory->camera);
	updateShaderTransforms(memory->projectionTransform, memory->viewTransform, memory->objectTransform,
						   &memory->camera, &memory->shaderInfo);

	glUseProgram(memory->shaderInfo.groundProgram);
	glBindVertexArray(memory->groundVAO);
	drawGrassField(memory->objectTransform, memory->shaderInfo.groundObjectTransform, memory->shaderInfo.patchPos,
				   6, GL_TRIANGLES);

	glUseProgram(memory->shaderInfo.grassProgram);

	time += 0.03f;
	glUniform1f(memory->shaderInfo.time, time);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, memory->alphaTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, memory->diffuseTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, memory->forceMap);

	glBindVertexArray(memory->grassVAO);

	drawGrassField(memory->objectTransform, memory->shaderInfo.grassObjectTransform, memory->shaderInfo.patchPos,
				   memory->numBladeVertices, GL_PATCHES);
	
	memory->oldController = input->controller;
	memory->lastMousePos = input->mouse.pos;
}
