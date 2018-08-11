#if !defined(DENIS_OPENGL_H_)
#define DENIS_OPENGL_H_

// defines taken from glext.h
#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
#define GL_PROGRAM_POINT_SIZE             0x8642
#define GL_ARRAY_BUFFER					  0x8892
#define GL_ELEMENT_ARRAY_BUFFER			  0x8893
#define GL_STATIC_DRAW					  0x88E4
#define GL_FRAGMENT_SHADER				  0x8B30
#define GL_VERTEX_SHADER				  0x8B31
#define GL_TESS_EVALUATION_SHADER         0x8E87
#define GL_TESS_CONTROL_SHADER            0x8E88
#define GL_COMPILE_STATUS				  0x8B81
#define GL_PATCHES                        0x000E
#define GL_PATCH_VERTICES                 0x8E72
#define GL_PATCH_DEFAULT_INNER_LEVEL      0x8E73
#define GL_PATCH_DEFAULT_OUTER_LEVEL      0x8E74
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_CLAMP_TO_EDGE                  0x812F

//NOTE(denis): functions used that are already part of Windows:
// - glDrawArrays
// - glDrawElements
// - glPolygonMode

typedef void(*GL_GEN_BUFFERS_PTR)(u32, u32*);
typedef void(*GL_BIND_BUFFER_PTR)(GLenum, u32);
typedef void(*GL_BUFFER_DATA_PTR)(GLenum, u32, void*, GLenum);
typedef u32(*GL_CREATE_SHADER_PTR)(GLenum);
typedef void(*GL_SHADER_SOURCE_PTR)(u32, u32, const char**, const s32**);
typedef void(*GL_COMPILE_SHADER_PTR)(u32);
typedef void(*GL_GET_SHADER_IV_PTR)(u32, GLenum, s32*);
typedef void(*GL_GET_SHADER_INFO_LOG_PTR)(u32, u32, u32*, char*);
typedef void(*GL_DELETE_SHADER_PTR)(u32);
typedef u32(*GL_CREATE_PROGRAM_PTR)(void);
typedef void(*GL_ATTACH_SHADER_PTR)(u32, u32);
typedef void(*GL_LINK_PROGRAM_PTR)(u32);
typedef void(*GL_USE_PROGRAM_PTR)(u32);
typedef void(*GL_VERTEX_ATTRIB_POINTER_PTR)(u32, s32, GLenum, GLboolean, u32, const void*);
typedef void(*GL_ENABLE_VERTEX_ATTRIB_ARRAY_PTR)(u32);
typedef void(*GL_GEN_VERTEX_ARRAYS_PTR)(u32, u32*);
typedef void(*GL_BIND_VERTEX_ARRAY_PTR)(u32);
typedef void(*GL_POLYGON_MODE_PTR)(GLenum, GLenum);
typedef void(*GL_BUFFER_SUB_DATA_PTR)(GLenum, s32*, u32*, const void*);
typedef s32(*GL_GET_UNIFORM_LOCATION_PTR)(u32, const char*);
typedef void(*GL_UNIFORM_1F_PTR)(s32, f32);
typedef void(*GL_UNIFORM_1I_PTR)(s32, s32);
typedef void(*GL_UNIFORM_2IV_PTR)(s32, s32, s32*);
typedef void(*GL_UNIFORM_2FV_PTR)(s32, s32, f32*);
typedef void(*GL_UNIFORM_3FV_PTR)(s32, s32, f32*);
typedef void(*GL_UNIFORM_MATRIX4FV_PTR)(s32, u32, GLboolean, const f32*);
typedef void(*GL_PATCH_PARAMETERI_PTR)(GLenum, s32);
typedef void(*GL_ACTIVE_TEXTURE_PTR)(GLenum);

GL_GEN_BUFFERS_PTR glGenBuffers = 0;
GL_BIND_BUFFER_PTR glBindBuffer = 0;
GL_BUFFER_DATA_PTR glBufferData = 0;
GL_CREATE_SHADER_PTR glCreateShader = 0;
GL_SHADER_SOURCE_PTR glShaderSource = 0;
GL_COMPILE_SHADER_PTR glCompileShader = 0;
GL_GET_SHADER_IV_PTR glGetShaderiv = 0;
GL_GET_SHADER_INFO_LOG_PTR glGetShaderInfoLog = 0;
GL_DELETE_SHADER_PTR glDeleteShader = 0;
GL_CREATE_PROGRAM_PTR glCreateProgram = 0;
GL_ATTACH_SHADER_PTR glAttachShader = 0;
GL_LINK_PROGRAM_PTR glLinkProgram = 0;
GL_USE_PROGRAM_PTR glUseProgram = 0;
GL_VERTEX_ATTRIB_POINTER_PTR glVertexAttribPointer = 0;
GL_ENABLE_VERTEX_ATTRIB_ARRAY_PTR glEnableVertexAttribArray = 0;
GL_GEN_VERTEX_ARRAYS_PTR glGenVertexArrays = 0;
GL_BIND_VERTEX_ARRAY_PTR glBindVertexArray = 0;
GL_BUFFER_SUB_DATA_PTR glBufferSubData = 0;
GL_GET_UNIFORM_LOCATION_PTR glGetUniformLocation = 0;
GL_UNIFORM_1F_PTR glUniform1f = 0;
GL_UNIFORM_1I_PTR glUniform1i = 0;
GL_UNIFORM_2IV_PTR glUniform2iv = 0;
GL_UNIFORM_2FV_PTR glUniform2fv = 0;
GL_UNIFORM_3FV_PTR glUniform3fv = 0;
GL_UNIFORM_MATRIX4FV_PTR glUniformMatrix4fv = 0;
GL_PATCH_PARAMETERI_PTR glPatchParameteri = 0;
GL_ACTIVE_TEXTURE_PTR glActiveTexture = 0;

static u32 createVertexBuffer(void* vertices, u32 numVertices, u32 vertexSize)
{
	u32 bufferResult = 0;

	glGenBuffers(1, &bufferResult);
	glBindBuffer(GL_ARRAY_BUFFER, bufferResult);

	glBufferData(GL_ARRAY_BUFFER, vertexSize*numVertices, vertices, GL_STATIC_DRAW);

	return bufferResult;	
}
static inline u32 createVertexBuffer(v3f* vertices, u32 numVertices)
{
	return createVertexBuffer(vertices, numVertices, sizeof(v3f));
}
static inline u32 createVertexBuffer(v4f* vertices, u32 numVertices)
{
	return createVertexBuffer(vertices, numVertices, sizeof(v4f));
}

static u32 createElementBuffer(u32* indices, u32 numIndices)
{
	u32 bufferResult = 0;

	glGenBuffers(1, &bufferResult);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferResult);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices*sizeof(u32), indices, GL_STATIC_DRAW);

	return bufferResult;
}
static u32 createElementBuffer(v3* faces, u32 numFaces)
{
	u32 bufferResult = 0;

	glGenBuffers(1, &bufferResult);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferResult);

	//NOTE(denis): relies on the fact that v3 is the same size as three u32 tightly packed
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(v3)*numFaces, faces, GL_STATIC_DRAW);

	return bufferResult;
}

static u32 compileShader(void* shaderData, GLenum shaderType)
{
	u32 shaderObject = 0;
	
	const char* shaderString = (char*)shaderData;
	shaderObject = glCreateShader(shaderType);
	glShaderSource(shaderObject, 1, &shaderString, 0);
	glCompileShader(shaderObject);

	s32 success;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512] = {};
		glGetShaderInfoLog(shaderObject, 512, NULL, infoLog);

		ASSERT(!infoLog);
		
		glDeleteShader(shaderObject);
		shaderObject = 0;
	}

	return shaderObject;
}

// returns the shader program associated with the two shaders given
static u32 initShaders(Platform platform, char* vertexFile, char* fragmentFile, char* tcsFile, char* tesFile)
{
	u32 shaderProgram = glCreateProgram();

	u32 vertexShader = 0;
	if (vertexFile)
	{
		void* vertexShaderData = platform.readFile(vertexFile);
	    vertexShader = compileShader(vertexShaderData, GL_VERTEX_SHADER);
		HEAP_FREE(vertexShaderData);

		glAttachShader(shaderProgram, vertexShader);
	}

	u32 fragmentShader = 0;
	if (fragmentFile)
	{
		void* fragmentShaderData = platform.readFile(fragmentFile);
	    fragmentShader = compileShader(fragmentShaderData, GL_FRAGMENT_SHADER);
		HEAP_FREE(fragmentShaderData);

		glAttachShader(shaderProgram, fragmentShader);
	}

	u32 tcs = 0;
	if (tcsFile)
	{
		void* tcsData = platform.readFile(tcsFile);
		tcs = compileShader(tcsData, GL_TESS_CONTROL_SHADER);
		HEAP_FREE(tcsData);

		glAttachShader(shaderProgram, tcs);
	}

	u32 tes = 0;
	if (tesFile)
	{
		void* tesData = platform.readFile(tesFile);
		tes = compileShader(tesData, GL_TESS_EVALUATION_SHADER);
		HEAP_FREE(tesData);

		glAttachShader(shaderProgram, tes);
	}

	//TODO(denis): check if this succeeded or failed
	glLinkProgram(shaderProgram);

	glUseProgram(shaderProgram);

	// this works because a value of 0 for the shader defined as being silently ignored
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(tcs);
	glDeleteShader(tes);
	
	return shaderProgram;
}

#endif
