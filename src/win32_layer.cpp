#define NOMINMAX
#define Rectangle _Rectangle
#include <windows.h>
#undef Rectangle
#undef near
#undef far

#include "Strsafe.h"
#include "Windowsx.h"
#include <gl/gl.h>

#include "denis_types.h"
#include "denis_math.h"
#include "denis_strings.h"
#include "denis_opengl.h"

#define PLATFORM_IMPLEMENTATION
#include "platform_layer.h"

//NOTE(denis): Windows specific OpenGL stuff
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

//NOTE(denis): this function is win32 specific
typedef HGLRC(*GL_CREATE_CONTEXT_PTR)(HDC, HGLRC, int*);

#define INIT_GL_FUNCTION(type, name) name = (type)win32_loadGLFunction(#name);

#define DEFAULT_WINDOW_WIDTH 640
#define DEFAULT_WINDOW_HEIGHT 480

struct Memory;

extern APP_UPDATE_CALL(appUpdate);
extern APP_INIT_CALL(appInit);

static bool _running = true;
static HDC _deviceContext;
static u32 _windowWidth;
static u32 _windowHeight;
static u32 _currentTouchPoint;

static Input _input;

static Platform _platform;

//NOTE: the .ray file name must start with a letter
bool getImageFromCmdLine(char* commandLine, char** imageFileName)
{
	bool success = false;

	bool startOfName = false;
	bool extensionReached = false;
	for (u32 i = 0; commandLine[i] != 0; ++i)
	{
		if (startOfName)
		{
			if (commandLine[i] == ' ' && !extensionReached)
			{
				startOfName = false;
				*imageFileName = 0;
			}
			else if (commandLine[i] == '.')
			{
				extensionReached = true;
				if (commandLine[i+1] == 0 || commandLine[i+2] == 0 || commandLine[i+3] == 0)
				{
					extensionReached = false;
					startOfName = false;
					*imageFileName = 0;
					continue;
				}

				//NOTE(denis): doesn't support file extensions longer than 4 letters
				if (IS_LETTER(commandLine[i+1]) && IS_LETTER(commandLine[i+2]) && IS_LETTER(commandLine[i+3]))
				{
					if (IS_LETTER(commandLine[i+4]) && !IS_LETTER(commandLine[i+5]))
					{
						(*imageFileName)[i+5] = 0;
					}
					else
					{
						(*imageFileName)[i+4] = 0;
					}

					success = true;
					break;
				}
			}
		}
		else if (IS_LETTER(commandLine[i]))
		{
			startOfName = true;
			*imageFileName = &commandLine[i];
		}
	}

	return success;
}

static void* win32_readFile(char* fileName)
{
	void* data = 0;
	
	HANDLE file = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (file == INVALID_HANDLE_VALUE)
		//TODO(denis): error message
		return 0;

	LARGE_INTEGER fileSize = {};
	if (GetFileSizeEx(file, &fileSize))
	{
		DWORD bytesRead = 0;
		data = HEAP_ALLOC(fileSize.QuadPart);
		if (data)
		{
			//TODO(denis): using LowPart assumes that our files won't be larger than 2^32 bits
			if (!ReadFile(file, data, fileSize.LowPart, &bytesRead, 0))
			{
				//TODO(denis): error message
			}
		}
		else
		{
			//TODO(denis): error message
		}
	}
	else
	{
		//TODO(denis): error message
	}

	CloseHandle(file);

	return data;
}

static void* win32_loadGLFunction(char* functionName)
{
	void* function = 0;
	function = wglGetProcAddress(functionName);

	// TODO(denis): wglGetProcAddress will fail if the function happens to be one of the functions included in the
	//version of OpenGL shipped with Windows
	
	if (!function)
	{
		char* errorString = concatStrings(functionName, " could not be loaded.\n");
		OutputDebugStringA(errorString);
		HEAP_FREE(errorString);
		//TODO(denis): exiting the entire program might be a bit harsh?
		exit(1);
	}
	
	return function;
}

static void win32_initOpenGL()
{
	// create a dummy context to use for loading modern OpenGL
	PIXELFORMATDESCRIPTOR pixelFormat = {};
	pixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelFormat.nVersion = 1;
	pixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormat.iPixelType = PFD_TYPE_RGBA;
	pixelFormat.cColorBits = 32;
	pixelFormat.cAlphaBits = 8;
	pixelFormat.iLayerType = PFD_MAIN_PLANE;

	u32 pixelFormatIndex = ChoosePixelFormat(_deviceContext, &pixelFormat);
	if (!pixelFormatIndex)
	{
		OutputDebugStringA("Could not get requested pixel format!\n");
		exit(1);
	}

	PIXELFORMATDESCRIPTOR devicePixelFormat;
	DescribePixelFormat(_deviceContext, pixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &devicePixelFormat);
	
	if (!SetPixelFormat(_deviceContext, pixelFormatIndex, &devicePixelFormat))
	{
		OutputDebugStringA("Error setting the pixel format\n");
		exit(1);
	}

	HGLRC glContext = wglCreateContext(_deviceContext);
	if (!wglMakeCurrent(_deviceContext, glContext))
	{
		OutputDebugStringA("Error setting current OpenGL context\n");
		exit(1);
	}

	// now we load the extensions required to create a modern OpenGL context
	GL_CREATE_CONTEXT_PTR wglCreateContextAttribsARB =
		(GL_CREATE_CONTEXT_PTR)win32_loadGLFunction("wglCreateContextAttribsARB");;

	int contextAttribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};
	HGLRC modernGLContext = wglCreateContextAttribsARB(_deviceContext, 0, contextAttribs);
	if (!modernGLContext)
	{
		OutputDebugStringA("Error creating modern OpenGL context\n");
		exit(1);
	}

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(glContext);
	if (!wglMakeCurrent(_deviceContext, modernGLContext))
	{
		OutputDebugStringA("Error making OpenGL 3.2 context current\n");
		exit(1);
	}
}

LRESULT CALLBACK win32_messageCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	
	switch(message)
	{
		case WM_DESTROY:
		{
			_running = false;
		} break;

		case WM_QUIT:
		{
			_running = false;
		} break;

		case WM_SIZE:
		{
			RECT clientRect;
			GetClientRect(windowHandle, &clientRect);
			_windowWidth = clientRect.right - clientRect.left;
			_windowHeight = clientRect.bottom - clientRect.top;

			glViewport(0, 0, _windowWidth, _windowHeight);
		} break;
		
		case WM_PAINT:
		{
			SwapBuffers(_deviceContext);

			RECT windowRect = { 0, 0, (LONG)_windowWidth, (LONG)_windowHeight };
			ValidateRect(windowHandle, &windowRect);
		} break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			if (wParam == VK_UP)
			{
				_input.controller.upPressed = true;
			}
			else if (wParam == VK_DOWN)
			{
				_input.controller.downPressed = true;
			}
			else if (wParam == VK_LEFT)
			{
				_input.controller.leftPressed = true;
			}
			else if (wParam == VK_RIGHT)
			{
				_input.controller.rightPressed = true;
			}

			if (wParam == VK_SPACE)
			{
				_input.controller.actionPressed = true;
			}
		} break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			if (wParam == VK_UP)
			{
				_input.controller.upPressed = false;
			}
			else if (wParam == VK_DOWN)
			{
				_input.controller.downPressed = false;
			}
			else if (wParam == VK_LEFT)
			{
				_input.controller.leftPressed = false;
			}
			else if (wParam == VK_RIGHT)
			{
				_input.controller.rightPressed = false;
			}

			if (wParam == VK_SPACE)
			{
				_input.controller.actionPressed = false;
			}
		} break;

		case WM_MOUSEMOVE:
		{
			v2 mousePos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			_input.mouse.pos = mousePos;
		} break;

		case WM_LBUTTONDOWN:
		{
			v2 mousePos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			_input.mouse.pos = mousePos;
			_input.mouse.leftClickStartPos = mousePos;

			_input.mouse.leftWasPressed = false;
			_input.mouse.leftPressed = true;
		} break;

		case WM_LBUTTONUP:
		{
			v2 mousePos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			_input.mouse.pos = mousePos;

			_input.mouse.leftWasPressed = true;
			_input.mouse.leftPressed = false;
		} break;

		case WM_RBUTTONDOWN:
		{
			v2 mousePos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			_input.mouse.pos = mousePos;
			_input.mouse.rightClickStartPos = mousePos;

			_input.mouse.rightWasPressed = false;
			_input.mouse.rightPressed = true;
		} break;

		case WM_RBUTTONUP:
		{
			v2 mousePos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			_input.mouse.pos = mousePos;

			_input.mouse.rightWasPressed = true;
			_input.mouse.rightPressed = false;
		} break;
		
		//NOTE(denis): used for touch and pen input
		
		case WM_POINTERUPDATE:
		{
			u32 pointerID = GET_POINTERID_WPARAM(wParam);
			POINTER_INPUT_TYPE inputType;
			
			if (GetPointerType(pointerID, &inputType))
			{
				switch (inputType)
				{
					case PT_TOUCH:
					{
						//TODO(denis): for now we only process up to five points
						if (_currentTouchPoint >= 4)
							break;
						
						POINTER_TOUCH_INFO touchInfo = {};
						GetPointerTouchInfo(pointerID, &touchInfo);

						RECT touchRect = touchInfo.rcContactRaw;
						POINT touchPoint;
						if (touchInfo.touchMask & TOUCH_MASK_CONTACTAREA)
						{
							touchPoint.x =
								touchRect.left + (touchRect.right - touchRect.left)/2;
							touchPoint.y =
								touchRect.top + (touchRect.bottom - touchRect.top)/2;
						}
						else
						{
							touchPoint.x = touchRect.left;
							touchPoint.y = touchRect.top;
						}

						ScreenToClient(windowHandle, &touchPoint);
						_input.touch.points[_currentTouchPoint].x = touchPoint.x;
						_input.touch.points[_currentTouchPoint].y = touchPoint.y;
						
						++_currentTouchPoint;
						_input.touch.numActivePoints = _currentTouchPoint;
					} break;

					case PT_PEN:
					{
						POINTER_PEN_INFO penInfo = {};
						GetPointerPenInfo(pointerID, &penInfo);

						POINT penPos = penInfo.pointerInfo.ptPixelLocationRaw;
						ScreenToClient(windowHandle, &penPos);

						_input.pen.x = penPos.x;
						_input.pen.y = penPos.y;

						_input.pen.usingEraser = penInfo.penFlags & PEN_FLAG_ERASER;
						
						if (penInfo.penMask & PEN_MASK_PRESSURE)
						{
							_input.pen.pressure = penInfo.pressure;
						}
						else
						{
							_input.pen.pressure = 0;
						}
					} break;
				}
			}
		} break;

		case WM_POINTERDOWN:
		{
			u32 pointerID = GET_POINTERID_WPARAM(wParam);
			POINTER_INPUT_TYPE inputType;

			if (GetPointerType(pointerID, &inputType))
			{
				switch (inputType)
				{
					case PT_TOUCH:
					{
						
					} break;

					case PT_PEN:
					{
						
					} break;
				}	
			}
		} break;

		case WM_POINTERUP:
		{
			//TODO(denis): do something?
		} break;
		
		default:
		{
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		} break;
	}

	return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
	_windowWidth = DEFAULT_WINDOW_WIDTH;
	_windowHeight = DEFAULT_WINDOW_HEIGHT;
	
	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = win32_messageCallback;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);
	windowClass.lpszClassName = "win32WindowClass";

	char* forceMapFile = 0;
	if (cmdLine[0] != 0)
	{
	    getImageFromCmdLine(cmdLine, &forceMapFile);
	}

	if (!forceMapFile)
		forceMapFile = "default_force_map.png";
	
	if (!RegisterClassEx(&windowClass))
	{
		OutputDebugString("Error creating window class\n");
		return 1;
	}

	DWORD windowStyles = WS_OVERLAPPEDWINDOW|WS_VISIBLE;

	RECT windowRect = {0, 0, (LONG)_windowWidth, (LONG)_windowHeight};
	AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, FALSE, 0);
	
	HWND windowHandle =
		CreateWindowEx(0, windowClass.lpszClassName, "Real-time Grass Rendering",
					   windowStyles,
					   CW_USEDEFAULT, CW_USEDEFAULT,
					   windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
					   0, 0, instance, 0);
	
	if (!windowHandle)
	{
		OutputDebugString("Error creating window\n");
		return 1;
	}

	//NOTE(denis): don't need to release this because this is our window's private DC
	_deviceContext = GetDC(windowHandle);

	win32_initOpenGL();

	INIT_GL_FUNCTION(GL_GEN_BUFFERS_PTR, glGenBuffers);
	INIT_GL_FUNCTION(GL_BIND_BUFFER_PTR, glBindBuffer);
	INIT_GL_FUNCTION(GL_BUFFER_DATA_PTR, glBufferData);
	INIT_GL_FUNCTION(GL_CREATE_SHADER_PTR, glCreateShader);
	INIT_GL_FUNCTION(GL_SHADER_SOURCE_PTR, glShaderSource);
	INIT_GL_FUNCTION(GL_COMPILE_SHADER_PTR, glCompileShader);
	INIT_GL_FUNCTION(GL_GET_SHADER_IV_PTR, glGetShaderiv);
	INIT_GL_FUNCTION(GL_GET_SHADER_INFO_LOG_PTR, glGetShaderInfoLog);
	INIT_GL_FUNCTION(GL_DELETE_SHADER_PTR, glDeleteShader);
	INIT_GL_FUNCTION(GL_CREATE_PROGRAM_PTR, glCreateProgram);
	INIT_GL_FUNCTION(GL_ATTACH_SHADER_PTR, glAttachShader);
	INIT_GL_FUNCTION(GL_LINK_PROGRAM_PTR, glLinkProgram);
	INIT_GL_FUNCTION(GL_USE_PROGRAM_PTR, glUseProgram);
	INIT_GL_FUNCTION(GL_VERTEX_ATTRIB_POINTER_PTR, glVertexAttribPointer);
	INIT_GL_FUNCTION(GL_ENABLE_VERTEX_ATTRIB_ARRAY_PTR, glEnableVertexAttribArray);
	INIT_GL_FUNCTION(GL_GEN_VERTEX_ARRAYS_PTR, glGenVertexArrays);
	INIT_GL_FUNCTION(GL_BIND_VERTEX_ARRAY_PTR, glBindVertexArray);
	INIT_GL_FUNCTION(GL_BUFFER_SUB_DATA_PTR, glBufferSubData);
	INIT_GL_FUNCTION(GL_GET_UNIFORM_LOCATION_PTR, glGetUniformLocation);
	INIT_GL_FUNCTION(GL_UNIFORM_1F_PTR, glUniform1f);
	INIT_GL_FUNCTION(GL_UNIFORM_1I_PTR, glUniform1i);
	INIT_GL_FUNCTION(GL_UNIFORM_2IV_PTR, glUniform2iv);
	INIT_GL_FUNCTION(GL_UNIFORM_2FV_PTR, glUniform2fv);
	INIT_GL_FUNCTION(GL_UNIFORM_3FV_PTR, glUniform3fv);
	INIT_GL_FUNCTION(GL_UNIFORM_MATRIX4FV_PTR, glUniformMatrix4fv);
	INIT_GL_FUNCTION(GL_PATCH_PARAMETERI_PTR, glPatchParameteri);
	INIT_GL_FUNCTION(GL_ACTIVE_TEXTURE_PTR, glActiveTexture);
	
	glViewport(0, 0, _windowWidth, _windowHeight);
	
	//TODO(denis): should probably let the user set the size of this
	void* mainMemory = VirtualAlloc(0, MEGABYTE(256), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
	
	LARGE_INTEGER countFrequency;
	QueryPerformanceFrequency(&countFrequency); //NOTE(denis): counts/second
	
	LARGE_INTEGER lastCounts;
	QueryPerformanceCounter(&lastCounts);

	Mouse oldMouse = {};
	_input.mouse.leftClickStartPos = V2(-1, -1);
	_input.mouse.rightClickStartPos = V2(-1, -1);

	_platform.readFile = win32_readFile;
	
	appInit(_platform, (Memory*)mainMemory, forceMapFile);

	while (_running)
	{
		MSG message;
		while (PeekMessage(&message, windowHandle, 0, 0, PM_REMOVE))
		{
			// NOTE(denis): this is here because sometimes messages aren't dispatched properly for some reason
			if (message.message == WM_QUIT)
				_running = false;
				
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		glClearColor(0.4f, 0.5f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		appUpdate(_platform, (Memory*)mainMemory, &_input);

		SwapBuffers(_deviceContext);

		LARGE_INTEGER currentCounts;
		QueryPerformanceCounter(&currentCounts);
		u64 timePassed = currentCounts.QuadPart - lastCounts.QuadPart;
		f64 timeMs = (f64)timePassed * 1000.0 / (f64)countFrequency.QuadPart;

		//TODO(denis): probably don't do a busy loop
		//NOTE(denis): the epsilon is an attempt to lessen the effects of random spikes
		f32 epsilon = 0.01f;
		while (timeMs < (f64)1/(f64)60 * 1000.0 - epsilon)
		{
			QueryPerformanceCounter(&currentCounts);
			timePassed = currentCounts.QuadPart - lastCounts.QuadPart;
			timeMs = (f64)timePassed * 1000.0 / (f64)countFrequency.QuadPart;
		}
#if 0
		char timeBuffer[100];
		StringCbPrintf(timeBuffer, 100, "%f\n", timeMs);
		OutputDebugString(timeBuffer);
#endif
		lastCounts = currentCounts;

		_currentTouchPoint = 0;
		_input.touch = {};

		//TODO(denis): this only gives programs one frame to handle mouse clicks
		// is that enough? It seems like it should be fine, but maybe it would be safer with
		// more than one frame? But then users would need to add tests to avoid multiple clicks
		// registering in a program
		bool disableLeftWasPressed = _input.mouse.leftWasPressed && oldMouse.leftPressed;
		bool disableRightWasPressed = _input.mouse.rightWasPressed && oldMouse.rightPressed;

		oldMouse = _input.mouse;
		if (disableLeftWasPressed)
		{
			_input.mouse.leftWasPressed = false;
			_input.mouse.leftClickStartPos = V2(-1, -1);
		}
		if (disableRightWasPressed)
		{
			_input.mouse.rightWasPressed = false;
			_input.mouse.rightClickStartPos = V2(-1, -1);
		}
	}
	
	DestroyWindow(windowHandle);
	
	return 0;
}
