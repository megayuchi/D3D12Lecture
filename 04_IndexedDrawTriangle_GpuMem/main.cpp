// 01_CreateDevice.cpp : Defines the entry point for the application.
//



#include "pch.h"
#include <Windows.h>
#include "Resource.h"
#include "D3D12Renderer.h"

// required .lib files
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

extern "C" { __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }

//////////////////////////////////////////////////////////////////////////////////////////////////////
// D3D12 Agility SDK Runtime

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 614; }	

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

CD3D12Renderer* g_pRenderer = nullptr;
void* g_pMeshObj = nullptr;

ULONGLONG g_PrvFrameCheckTick = 0;
ULONGLONG g_PrvUpdateTick = 0;
DWORD	g_FrameCount = 0;

void RunGame();
void Update();

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

	g_pRenderer = new CD3D12Renderer;
	g_pRenderer->Initialize(g_hMainWindow, TRUE, TRUE);
	g_pMeshObj = g_pRenderer->CreateBasicMeshObject();

	SetWindowText(g_hMainWindow, L"IndexedDrawTriangle_GpuMem");
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
			RunGame();
		}
	}
	if (g_pMeshObj)
	{
		g_pRenderer->DeleteBasicMeshObject(g_pMeshObj);
		g_pMeshObj = nullptr;
	}
	if (g_pRenderer)
	{
		delete g_pRenderer;
		g_pRenderer = nullptr;
	}
#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif
	return (int)msg.wParam;
}

void RunGame()
{
	g_FrameCount++;

	// begin
	ULONGLONG CurTick = GetTickCount64();

	g_pRenderer->BeginRender();

	// game business logic
	if (CurTick - g_PrvUpdateTick > 16)
	{
		// Update Scene with 60FPS
		Update();
		g_PrvUpdateTick = CurTick;
	}

	// rendering objects
	g_pRenderer->RenderMeshObject(g_pMeshObj);
	// end
	g_pRenderer->EndRender();

	// Present
	g_pRenderer->Present();
	
	if (CurTick - g_PrvFrameCheckTick > 1000)
	{
		g_PrvFrameCheckTick = CurTick;	
				
		WCHAR wchTxt[64];
		swprintf_s(wchTxt, L"FPS:%u", g_FrameCount);
		SetWindowText(g_hMainWindow, wchTxt);
				
		g_FrameCount = 0;
	}
}
void Update()
{

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
				if (g_pRenderer)
				{
					RECT	rect;
					GetClientRect(hWnd, &rect);
					DWORD	dwWndWidth = rect.right - rect.left;
					DWORD	dwWndHeight = rect.bottom - rect.top;
					g_pRenderer->UpdateWindowSize(dwWndWidth, dwWndHeight);
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
