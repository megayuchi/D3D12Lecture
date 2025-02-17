// 01_CreateDevice.cpp : Defines the entry point for the application.
//



#include "pch.h"
#include <Windows.h>
#include "Resource.h"
#include <DirectXMath.h>
#include "VertexUtil.h"
#include "D3D12Renderer.h"
#include "Game.h"

using namespace DirectX;

// required .lib files
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment( lib, "d3d11.lib" )
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#if defined(_M_ARM64EC) || defined(_M_ARM64)
	#ifdef _DEBUG
		#pragma comment(lib, "../DirectXTex/DirectXTex/Bin/Desktop_2022/ARM64/debug/DirectXTex.lib")
	#else
		#pragma comment(lib, "../DirectXTex/DirectXTex/Bin/Desktop_2022/ARM64/release/DirectXTex.lib")
	#endif
#elif defined(_M_AMD64)
	#ifdef _DEBUG
		#pragma comment(lib, "../DirectXTex/DirectXTex/Bin/Desktop_2022/x64/debug/DirectXTex.lib")
	#else
		#pragma comment(lib, "../DirectXTex/DirectXTex/Bin/Desktop_2022/x64/release/DirectXTex.lib")
	#endif
#elif defined(_M_IX86)
	#ifdef _DEBUG
		#pragma comment(lib, "../DirectXTex/DirectXTex/Bin/Desktop_2022/win32/debug/DirectXTex.lib")
	#else
		#pragma comment(lib, "../DirectXTex/DirectXTex/Bin/Desktop_2022/win32/release/DirectXTex.lib")
	#endif
#endif


extern "C" { __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }

//////////////////////////////////////////////////////////////////////////////////////////////////////
// D3D12 Agility SDK Runtime

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 615; }	

#if defined(_M_ARM64EC)
	extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\arm64\\"; }
#elif defined(_M_ARM64)
	extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\arm64\\"; }
#elif defined(_M_AMD64)
	extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\x64\\"; }
#elif defined(_M_IX86)
	extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\x86\\"; }
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst = nullptr;                                // current instance
HWND g_hMainWindow = nullptr;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


CGame* g_pGame = nullptr;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					  _In_opt_ HINSTANCE hPrevInstance,
					  _In_ LPWSTR    lpCmdLine,
					  _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MY01CREATEDEVICE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	g_hMainWindow = InitInstance (hInstance, nCmdShow);
	if (!g_hMainWindow)
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY01CREATEDEVICE));

	MSG msg;

	g_pGame = new CGame;
	//g_pGame->Initialize(g_hMainWindow, TRUE, TRUE);
	g_pGame->Initialize(g_hMainWindow, FALSE, FALSE);

	SetWindowText(g_hMainWindow, L"RenderQueue");
	// Main message loop:
	//while (GetMessage(&msg, nullptr, 0, 0))
	//{
	//	if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
	//	{
	//		TranslateMessage(&msg);
	//		DispatchMessage(&msg);
	//	}
	//}
	// Main message loop:
	while (1)
	{
		// call WndProc
		//g_bCanUseWndProc == FALSE이면 DefWndProc호출

		BOOL	bHasMsg = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);

		if (bHasMsg)
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);


		}
		else
		{
			if (g_pGame)
			{
				g_pGame->Run();
			}
		}
	}
	if (g_pGame)
	{
		delete g_pGame;
		g_pGame = nullptr;
	}
#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif
	return (int)msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY01CREATEDEVICE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY01CREATEDEVICE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
							  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return nullptr;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
			{
				int wmId = LOWORD(wParam);
				// Parse the menu selections:
				switch (wmId)
				{
					case IDM_ABOUT:
						DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
						break;
					case IDM_EXIT:
						DestroyWindow(hWnd);
						break;
					default:
						return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				// TODO: Add any drawing code that uses hdc here...
				EndPaint(hWnd, &ps);
			}
			break;
		case WM_SIZE:
			{
				if (g_pGame)
				{
					RECT	rect;
					GetClientRect(hWnd, &rect);
					DWORD	dwWndWidth = rect.right - rect.left;
					DWORD	dwWndHeight = rect.bottom - rect.top;
					g_pGame->UpdateWindowSize(dwWndWidth, dwWndHeight);
				}
			}
			break;
		case WM_KEYDOWN:
			{
				if (g_pGame)
				{
					UINT	uiScanCode = (0x00ff0000 & lParam) >> 16;
					UINT	vkCode = MapVirtualKey(uiScanCode, MAPVK_VSC_TO_VK);
					if (!(lParam & 0x40000000))
					{
						g_pGame->OnKeyDown(vkCode, uiScanCode);

					}
				}
			}
			break;

		case WM_KEYUP:
			{
				if (g_pGame)
				{
					UINT	uiScanCode = (0x00ff0000 & lParam) >> 16;
					UINT	vkCode = MapVirtualKey(uiScanCode, MAPVK_VSC_TO_VK);
					g_pGame->OnKeyUp(vkCode, uiScanCode);
				}
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
