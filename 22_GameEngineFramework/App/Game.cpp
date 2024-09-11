#include "pch.h"
#include <Windows.h>
#include <DirectXMath.h>
#include "../Common/typedef.h"
#include "../Common/IRenderer.h"
#include "../Util/LinkedList.h"
#include "GameObject.h"
#include "Game.h"
#include <shlwapi.h>


CGame::CGame()
{

}
BOOL CGame::Initialiize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV)
{
	const WCHAR* wchRendererFileName = nullptr;

#if defined(_M_ARM64EC) || defined(_M_ARM64)
	#ifdef _DEBUG
		wchRendererFileName = L"../DLL/RendererD3D12_arm64_debug.dll";
	#else
		wchRendererFileName = L"../DLL/RendererD3D12_arm64_release.dll";
	#endif
#elif defined(_M_AMD64)
	#ifdef _DEBUG
		wchRendererFileName = L"../DLL/RendererD3D12_x64_debug.dll";
	#else
		wchRendererFileName = L"../DLL/RendererD3D12_x64_release.dll";
	#endif
#elif defined(_M_IX86)
	#ifdef _DEBUG
		wchRendererFileName = L"../DLL/RendererD3D12_x86_debug.dll";
	#else
		wchRendererFileName = L"../DLL/RendererD3D12_x86_release.dll";
	#endif
#endif
	WCHAR	wchErrTxt[128] = {};
	DWORD	dwErrCode = 0;

	m_hRendererDLL = LoadLibrary(wchRendererFileName);
	if (!m_hRendererDLL)
	{
		dwErrCode = GetLastError();
		swprintf_s(wchErrTxt, L"Fail to LoadLibrary(%s) - Error Code: %u", wchRendererFileName, dwErrCode);
		MessageBox(hWnd, wchErrTxt, L"Error", MB_OK);
		__debugbreak();
	}
	CREATE_INSTANCE_FUNC	pCreateFunc = (CREATE_INSTANCE_FUNC)GetProcAddress(m_hRendererDLL, "DllCreateInstance");
	pCreateFunc(&m_pRenderer);

	// Get App Path and Set Shader Path
	WCHAR wchShaderPath[_MAX_PATH] = {};
	HMODULE hModule = GetModuleHandle(nullptr);
	if (hModule)
	{
		WCHAR wchOldPath[_MAX_PATH] = {};
		GetCurrentDirectory(_MAX_PATH, wchOldPath);

		int ret = GetModuleFileName(hModule, m_wchAppPath, _MAX_PATH);
		PathRemoveFileSpec(m_wchAppPath);
		SetCurrentDirectory(m_wchAppPath);
		SetCurrentDirectory(L"../ShadersD3D12");
		GetCurrentDirectory(_MAX_PATH, wchShaderPath);

		SetCurrentDirectory(wchOldPath);
	}
	m_pRenderer->Initialize(hWnd, bEnableDebugLayer, bEnableGBV, wchShaderPath);
	m_hWnd = hWnd;

	// Create Font
	m_pFontObj = m_pRenderer->CreateFontObject(L"Tahoma", 18.0f);
	
	// create texture for draw text
	m_TextImageWidth = 512;
	m_TextImageHeight = 256;
	m_pTextImage = (BYTE*)malloc(m_TextImageWidth * m_TextImageHeight * 4);
	m_pTextTexTexHandle = m_pRenderer->CreateDynamicTexture(m_TextImageWidth, m_TextImageHeight);
	memset(m_pTextImage, 0, m_TextImageWidth * m_TextImageHeight * 4);

	m_pSpriteObjCommon = m_pRenderer->CreateSpriteObject();

	const DWORD GAME_OBJ_COUNT = 2000;
	for (DWORD i = 0; i < GAME_OBJ_COUNT; i++)
	{
		CGameObject* pGameObj = CreateGameObject();
		if (pGameObj)
		{
			float x = (float)((rand() % 21) - 10);	// -10m - 10m 
			float y = 0.0f;
			float z = (float)((rand() % 21) - 10);	// -10m - 10m 
			pGameObj->SetPosition(x, y, z);
			float rad = (rand() % 181) * (3.1415f / 180.0f);
			pGameObj->SetRotationY(rad);
		}
	}
	m_pRenderer->SetCameraPos(0.0f, 0.0f, -10.0f);
	return TRUE;
}

CGameObject* CGame::CreateGameObject()
{
	// meshobject를 공용으로 쓰도록 한다.
	//__debugbreak();
	CGameObject* pGameObj = new CGameObject;
	pGameObj->Initialize(this);
	LinkToLinkedListFIFO(&m_pGameObjLinkHead, &m_pGameObjLinkTail, &pGameObj->m_LinkInGame);

	return pGameObj;
}
void CGame::OnKeyDown(UINT nChar, UINT uiScanCode)
{
	switch (nChar)
	{
		case VK_SHIFT:
			m_bShiftKeyDown = TRUE;
			break;
		case 'W':
			if (m_bShiftKeyDown)
			{
				m_CamOffsetY = 0.05f;
			}
			else
			{
				m_CamOffsetZ = 0.05f;
			}
			break;
		case 'S':
			if (m_bShiftKeyDown)
			{
				m_CamOffsetY = -0.05f;
			}
			else
			{
				m_CamOffsetZ = -0.05f;
			}
			break;
		case 'A':
			m_CamOffsetX = -0.05f;
			break;
		case 'D':
			m_CamOffsetX = 0.05f;
			break;
	}
}
void CGame::OnKeyUp(UINT nChar, UINT uiScanCode)
{
	switch (nChar)
	{
		case VK_SHIFT:
			m_bShiftKeyDown = FALSE;
			break;
		case 'W':
			m_CamOffsetY = 0.0f;
			m_CamOffsetZ = 0.0f;
			break;
		case 'S':
			m_CamOffsetY = 0.0f;
			m_CamOffsetZ = 0.0f;
			break;
		case 'A':
			m_CamOffsetX = 0.0f;
			break;
		case 'D':
			m_CamOffsetX = 0.0f;
			break;
	}
}

void CGame::Run()
{
	m_FrameCount++;

	// begin
	ULONGLONG CurTick = GetTickCount64();

	// game business logic
	Update(CurTick);

	Render();

	if (CurTick - m_PrvFrameCheckTick > 1000)
	{
		m_PrvFrameCheckTick = CurTick;	
				
		WCHAR wchTxt[64];
		m_FPS = m_FrameCount;
		m_dwCommandListCount = m_pRenderer->GetCommandListCount();

		swprintf_s(wchTxt, L"FPS : %u, CommandList : %u ", m_FPS, m_dwCommandListCount);
		SetWindowText(m_hWnd, wchTxt);
				
		m_FrameCount = 0;
	}
}
BOOL CGame::Update(ULONGLONG CurTick)
{	
	// Update Scene with 60FPS
	if (CurTick - m_PrvUpdateTick < 16)
	{
		return FALSE;
	}
	m_PrvUpdateTick = CurTick;

	// Update camra
	if (m_CamOffsetX != 0.0f || m_CamOffsetY != 0.0f || m_CamOffsetZ != 0.0f)
	{
		m_pRenderer->MoveCamera(m_CamOffsetX, m_CamOffsetY, m_CamOffsetZ);
	}
	
	// update game objects
	SORT_LINK* pCur = m_pGameObjLinkHead;
	while (pCur)
	{
		CGameObject* pGameObj = (CGameObject*)pCur->pItem;
		pGameObj->Run();
		pCur = pCur->pNext;
	}
	
	// update status text
	int iTextWidth = 0;
	int iTextHeight = 0;
	WCHAR	wchTxt[64] = {};
	DWORD	dwTxtLen = swprintf_s(wchTxt, L"FrameRate: %u, CommandList: %u", m_FPS, m_dwCommandListCount);

	if (wcscmp(m_wchText, wchTxt))
	{
		// 텍스트가 변경된 경우
		memset(m_pTextImage, 0, m_TextImageWidth * m_TextImageHeight * 4);
		m_pRenderer->WriteTextToBitmap(m_pTextImage, m_TextImageWidth, m_TextImageHeight, m_TextImageWidth * 4, &iTextWidth, &iTextHeight, m_pFontObj, wchTxt, dwTxtLen);
		m_pRenderer->UpdateTextureWithImage(m_pTextTexTexHandle, m_pTextImage, m_TextImageWidth, m_TextImageHeight);
		wcscpy_s(m_wchText, wchTxt);
	}
	else
	{
		// 텍스트가 변경되지 않은 경우 - 업데이트 할 필요 없다.
		int a = 0;
	}
	return TRUE;
}
void CGame::Render()
{
	m_pRenderer->BeginRender();

	// render game objects
	SORT_LINK* pCur = m_pGameObjLinkHead;
	DWORD dwObjCount = 0;
	while (pCur)
	{
		CGameObject* pGameObj = (CGameObject*)pCur->pItem;
		pGameObj->Render();
		pCur = pCur->pNext;
		dwObjCount++;
	}	
	// render dynamic texture as text
	m_pRenderer->RenderSpriteWithTex(m_pSpriteObjCommon, 512 + 5, 256 + 5 + 256 + 5, 1.0f, 1.0f, nullptr, 0.0f, m_pTextTexTexHandle);

	// end
	m_pRenderer->EndRender();

	// Present
	m_pRenderer->Present();
}
void CGame::DeleteGameObject(CGameObject* pGameObj)
{
	UnLinkFromLinkedList(&m_pGameObjLinkHead, &m_pGameObjLinkTail, &pGameObj->m_LinkInGame);
	delete pGameObj;
}
void CGame::DeleteAllGameObjects()
{
	while (m_pGameObjLinkHead)
	{
		CGameObject* pGameObj = (CGameObject*)m_pGameObjLinkHead->pItem;
		DeleteGameObject(pGameObj);
	}
}
BOOL CGame::UpdateWindowSize(DWORD dwBackBufferWidth, DWORD dwBackBufferHeight)
{
	BOOL bResult = FALSE;
	if (m_pRenderer)
	{
		bResult = m_pRenderer->UpdateWindowSize(dwBackBufferWidth, dwBackBufferHeight);
	}
	return bResult;
}
void CGame::Cleanup()
{
	DeleteAllGameObjects();

	if (m_pTextImage)
	{
		free(m_pTextImage);
		m_pTextImage = nullptr;
	}
	if (m_pRenderer)
	{
		if (m_pFontObj)
		{
			m_pRenderer->DeleteFontObject(m_pFontObj);
			m_pFontObj = nullptr;
		}
	
		if (m_pTextTexTexHandle)
		{
			m_pRenderer->DeleteTexture(m_pTextTexTexHandle);
			m_pTextTexTexHandle = nullptr;
		}
		if (m_pSpriteObjCommon)
		{
			m_pSpriteObjCommon->Release();
			m_pSpriteObjCommon = nullptr;
		}

		m_pRenderer->Release();
		m_pRenderer = nullptr;
	}
	if (m_hRendererDLL)
	{
		FreeLibrary(m_hRendererDLL);
		m_hRendererDLL = nullptr;
	}
}
CGame::~CGame()
{
	Cleanup();
}


