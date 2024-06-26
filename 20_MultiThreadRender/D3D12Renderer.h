#pragma once

#include "Renderer_typedef.h"

#define USE_MULTI_THREAD

class CD3D12ResourceManager;
class CDescriptorPool;
class CSimpleConstantBufferPool;
class CSingleDescriptorAllocator;
class CConstantBufferManager;
class CFontManager;
class CTextureManager;
class CRenderQueue;
class CCommandListPool;
struct RENDER_THREAD_DESC;

class CD3D12Renderer
{
	static const UINT MAX_DRAW_COUNT_PER_FRAME = 4096;
	static const UINT MAX_DESCRIPTOR_COUNT = 4096;
	static const UINT MAX_RENDER_THREAD_COUNT = 8;

	HWND	m_hWnd = nullptr;
	ID3D12Device5*	m_pD3DDevice = nullptr;
	ID3D12CommandQueue*	m_pCommandQueue = nullptr;
	CD3D12ResourceManager*	m_pResourceManager = nullptr;
	CFontManager*			m_pFontManager = nullptr;
	CSingleDescriptorAllocator* m_pSingleDescriptorAllocator = nullptr;

	CCommandListPool* m_ppCommandListPool[MAX_PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
	CDescriptorPool* m_ppDescriptorPool[MAX_PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
	CConstantBufferManager* m_ppConstBufferManager[MAX_PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
	CRenderQueue* m_ppRenderQueue[MAX_RENDER_THREAD_COUNT] = {};
	DWORD m_dwRenderThreadCount = 0;
	DWORD m_dwCurThreadIndex = 0;

	LONG volatile m_lActiveThreadCount = 0;
	HANDLE m_hCompleteEvent = nullptr;
	RENDER_THREAD_DESC*	m_pThreadDescList = nullptr;

	CTextureManager* m_pTextureManager = nullptr;

	UINT64	m_pui64LastFenceValue[MAX_PENDING_FRAME_COUNT] = {};
	UINT64	m_ui64FenceVaule = 0;


	D3D_FEATURE_LEVEL	m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_ADAPTER_DESC1	m_AdapterDesc = {};
	IDXGISwapChain3*	m_pSwapChain = nullptr;
	D3D12_VIEWPORT	m_Viewport = {};
	D3D12_RECT		m_ScissorRect = {};
	DWORD			m_dwWidth = 0;
	DWORD			m_dwHeight = 0;
	float 			m_fDPI = 96.0f;
	ID3D12Resource*				m_pRenderTargets[SWAP_CHAIN_FRAME_COUNT] = {};
	ID3D12Resource*				m_pDepthStencil = nullptr;
	ID3D12DescriptorHeap*		m_pRTVHeap = nullptr;
	ID3D12DescriptorHeap*		m_pDSVHeap = nullptr;
	ID3D12DescriptorHeap*		m_pSRVHeap = nullptr;
	UINT	m_rtvDescriptorSize = 0;
	UINT	m_srvDescriptorSize = 0;
	UINT	m_dsvDescriptorSize = 0;
	UINT	m_dwSwapChainFlags = 0;
	UINT	m_uiRenderTargetIndex = 0;
	HANDLE	m_hFenceEvent = nullptr;
	ID3D12Fence* m_pFence = nullptr;

	DWORD	m_dwCurContextIndex = 0;
	XMMATRIX m_matView = {};
	XMMATRIX m_matProj = {};

	XMVECTOR	m_CamPos = {};
	XMVECTOR	m_CamDir = {};

	void	InitCamera();

	BOOL	CreateDepthStencil(UINT Width, UINT Height);

	void	CreateFence();
	void	CleanupFence();
	BOOL	CreateDescriptorHeapForRTV();
	BOOL	CreateDescriptorHeapForDSV();

	void	CleanupDescriptorHeapForRTV();
	void	CleanupDescriptorHeapForDSV();
	UINT64	Fence();
	void	WaitForFenceValue(UINT64 ExpectedFenceValue);

	void	Cleanup();

	// for multi-threads
	BOOL	InitRenderThreadPool(DWORD dwThreadCount);
	void	CleanupRenderThreadPool();


public:
	BOOL	Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV);
	void	BeginRender();
	void	EndRender();
	void	Present();
	BOOL	UpdateWindowSize(DWORD dwBackBufferWidth, DWORD dwBackBufferHeight);

	void*	CreateBasicMeshObject();
	void	DeleteBasicMeshObject(void* pMeshObjHandle);

	void*	CreateSpriteObject();
	void*	CreateSpriteObject(const WCHAR* wchTexFileName, int PosX, int PosY, int Width, int Height);
	void	DeleteSpriteObject(void* pSpriteObjHandle);

	BOOL	BeginCreateMesh(void* pMeshObjHandle, const BasicVertex* pVertexList, DWORD dwVertexCount, DWORD dwTriGroupCount);
	BOOL	InsertTriGroup(void* pMeshObjHandle, const WORD* pIndexList, DWORD dwTriCount, const WCHAR* wchTexFileName);
	void	EndCreateMesh(void* pMeshObjHandle);

	void*	CreateTiledTexture(UINT TexWidth, UINT TexHeight, DWORD r, DWORD g, DWORD b);
	void*	CreateDynamicTexture(UINT TexWidth, UINT TexHeight);
	void*	CreateTextureFromFile(const WCHAR* wchFileName);
	void	DeleteTexture(void* pTexHandle);

	void*	CreateFontObject(const WCHAR* wchFontFamilyName, float fFontSize);
	void	DeleteFontObject(void* pFontHandle);
	BOOL	WriteTextToBitmap(BYTE* pDestImage, UINT DestWidth, UINT DestHeight, UINT DestPitch, int* piOutWidth, int* piOutHeight, void* pFontObjHandle, const WCHAR* wchString, DWORD dwLen);

	void	RenderMeshObject(void* pMeshObjHandle, const XMMATRIX* pMatWorld);
	void	RenderSpriteWithTex(void* pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY, const RECT* pRect, float Z, void* pTexHandle);
	void	RenderSprite(void* pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY, float Z);
	void	UpdateTextureWithImage(void* pTexHandle, const BYTE* pSrcBits, UINT SrcWidth, UINT SrcHeight);

	void	SetCameraPos(float x, float y, float z);
	void	MoveCamera(float x, float y, float z);
	void	GetCameraPos(float* pfOutX, float* pfOutY, float* pfOutZ);
	void	SetCamera(const XMVECTOR* pCamPos, const XMVECTOR* pCamDir, const XMVECTOR* pCamUp);
	DWORD	GetCommandListCount();
	// for internal
	ID3D12Device5* INL_GetD3DDevice() const { return m_pD3DDevice; }
	CD3D12ResourceManager*	INL_GetResourceManager() { return m_pResourceManager; }

	CDescriptorPool*	INL_GetDescriptorPool(DWORD dwThreadIndex) { return m_ppDescriptorPool[m_dwCurContextIndex][dwThreadIndex]; }
	CSimpleConstantBufferPool*	GetConstantBufferPool(CONSTANT_BUFFER_TYPE type, DWORD dwThreadIndex);

	UINT INL_GetSrvDescriptorSize() { return m_srvDescriptorSize; }
	CSingleDescriptorAllocator* INL_GetSingleDescriptorAllocator() { return m_pSingleDescriptorAllocator; }
	void	GetViewProjMatrix(XMMATRIX* pOutMatView, XMMATRIX* pOutMatProj);
	DWORD	INL_GetScreenWidth() const { return m_dwWidth; }
	DWORD	INL_GetScreenHeigt() const { return m_dwHeight; }
	float	INL_GetDPI() const { return m_fDPI; }
	
	// from RenderThread
	void	ProcessByThread(DWORD dwThreadIndex);

	CD3D12Renderer();
	~CD3D12Renderer();
};

