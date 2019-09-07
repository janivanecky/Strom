#include <windows.h>
#include <windowsx.h>
#define DRAW
#define ENGINE
#include "main.h"

#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  

#include <stdio.h>

DrawContext dContext;
AppContext aContext;
bool isRunning = true;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	{
		isRunning = false;
	}
	break;
	case WM_SIZE:
	{
		if (dContext.swapChain)
		{
			Resize(&aContext, LOWORD(lParam), HIWORD(lParam));
		}
			
	} break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

HWND GetWin32Window(LPCSTR windowName, UINT screenWidth, UINT screenHeight)
{
	HINSTANCE hInstance = GetModuleHandle(0);
	WNDCLASSEXA windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEXA);
	windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = &WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);
	windowClass.lpszClassName = "DrawWindowClass";

	if (RegisterClassExA(&windowClass))
	{
		HWND window = CreateWindowA("DrawWindowClass", windowName, WS_VISIBLE | WS_POPUP | WS_CLIPCHILDREN, 0, 0,
			screenWidth, screenHeight, NULL, NULL, hInstance, NULL);
		return window;
	}
	return NULL;
}

void ProcessMessage(HWND window, Input *input)
{
	MSG message;
	while (PeekMessageA(&message, window, 0, 0, PM_REMOVE))
	{
		RECT windowRect;
		GetWindowRect(window, &windowRect);
		float screenWidth = (float)(windowRect.right - windowRect.left);
		float screenHeight = (float)(windowRect.top - windowRect.bottom);

		switch (message.message)
		{
		case WM_KEYDOWN:
		{
			char c = MapVirtualKey((UINT)message.wParam, MAPVK_VK_TO_CHAR);
			if (!input->keyDown[c])
			{
				input->keyPressed[c] = true;
			}
			input->keyDown[c] = true;
			switch (message.wParam)
			{
			case VK_ESCAPE:
				isRunning = false;
				break;
			case VK_LEFT:
			{
				if (!input->leftButtonDown)
					input->leftButtonPressed = true;
				input->leftButtonDown = true;
				break;
			}
			case VK_RIGHT:
			{
				if (!input->rightButtonDown)
					input->rightButtonPressed = true;
				input->rightButtonDown = true;
				break;
			}
			}
		} break;
		case WM_KEYUP:
		{
			char c = MapVirtualKey((UINT)message.wParam, MAPVK_VK_TO_CHAR);
			input->keyDown[c] = false;
			switch (message.wParam)
			{
				case VK_LEFT:
					input->leftButtonDown = false;
					break;
				case VK_RIGHT:
					input->rightButtonDown = false;
					break;
			}
		} break;
		case WM_LBUTTONUP:
		{
			input->leftMouseButtonDown = false;
		} break;
		case WM_LBUTTONDOWN:
		{
			if (!input->leftMouseButtonDown)
				input->leftMouseButtonPressed = true;
			input->leftMouseButtonDown = true;
		} break;
		case WM_MOUSEMOVE:
		{
			float newPositionX = (float)GET_X_LPARAM(message.lParam);// / screenWidth;
			float newPositionY = (float)GET_Y_LPARAM(message.lParam);// / screenWidth;
			input->mouseDX = newPositionX - input->mousePositionX;
			input->mouseDY = newPositionY - input->mousePositionY;
			input->mousePositionX = newPositionX;
			input->mousePositionY = newPositionY;
		} break;
		case WM_MOUSEWHEEL:
		{
			input->mouseScrollDelta = GET_WHEEL_DELTA_WPARAM(message.wParam);
		} break;
		default:
		{
			TranslateMessage(&message);
			DispatchMessageA(&message);
		} break;
		}
	}
}

#define SCREEN_X 2560
#define SCREEN_Y 1440

int main(int argc, char *argv[])
{
	HWND window = GetWin32Window("STROM", SCREEN_X, SCREEN_Y);
	SetProcessDPIAware();
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	if (window)
	{
		dContext = Draw::GetContext(window, SCREEN_X, SCREEN_Y);
		aContext.dContext = &dContext;

		Init(&aContext, SCREEN_X, SCREEN_Y);

		Input input = {};
		LARGE_INTEGER start, end;

		QueryPerformanceCounter(&end);
		while (isRunning)
		{
			POINT screenPos;
			GetCursorPos(&screenPos);
			input.rawPositionX = screenPos.x;
			input.rawPositionY = screenPos.y;
			input.mouseScrollDelta = 0.0f;
			input.leftMouseButtonPressed = false;
			input.mouseDY = 0.0f;
			input.mouseDX = 0.0f;
			input.leftButtonPressed = false;
			input.rightButtonPressed = false;
			for (uint32_t i = 0; i < ARRAYSIZE(input.keyPressed); ++i)
			{
				input.keyPressed[i] = false;
			}

			ProcessMessage(window, &input);
			QueryPerformanceCounter(&start);
			float dt = (start.LowPart - end.LowPart) / (float)freq.LowPart;
			end = start;
			if (Update(&aContext, &input, dt))
			{
				isRunning = false;
			}
			//printf("ASDASDASD\n");
		}
		
		Release(&aContext);
		Draw::Release(&dContext);
		_CrtDumpMemoryLeaks();
	}
	return 0;
}