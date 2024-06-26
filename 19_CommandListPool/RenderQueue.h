#pragma once

enum RENDER_ITEM_TYPE
{
	RENDER_ITEM_TYPE_MESH_OBJ,
	RENDER_ITEM_TYPE_SPRITE
};

	//void	RenderMeshObject(void* pMeshObjHandle, const XMMATRIX* pMatWorld);
	//void	RenderSpriteWithTex(void* pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY, const RECT* pRect, float Z, void* pTexHandle);
	//void	RenderSprite(void* pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY, float Z);

struct RENDER_MESH_OBJ_PARAM
{
	XMMATRIX matWorld;
};
struct RENDER_SPRITE_PARAM
{
	int iPosX;
	int iPosY;
	float fScaleX;
	float fScaleY;
	RECT Rect;
	BOOL bUseRect;
	float Z;
	void* pTexHandle;
};

struct RENDER_ITEM
{
	RENDER_ITEM_TYPE Type;
	void* pObjHandle;
	union
	{
		RENDER_MESH_OBJ_PARAM	MeshObjParam;
		RENDER_SPRITE_PARAM	SpriteParam;
	};
};
class CCommandListPool;
class CD3D12Renderer;
class CRenderQueue
{
	CD3D12Renderer* m_pRenderer = nullptr;
	char* m_pBuffer = nullptr;
	DWORD m_dwMaxBufferSize = 0;
	DWORD m_dwAllocatedSize = 0;
	DWORD m_dwReadBufferPos = 0;
	DWORD m_dwItemCount = 0;

	const RENDER_ITEM*	Dispatch();
	void	Cleanup();
	
public:
	BOOL Initialize(CD3D12Renderer* pRenderer, DWORD dwMaxItemNum);
	BOOL Add(const RENDER_ITEM* pItem);
	DWORD Process(ID3D12GraphicsCommandList* pCommandList);
	DWORD Process(CCommandListPool* pCommandListPool, ID3D12CommandQueue* pCommandQueue, DWORD dwProcessCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE dsv, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect);
	void Reset();

	CRenderQueue();
	~CRenderQueue();
};