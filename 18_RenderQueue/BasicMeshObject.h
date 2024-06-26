#pragma once

enum BASIC_MESH_DESCRIPTOR_INDEX_PER_OBJ
{
	BASIC_MESH_DESCRIPTOR_INDEX_PER_OBJ_CBV = 0
};
enum BASIC_MESH_DESCRIPTOR_INDEX_PER_TRI_GROUP
{
	BASIC_MESH_DESCRIPTOR_INDEX_PER_TRI_GROUP_TEX = 0
};

struct INDEXED_TRI_GROUP
{
	ID3D12Resource* pIndexBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};
	DWORD	dwTriCount;
	TEXTURE_HANDLE* pTexHandle;
};
class CD3D12Renderer;
class CBasicMeshObject
{
public:
	static const UINT DESCRIPTOR_COUNT_PER_OBJ = 1;			// | Constant Buffer
	static const UINT DESCRIPTOR_COUNT_PER_TRI_GROUP = 1;	// | SRV(tex)
	static const UINT MAX_TRI_GROUP_COUNT_PER_OBJ = 8;
	static const UINT MAX_DESCRIPTOR_COUNT_FOR_DRAW = DESCRIPTOR_COUNT_PER_OBJ + (MAX_TRI_GROUP_COUNT_PER_OBJ * DESCRIPTOR_COUNT_PER_TRI_GROUP);

private:
	// shared by all CBasicMeshObject instances.
	static ID3D12RootSignature* m_pRootSignature;
	static ID3D12PipelineState* m_pPipelineState;
	static DWORD	m_dwInitRefCount;

	CD3D12Renderer* m_pRenderer = nullptr;
	
	// vertex data
	ID3D12Resource* m_pVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};

	INDEXED_TRI_GROUP*	m_pTriGroupList = nullptr;
	DWORD	m_dwTriGroupCount = 0;
	DWORD	m_dwMaxTriGroupCount = 0;

	BOOL	InitCommonResources();
	void	CleanupSharedResources();

	BOOL	InitRootSinagture();
	BOOL	InitPipelineState();

	void	DeleteTriGroup(INDEXED_TRI_GROUP* pTriGroup);
	void	Cleanup();

public:
	BOOL	Initialize(CD3D12Renderer* pRenderer);
	void	Draw(ID3D12GraphicsCommandList* pCommandList, const XMMATRIX* pMatWorld);

	BOOL	BeginCreateMesh(const BasicVertex* pVertexList, DWORD dwVertexNum, DWORD dwTriGroupCount);
	BOOL	InsertIndexedTriList(const WORD* pIndexList, DWORD dwTriCount, const WCHAR* wchTexFileName);
	void	EndCreateMesh();

	CBasicMeshObject();
	~CBasicMeshObject();
};

