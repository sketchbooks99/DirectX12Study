#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#ifdef _DEBUG
#include <iostream>
#endif
#include "TexturedCubeApp.h"
#include <stdexcept>

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	PAINTSTRUCT ps;
	HDC hdc;

	// Timer processing.
	SYSTEMTIME stTime;
	static TCHAR strTime[128];

	switch (msg)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_CREATE:
		SetTimer(hWnd, 1, 300, NULL);
		return 0;

	case WM_TIMER:
		GetLocalTime(&stTime);
		wsprintf(strTime, L"Year: %d, Month: %d, Day: %d, Hour: %d, Minute: %d, Second: %d",
			stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour, stTime.wMinute, stTime.wSecond);
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

void DispConsole() {
	// Create the console window.
	AllocConsole();
	FILE *fp = NULL;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONIN$", "r", stdin);
}

void DebugOutputFormatString(const char* format)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
	TexturedCubeApp theApp{};

	DispConsole();

	WNDCLASSEX wc{};
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"HelloDirectX12";
	RegisterClassEx(&wc);

	DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
	RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rect, dwStyle, FALSE);

	auto hwnd = CreateWindow(wc.lpszClassName, L"TexturedCube",
		dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		nullptr,
		nullptr,
		hInstance,
		&theApp
	);
	try {
		theApp.Initialize(hwnd);

		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&theApp));
		ShowWindow(hwnd, nCmdShow);

		MSG msg{};
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			theApp.Render();
		}
		theApp.Terminate();
		return static_cast<int>(msg.wParam);
	}
	catch (std::runtime_error e){
		DebugBreak();
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
	}
	return 0;
}