#pragma once

#include "common.h"

// heShader: defines a shader object
typedef struct _heShader {

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint shaderProgram;

} heShader;

int heCreateShader(GLuint *shader, GLenum type, const char *code)
{
	// create shader
	*shader = glCreateShader(type);

	// attach source code to shader
	glShaderSource(*shader, 1, (const GLchar **)&code, NULL);

	// compile vertex shader source code
	glCompileShader(*shader);

	// compilation errors 
	GLint iShaderCompileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar *szInfoLog = NULL;

	glGetShaderiv(*shader, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(*shader, GL_INFO_LOG_LENGTH, &written, szInfoLog);

				heLog("Shader Compiler Info Log: %s", szInfoLog);
				free(szInfoLog);
			}
		}
	}

	return iShaderCompileStatus;
}

int heCreateProgram(GLuint *program, GLuint vertexShader, GLuint fragmentShader)
{
	// create shader program object 
	*program = glCreateProgram();

	// attach vertex shader to shader program
	glAttachShader(*program, vertexShader);

	// attach fragment shader to shader program
	glAttachShader(*program, fragmentShader);

	// link the shader program
	glLinkProgram(*program);

	// linking errors
	GLint iProgramLinkStatus = 0;
	GLint iInfoLogLength = 0;
	char *szInfoLog = NULL;

	glGetProgramiv(*program, GL_LINK_STATUS, &iProgramLinkStatus);
	if (iProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(*program, GL_INFO_LOG_LENGTH, &written, szInfoLog);

				heLog("Shader Program Linking Info Log: %s", szInfoLog);
				free(szInfoLog);
			}
		}
	}

	return iProgramLinkStatus;
}

// create a new shader program object using vertex and fragment shader source code
heShader *heNewShader(const char *vertexShaderCode, const char *fragmentShaderCode)
{
	int compileStatus;
	int linkStatus;
	heShader shader;

	compileStatus = heCreateShader(&shader.vertexShader, GL_VERTEX_SHADER, vertexShaderCode);
	if (compileStatus == GL_FALSE) 
	{
		heLog("Vertex shader compilation failed...");
		return NULL;
	}
	
	compileStatus = heCreateShader(&shader.fragmentShader, GL_FRAGMENT_SHADER, fragmentShaderCode);
	if (compileStatus == GL_FALSE)
	{
		heLog("Fragment shader compilation failed...");
		return NULL;
	}

	linkStatus = heCreateProgram(&shader.shaderProgram, shader.vertexShader, shader.fragmentShader);
	if (compileStatus == GL_FALSE)
	{
		heLog("Program linking failed...");
		return NULL;
	}

	return &shader;
}

// delete shader program object
void heShaderUninit(heShader *shader)
{
	GLsizei shaderCount;
	GLsizei shaderNumber;

	glUseProgram(shader->shaderProgram);
	glGetProgramiv(shader->shaderProgram, GL_ATTACHED_SHADERS, &shaderCount);

	GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);
	if (pShaders)
	{
		glGetAttachedShaders(shader->shaderProgram, shaderCount, &shaderCount, pShaders);

		for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
		{
			// detach shader
			glDetachShader(shader->shaderProgram, pShaders[shaderNumber]);

			// delete shader
			glDeleteShader(pShaders[shaderNumber]);
			pShaders[shaderNumber] = 0;
		}
		free(pShaders);
	}

	glDeleteProgram(shader->shaderProgram);
	shader->shaderProgram = 0;
	glUseProgram(0);
}



