#include <Windows.h>
#ifdef _DEBUG
#include <iostream>
#endif

#include "D3D12AppBase.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// Window procedure.
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// If window is destroyed. 
	if (msg == WM_DESTROY)
	{
		// Send the message that this application is quit to OS.
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

#ifdef _DEBUG
int main()
{
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
#endif
	D3D12AppBase d3dApp{};

	WNDCLASSEX w = {};

	w.cbSize = sizeof(w);
	w.style = CS_HREDRAW | CS_VREDRAW;
	w.lpfnWndProc = (WNDPROC)WindowProc; 		// Assignment of callback function.
	w.lpszClassName = L"DirectX 12 Madousho.";
#ifdef _DEBUG
	w.hInstance = GetModuleHandle(nullptr); 	// Get the handle
#else 
	w.hInstance = hInstance;
#endif
	w.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&w); // Application class.

	RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT}; // Define the window size.
	
	// Adjust the window size.
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

	// Create window object.
	DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
	HWND hwnd = CreateWindow(w.lpszClassName,	// Assignment of class name.
		L"DirectX12 Test",						// Name of window.
		dwStyle,					// The window have borderline between title bar and application.
		CW_USEDEFAULT,							// The x, y coordinate of window in display.
		CW_USEDEFAULT,			
		rect.right - rect.left,					// The width of window.
		rect.bottom - rect.top,					// The height of window.
		nullptr,								// Window handle of parent.
		nullptr,								// Menu handle.
		w.hInstance,							// Calling application.
		nullptr									// Additional parameter.
	);

	try
	{
		d3dApp.Initialize(hwnd);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&d3dApp));
		int cmdShow;
#ifdef _DEBUG
		cmdShow = SW_SHOW;
#else
		cmdShow = nCmdShow;
#endif
		ShowWindow(hwnd, cmdShow);

		MSG msg{};
		while(msg.message != WM_QUIT)
		{
			if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			d3dApp.Render();
		}

		d3dApp.Terminate();
		return static_cast<int>(msg.wParam);
	}
	catch(const std::exception& e) 
	{
		
	}

	UnregisterClass(w.lpszClassName, w.hInstance);
	return 0;
}