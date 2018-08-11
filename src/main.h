#if !defined(MAIN_H_)
#define MAIN_H_

#define NUM_BLADES_TO_GENERATE 7500

#define DEG_TO_RAD(value) ((value)*(f32)M_PI/180.0f)
#define CAMERA_FOV DEG_TO_RAD(15)

#define MAX_PAN_SPEED 0.005f

#define MIN_ZOOM 0.5f
#define MAX_ZOOM 15.0f

#define MIN_CAMERA_HEIGHT 0.5f
#define MAX_CAMERA_HEIGHT 6.5f
#define CAMERA_HEIGHT_MOVEMENT 0.025f

#define NEAR_PLANE 0.5f
#define FAR_PLANE 30.0f


typedef struct {
	v4f v[16];
	v4f& operator[](u32 index) { return v[index]; }
} GrassBlade;

struct ShaderInfo
{
	u32 groundProgram;
	u32 grassProgram;
	
	u32 groundObjectTransform;
	u32 groundViewTransform;
	u32 groundProjectionTransform;
	
	u32 grassObjectTransform;
	u32 grassViewTransform;
	u32 grassProjectionTransform;

	u32 cameraPos;
	u32 time;
	u32 windActive;
	u32 patchPos;
};

struct Camera
{
	f32 near;
	f32 far;
	f32 fov;

	v3f pos;
	v3f target;
	v3f upDir;
};

struct Memory
{
	ShaderInfo shaderInfo;
	
	u32 groundVAO;
	u32 grassVAO;

	u32 alphaTexture;
	u32 diffuseTexture;
	u32 forceMap;
	
	u32 numBladeVertices;

	Camera camera;
	
	Matrix4f viewTransform;
	Matrix4f projectionTransform;
	Matrix4f objectTransform;

    u8 windActive;
	
	v2 lastMousePos;
	//TODO(denis): this should be part of the controller struct instead
	Controller oldController;
};

#endif
