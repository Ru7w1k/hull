// Headers
#include "common.h"
#include "shader.h"

// Defines
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// Global Variables
bool gbActiveWindow = false;
bool gbIsFullScreen = false;
HDC ghDC = NULL;
HGLRC ghRC = NULL;
HWND ghWnd = NULL;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

heShader *mainShader = NULL;

// Global function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// function declarations
	int winInit(void);
	void openglInit(void);
	void display(void);
	void update(void);

	// variables 
	bool bDone = false;
	int iRet = 0;
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szClassName[] = TEXT("MyApp");

	// code

	// initialization of WNDCLASSEX
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszClassName = szClassName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// register class
	RegisterClassEx(&wndclass);

	// create window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szClassName,
		TEXT("hull engine"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghWnd = hwnd;


	iRet = winInit();
	if (iRet != 0)
	{
		heLog("winInit failed...\nterminating...");
		DestroyWindow(ghWnd);
	}

	openglInit();

	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// Game Loop 
	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == true) 
			{
				update();
			}

			display();
		}
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function declarations
	void resize(int, int);
	void uninitialize();

	void ToggleFullScreen(void);

	// code
	switch (iMsg)
	{

	case WM_SETFOCUS:
		gbActiveWindow = true;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = false;
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 0x46:
			ToggleFullScreen();
			break;
		}
		break;

		// returned from here, to block DefWindowProc
		// We have our own painter
	case WM_ERASEBKGND:
		return(0);
		break;

	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen()
{
	MONITORINFO MI;

	if (gbIsFullScreen == false)
	{
		dwStyle = GetWindowLong(ghWnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			MI = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghWnd, &wpPrev)
				&& GetMonitorInfo(MonitorFromWindow(ghWnd, MONITORINFOF_PRIMARY), &MI))
			{
				SetWindowLong(ghWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghWnd,
					HWND_TOP,
					MI.rcMonitor.left,
					MI.rcMonitor.top,
					MI.rcMonitor.right - MI.rcMonitor.left,
					MI.rcMonitor.bottom - MI.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
		gbIsFullScreen = true;
	}
	else
	{
		SetWindowLong(ghWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghWnd, &wpPrev);
		SetWindowPos(ghWnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbIsFullScreen = false;
	}
}

int winInit(void)
{
	// variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;
	GLenum result;

	// code
	if (fopen_s(&logFile, "log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Cannot Create log file!"), TEXT("Error"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	else
	{
		heLog("====== Application started ======");
	}

	// initialize pdf structure
	ZeroMemory((void *)&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	ghDC = GetDC(ghWnd);

	iPixelFormatIndex = ChoosePixelFormat(ghDC, &pfd);
	// iPixelFormatIndex is 1 based, so 0 indicates error
	if (iPixelFormatIndex == 0)
	{
		heLog("ChoosePixelFormat failed...");
		return(-1);
	}

	if (SetPixelFormat(ghDC, iPixelFormatIndex, &pfd) == FALSE)
	{
		heLog("SetPixelFormat failed...");
		return(-1);
	}

	ghRC = wglCreateContext(ghDC);
	if (ghRC == NULL)
	{
		heLog("wglCreateContext failed...");
		return(-1);
	}

	if (wglMakeCurrent(ghDC, ghRC) == FALSE)
	{
		heLog("wglMakeCurrent failed...");
		return(-1);
	}

	//// Programable Pipeline ////////////////////////////////////////////

	result = glewInit();
	if (result != GLEW_OK) {
		heLog("GLEW initialization failed..");
		return(-1);
	}

	return(0);
}

void openglInit(void){

	// function declarations
	void resize(int, int);

	// vertex shader source code 
	const GLchar *vertexShaderSourceCode = (GLchar *)
		"#version 450" \
		"\n" \
		"void main (void)" \
		"{" \
		"}";

	
	// fragment shader source code
	const GLchar *fragmentShaderSourceCode = (GLchar *)
		"#version 450" \
		"\n" \
		"void main (void)" \
		"{" \
		"}";

	mainShader = heNewShader(vertexShaderSourceCode, fragmentShaderSourceCode);
	if (mainShader == NULL)
	{
		heLog("mainShader failed...");
	}

	//////////////////////////////////////////////////////////////////////

	// clear the depth buffer
	glClearDepth(1.0f);

	// clear the screen by OpenGL
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	// enable depth
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	// warm-up call to resize
	resize(WIN_WIDTH, WIN_HEIGHT);

	return;
}

void resize(int width, int height)
{
	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(mainShader->shaderProgram);

	glUseProgram(0);

	SwapBuffers(ghDC);
}

void update(void)
{

}

void uninitialize(void)
{
	
	heShaderUninit(mainShader);

	// fullscreen check
	if (gbIsFullScreen == true)
	{
		SetWindowLong(ghWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghWnd, &wpPrev);
		SetWindowPos(ghWnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
	}

	// break the current context
	if (wglGetCurrentContext() == ghRC)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (ghRC)
	{
		wglDeleteContext(ghRC);
	}

	if (ghDC)
	{
		ReleaseDC(ghWnd, ghDC);
		ghDC = NULL;
	}

	if (logFile)
	{
		heLog("====== Application finished ======");
		fclose(logFile);
		logFile = NULL;
	}
}

