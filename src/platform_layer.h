#if !defined(PLATFORM_LAYER_H_)
#define PLATFORM_LAYER_H_

#include "denis_types.h"
#include "denis_math.h"

//TODO(denis): this doesn't take into account that the pitch may be different from the width of
// the image
//TODO(denis): maybe this should be defined in denis_drawing.h?
struct Bitmap
{
	u32* pixels;

	u32 width;
	u32 height;
};

//TODO(denis): I frequently want to know the status of the previous frame, so that should be in this struct
struct Controller
{
	bool upPressed;
	bool downPressed;
	bool leftPressed;
	bool rightPressed;

	bool actionPressed;
};

struct Mouse
{
	v2 pos;
	
	bool leftPressed;
	bool leftWasPressed;
	v2 leftClickStartPos;

	bool rightPressed;
	bool rightWasPressed;
	v2 rightClickStartPos;
};

//TODO(denis): currently only supported in Windows, need to implement in Linux
struct Touch
{
	//TODO(denis): eventually make this hold 10 different points?
	u32 numActivePoints;
	v2 points[5];
};

//TODO(denis): currently only supported in Windows, need to implement in Linux
struct Pen
{
	u32 pressure;
	u32 x;
	u32 y;
	bool usingEraser;
};

struct Input
{
	Pen pen;
	Touch touch;
	Mouse mouse;
	Controller controller;
};

struct Platform
{
	void*(*readFile)(char* fileName);
};

#define APP_INIT_CALL(name) void (name)(Platform platform, Memory* memory, char* forceMapFile)
#define APP_UPDATE_CALL(name) void (name)(Platform platform, Memory* memory, Input* input)

#if defined(DENIS_WIN32) && !defined(PLATFORM_IMPLEMENTATION)
#include "win32_layer.cpp"

#elif defined(DENIS_LINUX) && !defined(PLATFORM_IMPLEMENTATION)

#include "linux_layer.cpp"

#endif

#endif
