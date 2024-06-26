#pragma once

enum SPRITE_DESCRIPTOR_INDEX
{
	SPRITE_DESCRIPTOR_INDEX_CBV = 0,
	SPRITE_DESCRIPTOR_INDEX_TEX = 1
};

class CD3D12Renderer;
class CSpriteObject
{
public:
	static const UINT DESCRIPTOR_COUNT_FOR_DRAW = 2;	// | Constant Buffer | Tex |

private:
	// shared by all CSpriteObject instances.
	static ID3D12RootSignature* m_pRootSignature;
	static ID3D12PipelineState* m_pPipelineState;
	static DWORD	m_dwInitRefCount;

	// vertex data
	static ID3D12Resource* m_pVertexBuffer;
	static D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	// index data
	static ID3D12Resource* m_pIndexBuffer;
	static D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

	TEXTURE_HANDLE*	m_pTexHandle = nullptr;
	CD3D12Renderer* m_pRenderer = nullptr;
	RECT	m_Rect = {};
	XMFLOAT2	m_Scale = { 1.0f, 1.0f };
	

	DWORD	m_dwTriGroupCount = 0;
	DWORD	m_dwMaxTriGroupCount = 0;

	BOOL	InitCommonResources();
	void	CleanupSharedResources();

	BOOL	InitRootSinagture();
	BOOL	InitPipelineState();
	BOOL	InitMesh();

	void	Cleanup();

public:
	BOOL	Initialize(CD3D12Renderer* pRenderer);
	BOOL	Initialize(CD3D12Renderer* pRenderer, const WCHAR* wchTexFileName, const RECT* pRect);
	void	DrawWithTex(DWORD dwThreadIndex, ID3D12GraphicsCommandList* pCommandList, const XMFLOAT2* pPos, const XMFLOAT2* pScale, const RECT* pRect, float Z, TEXTURE_HANDLE* pTexHandle);
	void	Draw(DWORD dwThreadIndex, ID3D12GraphicsCommandList* pCommandList, const XMFLOAT2* pPos, const XMFLOAT2* pScale, float Z);

	CSpriteObject();
	~CSpriteObject();
};

