#pragma once

class CD3D12ResourceManager
{
	ID3D12Device5*	m_pD3DDevice = nullptr;
	ID3D12CommandQueue*	m_pCommandQueue = nullptr;
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;

	HANDLE	m_hFenceEvent = nullptr;
	ID3D12Fence* m_pFence = nullptr;
	UINT64	m_ui64FenceValue = 0;

	void	CreateFence();
	void	CleanupFence();
	void	CreateCommandList();
	void	CleanupCommandList();


	UINT64	Fence();
	void	WaitForFenceValue();

	void	Cleanup();

public:
	BOOL	Initialize(ID3D12Device5* pD3DDevice);
	HRESULT CreateVertexBuffer(UINT SizePerVertex, DWORD dwVertexNum, D3D12_VERTEX_BUFFER_VIEW* pOutVertexBufferView, ID3D12Resource **ppOutBuffer, void* pInitData, BOOL bUseGpuUploadHeaps);
	HRESULT CreateIndexBuffer(DWORD dwIndexNum, D3D12_INDEX_BUFFER_VIEW* pOutIndexBufferView, ID3D12Resource **ppOutBuffer, void* pInitData, BOOL bUseGpuUploadHeaps);
	void	UpdateTextureForWrite(ID3D12Resource* pDestTexResource, ID3D12Resource* pSrcTexResource);
	BOOL	CreateTexture(ID3D12Resource** ppOutResource, UINT Width, UINT Height, DXGI_FORMAT format, const BYTE* pInitImage);
	BOOL	CreateTextureFromFile(ID3D12Resource** ppOutResource, D3D12_RESOURCE_DESC* pOutDesc, const WCHAR* wchFileName, BOOL bUseGpuUploadHeaps);
	BOOL	CreateTexturePair(ID3D12Resource** ppOutResource, ID3D12Resource** ppOutUploadBuffer, UINT Width, UINT Height, DXGI_FORMAT format);
	CD3D12ResourceManager();
	~CD3D12ResourceManager();
};

