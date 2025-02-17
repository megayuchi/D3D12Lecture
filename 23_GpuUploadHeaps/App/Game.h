#pragma once

#include "../Common/typedef.h"
#include "../Common/IRenderer.h"
#include "../Util/LinkedList.h"

class CGameObject;
class CGame
{
	HMODULE m_hRendererDLL = nullptr;
	IRenderer*	m_pRenderer = nullptr;
	HWND	m_hWnd = nullptr;
	ISprite* m_pSpriteObjCommon = nullptr;

	BYTE* m_pTextImage = nullptr;
	UINT m_TextImageWidth = 0;
	UINT m_TextImageHeight = 0;
	void* m_pTextTexTexHandle = nullptr;
	void* m_pFontObj = nullptr;

	BOOL	m_bShiftKeyDown = FALSE;

	float m_CamOffsetX = 0.0f;
	float m_CamOffsetY = 0.0f;
	float m_CamOffsetZ = 0.0f;

	// Game Objects
	SORT_LINK*	m_pGameObjLinkHead = nullptr;
	SORT_LINK*	m_pGameObjLinkTail = nullptr;

	
	
	ULONGLONG m_PrvFrameCheckTick = 0;
	ULONGLONG m_PrvUpdateTick = 0;
	DWORD	m_FrameCount = 0;
	DWORD	m_FPS = 0;
	DWORD	m_dwCommandListCount = 0;
	WCHAR m_wchText[64] = {};

	WCHAR	m_wchAppPath[_MAX_PATH] = {};
	void	Render();
	CGameObject* CreateGameObject();
	void	DeleteGameObject(CGameObject* pGameObj);
	void	DeleteAllGameObjects();

	

	void	Cleanup();
public:
	BOOL	Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV);
	void	Run();
	BOOL	Update(ULONGLONG CurTick);
	void	OnKeyDown(UINT nChar, UINT uiScanCode);
	void	OnKeyUp(UINT nChar, UINT uiScanCode);
	BOOL	UpdateWindowSize(DWORD dwBackBufferWidth, DWORD dwBackBufferHeight);

	IRenderer* INL_GetRenderer() const { return m_pRenderer; }

	CGame();
	~CGame();
};