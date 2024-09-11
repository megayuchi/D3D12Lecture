#include "pch.h"

#include <d3dcompiler.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include "../Common/typedef.h"
#include "../Common/IRenderer.h"
#include "D3DUtil.h"
#include "D3D12ResourceManager.h"
#include "SimpleConstantBufferPool.h"
#include "SingleDescriptorAllocator.h"
#include "DescriptorPool.h"

#include "D3D12Renderer.h"
#include "BasicMeshObject.h"

using namespace DirectX;

ID3D12RootSignature* CBasicMeshObject::m_pRootSignature = nullptr;
ID3D12PipelineState* CBasicMeshObject::m_pPipelineState = nullptr;
DWORD CBasicMeshObject::m_dwInitRefCount = 0;


STDMETHODIMP CBasicMeshObject::QueryInterface(REFIID refiid, void** ppv)
{
	return E_NOINTERFACE;
}
STDMETHODIMP_(ULONG) CBasicMeshObject::AddRef()
{
	m_dwRefCount++;
	return m_dwRefCount;

}
STDMETHODIMP_(ULONG) CBasicMeshObject::Release()
{
	DWORD	ref_count = --m_dwRefCount;
	if (!m_dwRefCount)
		delete this;

	return ref_count;
}
CBasicMeshObject::CBasicMeshObject()
{
}

BOOL CBasicMeshObject::Initialize(CD3D12Renderer* pRenderer)
{
	m_pRenderer = pRenderer;

	BOOL bResult = InitCommonResources();
	return bResult;
}
BOOL CBasicMeshObject::InitCommonResources()
{
	if (m_dwInitRefCount)
		goto lb_true;

	InitRootSinagture();
	InitPipelineState();

lb_true:
	m_dwInitRefCount++;
	return m_dwInitRefCount;
}
BOOL CBasicMeshObject::InitRootSinagture()
{
	ID3D12Device5* pD3DDeivce = m_pRenderer->INL_GetD3DDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	// Object - CBV - RootParam(0)
	// {
	//   TriGrup 0 - SRV[0] - RootParam(1) - Draw()
	//   TriGrup 1 - SRV[1] - RootParam(1) - Draw()
	//   TriGrup 2 - SRV[2] - RootParam(1) - Draw()
	//   TriGrup 3 - SRV[3] - RootParam(1) - Draw()
	//   TriGrup 4 - SRV[4] - RootParam(1) - Draw()
	//   TriGrup 5 - SRV[5] - RootParam(1) - Draw()
	// }

	CD3DX12_DESCRIPTOR_RANGE rangesPerObj[1] = {};
	rangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// b0 : Constant Buffer View per Object

	CD3DX12_DESCRIPTOR_RANGE rangesPerTriGroup[1] = {};
	rangesPerTriGroup[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	// t0 : Shader Resource View(Tex) per Tri-Group
	
	CD3DX12_ROOT_PARAMETER rootParameters[2] = {};
	rootParameters[0].InitAsDescriptorTable(_countof(rangesPerObj), rangesPerObj, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsDescriptorTable(_countof(rangesPerTriGroup), rangesPerTriGroup, D3D12_SHADER_VISIBILITY_ALL);


	// default sampler
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	SetDefaultSamplerDesc(&sampler, 0);
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	// Allow input layout and deny uneccessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// Create an empty root signature.
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
	{
		__debugbreak();
	}

	if (FAILED(pD3DDeivce->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature))))
	{
		__debugbreak();
	}
	if (pSignature)
	{
		pSignature->Release();
		pSignature = nullptr;
	}
	if (pError)
	{
		pError->Release();
		pError = nullptr;
	}
	return TRUE;
}
BOOL CBasicMeshObject::InitPipelineState()
{
	ID3D12Device5* pD3DDeivce = m_pRenderer->INL_GetD3DDevice();

	ID3DBlob* pVertexShader = nullptr;
	ID3DBlob* pPixelShader = nullptr;


#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	m_pRenderer->SetCurrentPathForShader();
	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"shBasicMesh.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &pVertexShader, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"shBasicMesh.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pPixelShader, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	m_pRenderer->RestoreCurrentPath();

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	0, 28,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};


	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_pRootSignature;
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(pVertexShader->GetBufferPointer(), pVertexShader->GetBufferSize());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pPixelShader->GetBufferPointer(), pPixelShader->GetBufferSize());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	if (FAILED(pD3DDeivce->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState))))
	{
		__debugbreak();
	}

	if (pVertexShader)
	{
		pVertexShader->Release();
		pVertexShader = nullptr;
	}
	if (pPixelShader)
	{
		pPixelShader->Release();
		pPixelShader = nullptr;
	}
	return TRUE;
}
BOOL __stdcall CBasicMeshObject::BeginCreateMesh(const BasicVertex* pVertexList, DWORD dwVertexNum, DWORD dwTriGroupCount)
{
	BOOL bResult = FALSE;
	ID3D12Device5* pD3DDeivce = m_pRenderer->INL_GetD3DDevice();
	CD3D12ResourceManager*	pResourceManager = m_pRenderer->INL_GetResourceManager();

	if (dwTriGroupCount > MAX_TRI_GROUP_COUNT_PER_OBJ)
		__debugbreak();

	if (FAILED(pResourceManager->CreateVertexBuffer(sizeof(BasicVertex), dwVertexNum, &m_VertexBufferView, &m_pVertexBuffer, (void*)pVertexList)))
	{
		__debugbreak();
		goto lb_return;
	}
	
	m_dwMaxTriGroupCount = dwTriGroupCount;
	m_pTriGroupList = new INDEXED_TRI_GROUP[m_dwMaxTriGroupCount];
	memset(m_pTriGroupList, 0, sizeof(INDEXED_TRI_GROUP) * m_dwMaxTriGroupCount);


	bResult = TRUE;

lb_return:
	return bResult;
}
BOOL __stdcall CBasicMeshObject::InsertTriGroup(const WORD* pIndexList, DWORD dwTriCount, const WCHAR* wchTexFileName)
{
	BOOL bResult = FALSE;

	ID3D12Device5* pD3DDeivce = m_pRenderer->INL_GetD3DDevice();
	UINT srvDescriptorSize = m_pRenderer->INL_GetSrvDescriptorSize();
	CD3D12ResourceManager*	pResourceManager = m_pRenderer->INL_GetResourceManager();
	CSingleDescriptorAllocator* pSingleDescriptorAllocator = m_pRenderer->INL_GetSingleDescriptorAllocator();

	ID3D12Resource* pIndexBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};

	if (m_dwTriGroupCount >= m_dwMaxTriGroupCount)
	{
		__debugbreak();
		goto lb_return;
	}
	if (FAILED(pResourceManager->CreateIndexBuffer(dwTriCount * 3, &IndexBufferView, &pIndexBuffer, (void*)pIndexList)))
	{
		__debugbreak();
		goto lb_return;
	}
	INDEXED_TRI_GROUP*	pTriGroup = m_pTriGroupList + m_dwTriGroupCount;
	pTriGroup->pIndexBuffer = pIndexBuffer;
	pTriGroup->IndexBufferView = IndexBufferView;
	pTriGroup->dwTriCount = dwTriCount;
	pTriGroup->pTexHandle = (TEXTURE_HANDLE*)m_pRenderer->CreateTextureFromFile(wchTexFileName);
	m_dwTriGroupCount++;
	bResult = TRUE;
lb_return:
	return bResult;
}
void __stdcall CBasicMeshObject::EndCreateMesh()
{

}
void CBasicMeshObject::Draw(DWORD dwThreadIndex, ID3D12GraphicsCommandList* pCommandList, const XMMATRIX* pMatWorld)
{
	// 각각의 draw()작업의 무결성을 보장하려면 draw() 작업마다 다른 영역의 descriptor table(shader visible)과 다른 영역의 CBV를 사용해야 한다.
	// 따라서 draw()할 때마다 CBV는 ConstantBuffer Pool로부터 할당받고, 렌더리용 descriptor table(shader visible)은 descriptor pool로부터 할당 받는다.

	ID3D12Device5* pD3DDeivce = m_pRenderer->INL_GetD3DDevice();
	UINT srvDescriptorSize = m_pRenderer->INL_GetSrvDescriptorSize();
	CDescriptorPool* pDescriptorPool = m_pRenderer->INL_GetDescriptorPool(dwThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->INL_GetDescriptorHeap();
	CSimpleConstantBufferPool* pConstantBufferPool = m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_DEFAULT,dwThreadIndex);
	

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescriptorTable = {};
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorTable = {};
	DWORD dwRequiredDescriptorCount = DESCRIPTOR_COUNT_PER_OBJ + (m_dwTriGroupCount * DESCRIPTOR_COUNT_PER_TRI_GROUP);

	if (!pDescriptorPool->AllocDescriptorTable(&cpuDescriptorTable, &gpuDescriptorTable, dwRequiredDescriptorCount))
	{
		__debugbreak();
	}

	// 각각의 draw()에 대해 독립적인 constant buffer(내부적으로는 같은 resource의 다른 영역)를 사용한다.
	CB_CONTAINER* pCB = pConstantBufferPool->Alloc();
	if (!pCB)
	{
		__debugbreak();
	}
	CONSTANT_BUFFER_DEFAULT* pConstantBufferDefault = (CONSTANT_BUFFER_DEFAULT*)pCB->pSystemMemAddr;

	// constant buffer의 내용을 설정
	// view/proj matrix
	m_pRenderer->GetViewProjMatrix(&pConstantBufferDefault->matView, &pConstantBufferDefault->matProj);
	
	// world matrix
	pConstantBufferDefault->matWorld = XMMatrixTranspose(*pMatWorld);

	// Descriptor Table 구성
	// 이번에 사용할 constant buffer의 descriptor를 렌더링용(shader visible) descriptor table에 카피

	// per Obj
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dest(cpuDescriptorTable, BASIC_MESH_DESCRIPTOR_INDEX_PER_OBJ_CBV, srvDescriptorSize);
	pD3DDeivce->CopyDescriptorsSimple(1, Dest, pCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);	// cpu측 코드에서는 cpu descriptor handle에만 write가능
	Dest.Offset(1, srvDescriptorSize);

	// per tri-group
	for (DWORD i = 0; i < m_dwTriGroupCount; i++)
	{
		INDEXED_TRI_GROUP* pTriGroup = m_pTriGroupList + i;
		TEXTURE_HANDLE* pTexHandle = pTriGroup->pTexHandle;
		if (pTexHandle)
		{
			pD3DDeivce->CopyDescriptorsSimple(1, Dest, pTexHandle->srv, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);	// cpu측 코드에서는 cpu descriptor handle에만 write가능
		}
		else
		{
			__debugbreak();
		}
		Dest.Offset(1, srvDescriptorSize);
	}
	
	// set RootSignature
	pCommandList->SetGraphicsRootSignature(m_pRootSignature);
	pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);

	// ex) when TriGroupCount = 3
	// per OBJ | TriGroup 0 | TriGroup 1 | TriGroup 2 |
	// CBV     |     SRV    |     SRV    |     SRV    | 

	pCommandList->SetPipelineState(m_pPipelineState);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);

	// set descriptor table for root-param 0
	pCommandList->SetGraphicsRootDescriptorTable(0, gpuDescriptorTable);	// Entry per Obj

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescriptorTableForTriGroup(gpuDescriptorTable, DESCRIPTOR_COUNT_PER_OBJ, srvDescriptorSize);
	for (DWORD i = 0; i < m_dwTriGroupCount; i++)
	{
		// set descriptor table for root-param 1
		pCommandList->SetGraphicsRootDescriptorTable(1, gpuDescriptorTableForTriGroup);	// Entry of Tri-Groups
		gpuDescriptorTableForTriGroup.Offset(1, srvDescriptorSize);

		INDEXED_TRI_GROUP* pTriGroup = m_pTriGroupList + i;
		pCommandList->IASetIndexBuffer(&pTriGroup->IndexBufferView);
		pCommandList->DrawIndexedInstanced(pTriGroup->dwTriCount * 3, 1, 0, 0, 0);
	}
}

void CBasicMeshObject::Cleanup()
{
	m_pRenderer->EnsureCompleted();
	// delete all triangles-group
	
	if (m_pTriGroupList)
	{
		for (DWORD i = 0; i < m_dwTriGroupCount; i++)
		{
			if (m_pTriGroupList[i].pIndexBuffer)
			{
				m_pTriGroupList[i].pIndexBuffer->Release();
				m_pTriGroupList[i].pIndexBuffer = nullptr;
			}
			if (m_pTriGroupList[i].pTexHandle)
			{
				m_pRenderer->DeleteTexture(m_pTriGroupList[i].pTexHandle);
				m_pTriGroupList[i].pTexHandle = nullptr;
			}
		}
		delete[] m_pTriGroupList;
		m_pTriGroupList = nullptr;
	}

	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}
	CleanupSharedResources();
}
void CBasicMeshObject::CleanupSharedResources()
{
	if (!m_dwInitRefCount)
		return;

	DWORD ref_count = --m_dwInitRefCount;
	if (!ref_count)
	{
		if (m_pRootSignature)
		{
			m_pRootSignature->Release();
			m_pRootSignature = nullptr;
		}
		if (m_pPipelineState)
		{
			m_pPipelineState->Release();
			m_pPipelineState = nullptr;
		}
	}
}
CBasicMeshObject::~CBasicMeshObject()
{
	Cleanup();
}