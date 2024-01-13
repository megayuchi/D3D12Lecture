#pragma once

class CD3D12Renderer;
class CBasicMeshObject
{
	// shared by all CBasicMeshObject instances.
	static ID3D12RootSignature* m_pRootSignature;
	static ID3D12PipelineState* m_pPipelineState;
	static DWORD	m_dwInitRefCount;

	CD3D12Renderer* m_pRenderer = nullptr;
	ID3D12Resource* m_pVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};

	BOOL	InitRootSinagture();
	BOOL	InitPipelineState();
	void	Cleanup();

public:
	BOOL	Initialize(CD3D12Renderer* pRenderer);
	void	Draw(ID3D12GraphicsCommandList* pCommandList);
	BOOL	CreateMesh();

	CBasicMeshObject();
	~CBasicMeshObject();
};

