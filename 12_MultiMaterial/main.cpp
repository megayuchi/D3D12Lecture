// 01_CreateDevice.cpp : Defines the entry point for the application.
//



#include "pch.h"
#include <Windows.h>
#include "Resource.h"
#include <DirectXMath.h>
#include "VertexUtil.h"
#include "D3D12Renderer.h"

using namespace DirectX;

// required .lib files
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3D12.lib")
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

CD3D12Renderer* g_pRenderer = nullptr;
void* g_pMeshObj0 = nullptr;
void* g_pMeshObj1 = nullptr;

float g_fRot0 = 0.0f;
float g_fRot1 = 0.0f;
float g_fRot2 = 0.0f;

XMMATRIX g_matWorld0 = {};
XMMATRIX g_matWorld1 = {};
XMMATRIX g_matWorld2 = {};

ULONGLONG g_PrvFrameCheckTick = 0;
ULONGLONG g_PrvUpdateTick = 0;
DWORD	g_FrameCount = 0;

void RunGame();
void Update();
void* CreateBoxMeshObject();
void* CreateQuadMesh();

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
	//g_pRenderer->Initialize(g_hMainWindow, TRUE, TRUE);
	g_pRenderer->Initialize(g_hMainWindow, FALSE, FALSE);

	// Create Box Mesh
	g_pMeshObj0 = CreateBoxMeshObject();
	
	// create Triangle mesh
	g_pMeshObj1 = CreateQuadMesh();


	SetWindowText(g_hMainWindow, L"MultiMaterial");
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
	if (g_pMeshObj0)
	{
		g_pRenderer->DeleteBasicMeshObject(g_pMeshObj0);
		g_pMeshObj0 = nullptr;
	}
	if (g_pMeshObj1)
	{
		g_pRenderer->DeleteBasicMeshObject(g_pMeshObj1);
		g_pMeshObj1 = nullptr;
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
void* CreateBoxMeshObject()
{
	void* pMeshObj = nullptr;

	// create box mesh
	// create vertices and indices
	WORD	pIndexList[36] = {};
	BasicVertex* pVertexList = nullptr;
	DWORD dwVertexCount = CreateBoxMesh(&pVertexList, pIndexList, (DWORD)_countof(pIndexList), 0.25f);

	// create BasicMeshObject from Renderer
	pMeshObj = g_pRenderer->CreateBasicMeshObject();

	const WCHAR* wchTexFileNameList[6] =
	{
		L"tex_00.dds",
		L"tex_01.dds",
		L"tex_02.dds",
		L"tex_03.dds",
		L"tex_04.dds",
		L"tex_05.dds"
	};

	// Set meshes to the BasicMeshObject
	g_pRenderer->BeginCreateMesh(pMeshObj, pVertexList, dwVertexCount, 6);	// 박스의 6면-1면당 삼각형 2개-인덱스 6개
	for (DWORD i = 0; i < 6; i++)
	{
		g_pRenderer->InsertTriGroup(pMeshObj, pIndexList + i * 6, 2, wchTexFileNameList[i]);
	}
	g_pRenderer->EndCreateMesh(pMeshObj);

	// delete vertices and indices
	if (pVertexList)
	{
		DeleteBoxMesh(pVertexList);
		pVertexList = nullptr;
	}
	return pMeshObj;
}
void* CreateQuadMesh()
{
	void* pMeshObj = nullptr;
	pMeshObj = g_pRenderer->CreateBasicMeshObject();

	// Set meshes to the BasicMeshObject
	BasicVertex pVertexList[] =
	{
		{ { -0.25f, 0.25f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { 0.25f, 0.25f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { 0.25f, -0.25f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
	};

	WORD pIndexList[] =
	{
		0, 1, 2,
		0, 2, 3
	};


	g_pRenderer->BeginCreateMesh(pMeshObj, pVertexList, (DWORD)_countof(pVertexList), 1);	
	g_pRenderer->InsertTriGroup(pMeshObj, pIndexList, 2, L"tex_06.dds");
	g_pRenderer->EndCreateMesh(pMeshObj);
	return pMeshObj;
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

	// 하나의 오브젝트를 여러 컨텍스트로 렌더링
	g_pRenderer->RenderMeshObject(g_pMeshObj0, &g_matWorld0);
	g_pRenderer->RenderMeshObject(g_pMeshObj0, &g_matWorld1);

	// 다른 오브젝트를 렌더링
	g_pRenderer->RenderMeshObject(g_pMeshObj1, &g_matWorld2);
	
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
	//
	// world matrix 0
	//
	g_matWorld0 = XMMatrixIdentity();

	// rotation 
	XMMATRIX matRot0 = XMMatrixRotationX(g_fRot0);

	// translation
	XMMATRIX matTrans0 = XMMatrixTranslation(-0.5f, 0.0f, 0.25f);

	// rot0 x trans0
	g_matWorld0 = XMMatrixMultiply(matRot0, matTrans0);

	//
	// world matrix 1
	//
	g_matWorld1 = XMMatrixIdentity();

	// world matrix 1
	// rotation 
	XMMATRIX matRot1 = XMMatrixRotationY(g_fRot1);

	// translation
	XMMATRIX matTrans1 = XMMatrixTranslation(0.0f, 0.0f, 0.25f);

	// rot1 x trans1
	g_matWorld1 = XMMatrixMultiply(matRot1, matTrans1);

	//
	// world matrix 2
	//
	g_matWorld2 = XMMatrixIdentity();

	// world matrix 2
	// rotation 
	XMMATRIX matRot2 = XMMatrixRotationZ(g_fRot2);

	// translation
	XMMATRIX matTrans2 = XMMatrixTranslation(0.5f, 0.0f, 0.25f);

	// rot2 x trans2
	g_matWorld2 = XMMatrixMultiply(matRot2, matTrans2);


	BOOL	bChangeTex = FALSE;
	g_fRot0 += 0.05f;
	if (g_fRot0 > 2.0f * 3.1415f)
	{
		g_fRot0 = 0.0f;
		bChangeTex = TRUE;
	}

	g_fRot1 += 0.1f;
	if (g_fRot1 > 2.0f * 3.1415f)
	{
		g_fRot1 = 0.0f;
	}

	g_fRot2 += 0.1f;
	if (g_fRot2 > 2.0f * 3.1415f)
	{
		g_fRot2 = 0.0f;
	}
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
