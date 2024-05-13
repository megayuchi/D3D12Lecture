#pragma once

const UINT SWAP_CHAIN_FRAME_COUNT = 2;
const UINT MAX_PENDING_FRAME_NUM = SWAP_CHAIN_FRAME_COUNT - 1;

class CD3D12ResourceManager;
class CDescriptorPool;
class CSimpleConstantBufferPool;
class CSingleDescriptorAllocator;
class CD3D12Renderer
{
	static const UINT MAX_DRAW_COUNT_PER_FRAME = 256;
	static const UINT MAX_DESCRIPTOR_COUNT = 4096;
	static const UINT PRE_CREATED_TEXTURE_COUNT = 2;
	HWND	m_hWnd = nullptr;
	ID3D12Device5*	m_pD3DDevice = nullptr;
	ID3D12CommandQueue*	m_pCommandQueue = nullptr;
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;
	CD3D12ResourceManager*	m_pResourceManager = nullptr;
	CDescriptorPool*	m_pDescriptorPool = nullptr;
	CSimpleConstantBufferPool* m_pConstantBufferPool = nullptr;
	CSingleDescriptorAllocator* m_pSingleDescriptorAllocator = nullptr;

	D3D_FEATURE_LEVEL	m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_ADAPTER_DESC1	m_AdapterDesc = {};
	IDXGISwapChain3*	m_pSwapChain = nullptr;
	D3D12_VIEWPORT	m_Viewport = {};
	D3D12_RECT		m_ScissorRect = {};
	DWORD			m_dwWidth = 0;
	DWORD			m_dwHeight = 0;

	ID3D12Resource*				m_pRenderTargets[SWAP_CHAIN_FRAME_COUNT] = {};
	ID3D12Resource*				m_pDepthStencil = nullptr;
	ID3D12DescriptorHeap*		m_pRTVHeap = nullptr;
	ID3D12DescriptorHeap*		m_pDSVHeap = nullptr;
	ID3D12DescriptorHeap*		m_pSRVHeap = nullptr;
	UINT	m_rtvDescriptorSize = 0;
	UINT	m_srvDescriptorSize = 0;
	UINT	m_dsvDescriptorSize = 0;
	UINT	m_dwSwapChainFlags = 0;
	UINT	m_uiFrameIndex = 0;
	HANDLE	m_hFenceEvent = nullptr;
	ID3D12Fence* m_pFence = nullptr;
	UINT64	m_ui64FenceValue = 0;
	
	DWORD	m_dwCurContextIndex = 0;
	XMMATRIX m_matView = {};
	XMMATRIX m_matProj = {};

	TEXTURE_HANDLE	m_TextureList[PRE_CREATED_TEXTURE_COUNT] = {};

	void	InitCamera();

	BOOL	CreateDepthStencil(UINT Width, UINT Height);

	void	CreateFence();
	void	CleanupFence();
	void	CreateCommandList();
	void	CleanupCommandList();
	BOOL	CreateDescriptorHeapForRTV();
	BOOL	CreateDescriptorHeapForDSV();

	void	CleanupDescriptorHeapForRTV();
	void	CleanupDescriptorHeapForDSV();

	UINT64	Fence();
	void	WaitForFenceValue();

	void	Cleanup();

	
public:
	BOOL	Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV);
	void	BeginRender();
	void	EndRender();
	void	Present();
	BOOL	UpdateWindowSize(DWORD dwBackBufferWidth, DWORD dwBackBufferHeight);
	
	void*	CretateBasicMeshObject();
	void	DeleteBasicMeshObject(void* pMeshObjHandle);

	void*	CreateTiledTexture(UINT TexWidth, UINT TexHeight, DWORD r, DWORD g, DWORD b);
	void*	CreateTextureFromFile(const WCHAR* wchFileName);
	void	DeleteTexture(void* pTexHandle);

	void	RenderMeshObject(void* pMeshObjHandle, const XMMATRIX* pMatWorld, void* pTexHandle);

	// for internal
	ID3D12Device5* INL_GetD3DDevice() const { return m_pD3DDevice; }
	CD3D12ResourceManager*	INL_GetResourceManager() { return m_pResourceManager; }
	CDescriptorPool*	INL_GetDescriptorPool() { return m_pDescriptorPool; }
	CSimpleConstantBufferPool* INL_GetConstantBufferPool() { return m_pConstantBufferPool; }
	UINT INL_GetSrvDescriptorSize() { return m_srvDescriptorSize; }
	CSingleDescriptorAllocator* INL_GetSingleDescriptorAllocator() { return m_pSingleDescriptorAllocator; }
	void	GetViewProjMatrix(XMMATRIX* pOutMatView, XMMATRIX* pOutMatProj);
	CD3D12Renderer();
	~CD3D12Renderer();
};

