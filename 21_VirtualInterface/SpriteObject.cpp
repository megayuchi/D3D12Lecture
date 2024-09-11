#include "pch.h"
#include "typedef.h"
#include <d3dcompiler.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include "../D3D_Util/D3DUtil.h"
#include "D3D12ResourceManager.h"
#include "SimpleConstantBufferPool.h"
#include "SingleDescriptorAllocator.h"
#include "DescriptorPool.h"
#include "D3D12Renderer.h"
#include "SpriteObject.h"

using namespace DirectX;

ID3D12RootSignature* CSpriteObject::m_pRootSignature = nullptr;
ID3D12PipelineState* CSpriteObject::m_pPipelineState = nullptr;

ID3D12Resource* CSpriteObject::m_pVertexBuffer = nullptr;
D3D12_VERTEX_BUFFER_VIEW CSpriteObject::m_VertexBufferView = {};

ID3D12Resource* CSpriteObject::m_pIndexBuffer = nullptr;
D3D12_INDEX_BUFFER_VIEW CSpriteObject::m_IndexBufferView = {};

DWORD CSpriteObject::m_dwInitRefCount = 0;

STDMETHODIMP CSpriteObject::QueryInterface(REFIID refiid, void** ppv)
{
	return E_NOINTERFACE;
}
STDMETHODIMP_(ULONG) CSpriteObject::AddRef()
{
	m_dwRefCount++;
	return m_dwRefCount;

}
STDMETHODIMP_(ULONG) CSpriteObject::Release()
{
	DWORD	ref_count = --m_dwRefCount;
	if (!m_dwRefCount)
		delete this;

	return ref_count;
}

CSpriteObject::CSpriteObject()
{
}

BOOL CSpriteObject::Initialize(CD3D12Renderer* pRenderer)
{
	m_pRenderer = pRenderer;

	BOOL bResult = InitCommonResources();
	return bResult;
}
BOOL CSpriteObject::Initialize(CD3D12Renderer* pRenderer, const WCHAR* wchTexFileName, const RECT* pRect)
{
	m_pRenderer = pRenderer;

	BOOL bResult = (InitCommonResources() != 0);
	if (bResult)
	{
		UINT TexWidth = 1;
		UINT TexHeight = 1;
		m_pTexHandle = (TEXTURE_HANDLE*)m_pRenderer->CreateTextureFromFile(wchTexFileName);
		if (m_pTexHandle)
		{
			D3D12_RESOURCE_DESC	 desc = m_pTexHandle->pTexResource->GetDesc();
			TexWidth = (UINT)desc.Width;
			TexHeight = (UINT)desc.Height;
		}
		if (pRect)
		{
			m_Rect = *pRect;
			m_Scale.x = (float)(m_Rect.right - m_Rect.left) / (float)TexWidth;
			m_Scale.y = (float)(m_Rect.bottom - m_Rect.top) / (float)TexHeight;
		}
		else
		{
			if (m_pTexHandle)
			{
				D3D12_RESOURCE_DESC	 desc = m_pTexHandle->pTexResource->GetDesc();
				m_Rect.left = 0;
				m_Rect.top = 0;
				m_Rect.right = (LONG)desc.Width;
				m_Rect.bottom = (LONG)desc.Height;
			}
		}
	}
	return bResult;
}
BOOL CSpriteObject::InitCommonResources()
{
	if (m_dwInitRefCount)
		goto lb_true;

	InitRootSinagture();
	InitPipelineState();
	InitMesh();

lb_true:
	m_dwInitRefCount++;
	return m_dwInitRefCount;
}
BOOL CSpriteObject::InitRootSinagture()
{
	ID3D12Device5* pD3DDeivce = m_pRenderer->INL_GetD3DDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	CD3DX12_DESCRIPTOR_RANGE ranges[2] = {};
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// b0 : Constant Buffer View
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	// t0 : Shader Resource View(Tex)

	CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
	rootParameters[0].InitAsDescriptorTable(_countof(ranges), ranges, D3D12_SHADER_VISIBILITY_ALL);

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
BOOL CSpriteObject::InitPipelineState()
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
	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"./Shaders/shSprite.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &pVertexShader, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"./Shaders/shSprite.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pPixelShader, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}


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
BOOL CSpriteObject::InitMesh()
{
	// 바깥에서 버텍스데이터와 텍스처를 입력하는 식으로 변경할 것

	BOOL bResult = FALSE;
	ID3D12Device5* pD3DDeivce = m_pRenderer->INL_GetD3DDevice();
	UINT srvDescriptorSize = m_pRenderer->INL_GetSrvDescriptorSize();
	CD3D12ResourceManager*	pResourceManager = m_pRenderer->INL_GetResourceManager();
	CSingleDescriptorAllocator* pSingleDescriptorAllocator = m_pRenderer->INL_GetSingleDescriptorAllocator();

	// Create the vertex buffer.
	// Define the geometry for a triangle.
	BasicVertex Vertices[] =
	{
		{ { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
	};


	WORD Indices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	const UINT VertexBufferSize = sizeof(Vertices);

	if (FAILED(pResourceManager->CreateVertexBuffer(sizeof(BasicVertex), (DWORD)_countof(Vertices), &m_VertexBufferView, &m_pVertexBuffer, Vertices)))
	{
		__debugbreak();
		goto lb_return;
	}

	if (FAILED(pResourceManager->CreateIndexBuffer((DWORD)_countof(Indices), &m_IndexBufferView, &m_pIndexBuffer, Indices)))
	{
		__debugbreak();
		goto lb_return;
	}
	bResult = TRUE;

lb_return:
	return bResult;
}
void CSpriteObject::Draw(DWORD dwThreadIndex, ID3D12GraphicsCommandList* pCommandList, const XMFLOAT2* pPos, const XMFLOAT2* pScale, float Z)
{
	XMFLOAT2 Scale = { m_Scale.x * pScale->x, m_Scale.y * pScale->y };
	DrawWithTex(dwThreadIndex, pCommandList, pPos, &Scale, &m_Rect, Z, m_pTexHandle);
}
void CSpriteObject::DrawWithTex(DWORD dwThreadIndex, ID3D12GraphicsCommandList* pCommandList, const XMFLOAT2* pPos, const XMFLOAT2* pScale, const RECT* pRect, float Z, TEXTURE_HANDLE* pTexHandle)
{
	// 각각의 draw()작업의 무결성을 보장하려면 draw() 작업마다 다른 영역의 descriptor table(shader visible)과 다른 영역의 CBV를 사용해야 한다.
	// 따라서 draw()할 때마다 CBV는 ConstantBuffer Pool로부터 할당받고, 렌더리용 descriptor table(shader visible)은 descriptor pool로부터 할당 받는다.

	ID3D12Device5* pD3DDeivce = m_pRenderer->INL_GetD3DDevice();
	UINT srvDescriptorSize = m_pRenderer->INL_GetSrvDescriptorSize();
	CDescriptorPool* pDescriptorPool = m_pRenderer->INL_GetDescriptorPool(dwThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->INL_GetDescriptorHeap();
	CSimpleConstantBufferPool* pConstantBufferPool = m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_SPRITE, dwThreadIndex);

	UINT TexWidth = 0;
	UINT TexHeight = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE srv = {};
	if (pTexHandle)
	{
		D3D12_RESOURCE_DESC desc = pTexHandle->pTexResource->GetDesc();
		TexWidth = (UINT)desc.Width;
		TexHeight = (UINT)desc.Height;
		srv = pTexHandle->srv;
	}

	RECT rect;
	if (!pRect)
	{
		rect.left = 0;
		rect.top = 0;
		rect.right = TexWidth;
		rect.bottom = TexHeight;
		pRect = &rect;
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescriptorTable = {};
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorTable = {};

	if (!pDescriptorPool->AllocDescriptorTable(&cpuDescriptorTable, &gpuDescriptorTable, DESCRIPTOR_COUNT_FOR_DRAW))
	{
		__debugbreak();
	}

	// 각각의 draw()에 대해 독립적인 constant buffer(내부적으로는 같은 resource의 다른 영역)를 사용한다.
	CB_CONTAINER* pCB = pConstantBufferPool->Alloc();
	if (!pCB)
	{
		__debugbreak();
	}
	CONSTANT_BUFFER_SPRITE* pConstantBufferSprite = (CONSTANT_BUFFER_SPRITE*)pCB->pSystemMemAddr;

	// constant buffer의 내용을 설정
	pConstantBufferSprite->ScreenRes.x = (float)m_pRenderer->INL_GetScreenWidth();
	pConstantBufferSprite->ScreenRes.y = (float)m_pRenderer->INL_GetScreenHeigt();
	pConstantBufferSprite->Pos = *pPos;
	pConstantBufferSprite->Scale = *pScale;
	pConstantBufferSprite->TexSize.x = (float)TexWidth;
	pConstantBufferSprite->TexSize.y = (float)TexHeight;
	pConstantBufferSprite->TexSampePos.x = (float)pRect->left;
	pConstantBufferSprite->TexSampePos.y = (float)pRect->top;
	pConstantBufferSprite->TexSampleSize.x = (float)(pRect->right - pRect->left);
	pConstantBufferSprite->TexSampleSize.y = (float)(pRect->bottom - pRect->top);
	pConstantBufferSprite->Z = Z;
	pConstantBufferSprite->Alpha = 1.0f;



	// set RootSignature
	pCommandList->SetGraphicsRootSignature(m_pRootSignature);
	pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);

	// Descriptor Table 구성
	// 이번에 사용할 constant buffer의 descriptor를 렌더링용(shader visible) descriptor table에 카피

	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvDest(cpuDescriptorTable, SPRITE_DESCRIPTOR_INDEX_CBV, srvDescriptorSize);
	pD3DDeivce->CopyDescriptorsSimple(1, cbvDest, pCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	if (srv.ptr)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvDest(cpuDescriptorTable, SPRITE_DESCRIPTOR_INDEX_TEX, srvDescriptorSize);
		pD3DDeivce->CopyDescriptorsSimple(1, srvDest, srv, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	pCommandList->SetGraphicsRootDescriptorTable(0, gpuDescriptorTable);

	pCommandList->SetPipelineState(m_pPipelineState);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	pCommandList->IASetIndexBuffer(&m_IndexBufferView);
	pCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

}

void CSpriteObject::Cleanup()
{
	if (m_pTexHandle)
	{
		m_pRenderer->DeleteTexture(m_pTexHandle);
		m_pTexHandle = nullptr;
	}
	CleanupSharedResources();
}
void CSpriteObject::CleanupSharedResources()
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
		if (m_pVertexBuffer)
		{
			m_pVertexBuffer->Release();
			m_pVertexBuffer = nullptr;
		}
		if (m_pIndexBuffer)
		{
			m_pIndexBuffer->Release();
			m_pIndexBuffer = nullptr;
		}
	}
}
CSpriteObject::~CSpriteObject()
{
	m_pRenderer->EnsureCompleted();
	Cleanup();
}