#pragma once

#include <Windows.h>
#include <stdio.h>

#include <GL\glew.h>
#include <gl\GL.h>

// Linker Options
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")

#define heLog(...)    fprintf(logFile, __VA_ARGS__);fprintf(logFile, "\n")

enum {
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXCOORD0
};

FILE *logFile = NULL;

// initialize all components and OpenGL requirements
// compile and link all shaders
// create vao and vbos for objects
// load models
// load textures
// ...
void engineInit()
{

}

// uninitialize all components
// delete textures
// delete models
// delete objects
// delete shader programs
// ...
void engineUninit()
{

}
	