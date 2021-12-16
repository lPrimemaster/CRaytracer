#include "../inc/display.h"
#include "../inc/common.h"

#include <windows.h>
#include <stdio.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

//TODO: global for now
global_variable uint8 running;
global_variable BITMAPINFO bitmapInfo;
global_variable void* bitmapMemory;
global_variable int bitmapWidth;
global_variable int bitmapHeight;

internal void Win32InitDIBSection(int width, int height)
{
	bitmapWidth = width;
	bitmapHeight = height;

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = bitmapWidth;
	bitmapInfo.bmiHeader.biHeight = -bitmapHeight;		// use a top-down DIB
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;				// bits per pixel
	bitmapInfo.bmiHeader.biCompression = BI_RGB;		// uncompressed
	
	// NOTE: no more DC. We can allocate memory ourselves
	int bytesPerPixel = 4;
	int bitmapMemorySize = (width * height) * bytesPerPixel;
	bitmapMemory = VirtualAlloc(NULL, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32UpdateWindow(HDC deviceContext, RECT* windowRect, int x, int y, int width, int height)
{
	int windowWidth = windowRect->right - windowRect->left;
	int windowHeight = windowRect->bottom - windowRect->top;

	int cnt = StretchDIBits(deviceContext,
		/*x, y, width, height,
		x, y, width, height,*/
		0, 0, bitmapWidth, bitmapHeight,
		0, 0, windowWidth, windowHeight,
		bitmapMemory,
		&bitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(
	HWND window,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	LRESULT result = 0;

	switch (message)
	{
		case WM_DESTROY:
		{
			// TODO: handle this as an error - recreate window?
			running = 0;
		} break;

		case WM_CLOSE:
		{
			// TODO: handle this with a message to the user? because we may want to close just an internal window in the game
			running = 0;
		} break;

		// when window becomes the active window
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(window, &paint);

			int x = paint.rcPaint.left;
			int y = paint.rcPaint.top;
			int height = paint.rcPaint.bottom - paint.rcPaint.top;
			int width = paint.rcPaint.right - paint.rcPaint.left;

			RECT clientRect;
			GetClientRect(window, &clientRect);
			
			Win32UpdateWindow(deviceContext, &clientRect, x, y, width, height);
			
			EndPaint(window, &paint);
		} break;

		default:
		{
			//OutputDebugStringA("default\n");
			result = DefWindowProc(window, message, wParam, lParam);
		} break;
	}

	return result;
}

int run_window(u32* data, i32 width, i32 height)
{
    Win32InitDIBSection(width, height);

	WNDCLASS windowClass = { 0 };
	
	// TODO: Check if these flags still matter
	windowClass.style = (CS_OWNDC | CS_HREDRAW | CS_VREDRAW) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
	windowClass.lpfnWndProc = Win32MainWindowCallback;
	windowClass.hInstance = GetModuleHandle(0); // or GetModuleHandle(0);
	//windowClass.hIcon;
	windowClass.lpszClassName = "RaytracerWindowClass";
	
	// register window class before creating the window
	if (RegisterClass(&windowClass))
	{
		HWND windowHandle = CreateWindowExA(
			0,
			windowClass.lpszClassName,
			"CRaytracer",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			bitmapWidth,
			bitmapHeight,
			0,
			0,
			windowClass.hInstance,
			0);

		if (windowHandle)
		{
			// start message loop
			MSG message;
			running = 1;
            u8 time = 0;
			while (running)
			{
				while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				}

                EnterCriticalSection(&CriticalSection);
                memcpy(bitmapMemory, data, bitmapWidth * bitmapHeight * 4);
                LeaveCriticalSection(&CriticalSection);
                RECT clientRect;
                GetClientRect(windowHandle, &clientRect);
                Win32UpdateWindow(GetDC(windowHandle), &clientRect, 0, 0, bitmapWidth, bitmapHeight);
			}
		}
		else
		{
			// TODO: logging
		}
	}
	else
	{
		//TODO: logging
	}

	return 0;
}