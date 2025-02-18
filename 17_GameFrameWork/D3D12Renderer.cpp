#include "pch.h"
#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <dxgidebug.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include "typedef.h"
#include "../D3D_Util/D3DUtil.h"
#include "BasicMeshObject.h"
#include "SpriteObject.h"
#include "D3D12ResourceManager.h"
#include "FontManager.h"
#include "DescriptorPool.h"
#include "SimpleConstantBufferPool.h"
#include "SingleDescriptorAllocator.h"
#include "ConstantBufferManager.h"
#include "TextureManager.h"
#include "D3D12Renderer.h"

using namespace DirectX;

CD3D12Renderer::CD3D12Renderer()
{

}
BOOL CD3D12Renderer::Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV)
{
	BOOL	bResult = FALSE;

	HRESULT hr = S_OK;
	ID3D12Debug*	pDebugController = nullptr;
	IDXGIFactory4*	pFactory = nullptr;
	IDXGIAdapter1*	pAdapter = nullptr;
	DXGI_ADAPTER_DESC1 AdapterDesc = {};

	DWORD dwCreateFlags = 0;
	DWORD dwCreateFactoryFlags = 0;
	m_fDPI = GetDpiForWindow(hWnd);

	// if use debug Layer...
	if (bEnableDebugLayer)
	{
		// Enable the D3D12 debug layer.
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
		{
			pDebugController->EnableDebugLayer();
		}
		dwCreateFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
		if (bEnableGBV)
		{
			ID3D12Debug5*	pDebugController5 = nullptr;
			if (S_OK == pDebugController->QueryInterface(IID_PPV_ARGS(&pDebugController5)))
			{
				pDebugController5->SetEnableGPUBasedValidation(TRUE);
				pDebugController5->SetEnableAutoName(TRUE);
				pDebugController5->Release();
			}
		}
	}

	// Create DXGIFactory
	CreateDXGIFactory2(dwCreateFactoryFlags, IID_PPV_ARGS(&pFactory));

	D3D_FEATURE_LEVEL	featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	DWORD	FeatureLevelNum = _countof(featureLevels);

	for (DWORD featerLevelIndex = 0; featerLevelIndex < FeatureLevelNum; featerLevelIndex++)
	{
		UINT adapterIndex = 0;
		while (DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &pAdapter))
		{
			pAdapter->GetDesc1(&AdapterDesc);

			if (SUCCEEDED(D3D12CreateDevice(pAdapter, featureLevels[featerLevelIndex], IID_PPV_ARGS(&m_pD3DDevice))))
			{
				goto lb_exit;

			}
			pAdapter->Release();
			pAdapter = nullptr;
			adapterIndex++;
		}
	}
lb_exit:

	if (!m_pD3DDevice)
	{
		__debugbreak();
		goto lb_return;
	}

	m_AdapterDesc = AdapterDesc;
	m_hWnd = hWnd;

	if (pDebugController)
	{
		SetDebugLayerInfo(m_pD3DDevice);
	}

	// Describe and create the command queue.
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		hr = m_pD3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));
		if (FAILED(hr))
		{
			__debugbreak();
			goto lb_return;
		}
	}

	CreateDescriptorHeapForRTV();

	RECT	rect;
	::GetClientRect(hWnd, &rect);
	DWORD	dwWndWidth = rect.right - rect.left;
	DWORD	dwWndHeight = rect.bottom - rect.top;
	UINT	dwBackBufferWidth = rect.right - rect.left;
	UINT	dwBackBufferHeight = rect.bottom - rect.top;

	// Describe and create the swap chain.
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = dwBackBufferWidth;
		swapChainDesc.Height = dwBackBufferHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//swapChainDesc.BufferDesc.RefreshRate.Numerator = m_uiRefreshRate;
		//swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = SWAP_CHAIN_FRAME_COUNT;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		m_dwSwapChainFlags = swapChainDesc.Flags;


		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
		fsSwapChainDesc.Windowed = TRUE;

		IDXGISwapChain1*	pSwapChain1 = nullptr;
		if (FAILED(pFactory->CreateSwapChainForHwnd(m_pCommandQueue, hWnd, &swapChainDesc, &fsSwapChainDesc, nullptr, &pSwapChain1)))
		{
			__debugbreak();
		}
		pSwapChain1->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
		pSwapChain1->Release();
		pSwapChain1 = nullptr;
		m_uiRenderTargetIndex = m_pSwapChain->GetCurrentBackBufferIndex();
	}
	m_Viewport.Width = (float)dwWndWidth;
	m_Viewport.Height = (float)dwWndHeight;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.right = dwWndWidth;
	m_ScissorRect.bottom = dwWndHeight;

	m_dwWidth = dwWndWidth;
	m_dwHeight = dwWndHeight;

	// Create frame resources.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());

	// Create a RTV for each frame.
	// Descriptor Table
	// |        0        |        1	       |
	// | Render Target 0 | Render Target 1 |
	for (UINT n = 0; n < SWAP_CHAIN_FRAME_COUNT; n++)
	{
		m_pSwapChain->GetBuffer(n, IID_PPV_ARGS(&m_pRenderTargets[n]));
		m_pD3DDevice->CreateRenderTargetView(m_pRenderTargets[n], nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}
	m_srvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	// Create Depth Stencile resources
	CreateDescriptorHeapForDSV();
	CreateDepthStencil(m_dwWidth, m_dwHeight);

	CreateCommandList();

	// Create synchronization objects.
	CreateFence();

	m_pFontManager = new CFontManager;
	m_pFontManager->Initialize(this, m_pCommandQueue, 1024, 256, bEnableDebugLayer);

	m_pResourceManager = new CD3D12ResourceManager;
	m_pResourceManager->Initialize(m_pD3DDevice);

	m_pTextureManager = new CTextureManager;
	m_pTextureManager->Initialize(this, 1024 / 16, 1024);

	for (DWORD i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
	{
		m_ppDescriptorPool[i] = new CDescriptorPool;
		m_ppDescriptorPool[i]->Initialize(m_pD3DDevice, MAX_DRAW_COUNT_PER_FRAME * CBasicMeshObject::MAX_DESCRIPTOR_COUNT_FOR_DRAW);

		m_ppConstBufferManager[i] = new CConstantBufferManager;
		m_ppConstBufferManager[i]->Initialize(m_pD3DDevice, MAX_DRAW_COUNT_PER_FRAME);
	}
	m_pSingleDescriptorAllocator = new CSingleDescriptorAllocator;
	m_pSingleDescriptorAllocator->Initialize(m_pD3DDevice, MAX_DESCRIPTOR_COUNT, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	InitCamera();

	bResult = TRUE;
lb_return:
	if (pDebugController)
	{
		pDebugController->Release();
		pDebugController = nullptr;
	}
	if (pAdapter)
	{
		pAdapter->Release();
		pAdapter = nullptr;
	}
	if (pFactory)
	{
		pFactory->Release();
		pFactory = nullptr;
	}
	return bResult;
}
void CD3D12Renderer::InitCamera()
{
	// 카메라 위치, 카메라 방향, 위쪽 방향을 설정
	m_CamPos = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f); // 카메라 위치
	m_CamDir = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // 카메라 방향 (정면을 향하도록 설정)
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 위쪽 방향 (일반적으로 y축을 따라 설정)

	SetCamera(&m_CamPos, &m_CamDir, &Up);
}
void CD3D12Renderer::SetCamera(const XMVECTOR* pCamPos, const XMVECTOR* pCamDir, const XMVECTOR* pCamUp)
{
	// view matrix
	m_matView = XMMatrixLookToLH(*pCamPos, *pCamDir, *pCamUp);

	// 시야각 (FOV) 설정 (라디안 단위)
	float fovY = XM_PIDIV4; // 90도 (라디안으로 변환)

	// projection matrix
	float fAspectRatio = (float)m_dwWidth / (float)m_dwHeight;
	float fNear = 0.1f;
	float fFar = 1000.0f;
	m_matProj = XMMatrixPerspectiveFovLH(fovY, fAspectRatio, fNear, fFar);
}
void CD3D12Renderer::SetCameraPos(float x, float y, float z)
{
	m_CamPos.m128_f32[0] = x;
	m_CamPos.m128_f32[1] = y;
	m_CamPos.m128_f32[2] = z;
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 위쪽 방향 (일반적으로 y축을 따라 설정)

	SetCamera(&m_CamPos, &m_CamDir, &Up);
}
void CD3D12Renderer::MoveCamera(float x, float y, float z)
{
	m_CamPos.m128_f32[0] += x;
	m_CamPos.m128_f32[1] += y;
	m_CamPos.m128_f32[2] += z;
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 위쪽 방향 (일반적으로 y축을 따라 설정)

	SetCamera(&m_CamPos, &m_CamDir, &Up);
}

void CD3D12Renderer::GetCameraPos(float* pfOutX, float* pfOutY, float* pfOutZ)
{
	*pfOutX = m_CamPos.m128_f32[0];
	*pfOutY = m_CamPos.m128_f32[1];
	*pfOutZ = m_CamPos.m128_f32[2];
}
void CD3D12Renderer::GetViewProjMatrix(XMMATRIX* pOutMatView, XMMATRIX* pOutMatProj)
{
	*pOutMatView = XMMatrixTranspose(m_matView);
	*pOutMatProj = XMMatrixTranspose(m_matProj);
}
CSimpleConstantBufferPool* CD3D12Renderer::GetConstantBufferPool(CONSTANT_BUFFER_TYPE type)
{
	CConstantBufferManager* pConstBufferManager = m_ppConstBufferManager[m_dwCurContextIndex];
	CSimpleConstantBufferPool* pConstBufferPool = pConstBufferManager->GetConstantBufferPool(type);
	return pConstBufferPool;
}

BOOL CD3D12Renderer::CreateDepthStencil(UINT Width, UINT Height)
{
	// Create the depth stencil view.
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	CD3DX12_RESOURCE_DESC depthDesc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		Width,
		Height,
		1,
		1,
		DXGI_FORMAT_R32_TYPELESS,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	if (FAILED(m_pD3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_pDepthStencil)
		)))
	{
		__debugbreak();
	}
	m_pDepthStencil->SetName(L"CD3D12Renderer::m_pDepthStencil");

	CD3DX12_CPU_DESCRIPTOR_HANDLE	dsvHandle(m_pDSVHeap->GetCPUDescriptorHandleForHeapStart());
	m_pD3DDevice->CreateDepthStencilView(m_pDepthStencil, &depthStencilDesc, dsvHandle);

	return TRUE;
}

BOOL CD3D12Renderer::UpdateWindowSize(DWORD dwBackBufferWidth, DWORD dwBackBufferHeight)
{
	BOOL	bResult = FALSE;

	if (!(dwBackBufferWidth * dwBackBufferHeight))
		return FALSE;

	if (m_dwWidth == dwBackBufferWidth && m_dwHeight == dwBackBufferHeight)
		return FALSE;

	// wait for all commands
	Fence();

	for (DWORD i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
	{
		WaitForFenceValue(m_pui64LastFenceValue[i]);
	}

	DXGI_SWAP_CHAIN_DESC1	desc;
	HRESULT	hr = m_pSwapChain->GetDesc1(&desc);
	if (FAILED(hr))
		__debugbreak();

	for (UINT n = 0; n < SWAP_CHAIN_FRAME_COUNT; n++)
	{
		m_pRenderTargets[n]->Release();
		m_pRenderTargets[n] = nullptr;
	}

	if (m_pDepthStencil)
	{
		m_pDepthStencil->Release();
		m_pDepthStencil = nullptr;
	}

	if (FAILED(m_pSwapChain->ResizeBuffers(SWAP_CHAIN_FRAME_COUNT, dwBackBufferWidth, dwBackBufferHeight, DXGI_FORMAT_R8G8B8A8_UNORM, m_dwSwapChainFlags)))
	{
		__debugbreak();
	}


	m_uiRenderTargetIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	// Create frame resources.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());

	// Create a RTV for each frame.
	for (UINT n = 0; n < SWAP_CHAIN_FRAME_COUNT; n++)
	{
		m_pSwapChain->GetBuffer(n, IID_PPV_ARGS(&m_pRenderTargets[n]));
		m_pD3DDevice->CreateRenderTargetView(m_pRenderTargets[n], nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}

	CreateDepthStencil(dwBackBufferWidth, dwBackBufferHeight);

	m_dwWidth = dwBackBufferWidth;
	m_dwHeight = dwBackBufferHeight;
	m_Viewport.Width = (float)m_dwWidth;
	m_Viewport.Height = (float)m_dwHeight;
	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.right = m_dwWidth;
	m_ScissorRect.bottom = m_dwHeight;

	InitCamera();

	return TRUE;
}
void CD3D12Renderer::BeginRender()
{
	//
	// 화면 클리어 및 이번 프레임 렌더링을 위한 자료구조 초기화
	//
	ID3D12CommandAllocator* pCommandAllocator = m_ppCommandAllocator[m_dwCurContextIndex];
	ID3D12GraphicsCommandList* pCommandList = m_ppCommandList[m_dwCurContextIndex];

	if (FAILED(pCommandAllocator->Reset()))
		__debugbreak();

	if (FAILED(pCommandList->Reset(pCommandAllocator, nullptr)))
		__debugbreak();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart(), m_uiRenderTargetIndex, m_rtvDescriptorSize);

	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_uiRenderTargetIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_pDSVHeap->GetCPUDescriptorHandleForHeapStart());

	// Record commands.
	const float BackColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	pCommandList->ClearRenderTargetView(rtvHandle, BackColor, 0, nullptr);
	pCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	pCommandList->RSSetViewports(1, &m_Viewport);
	pCommandList->RSSetScissorRects(1, &m_ScissorRect);
	pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
}

void CD3D12Renderer::RenderMeshObject(void* pMeshObjHandle, const XMMATRIX* pMatWorld)
{
	ID3D12GraphicsCommandList* pCommandList = m_ppCommandList[m_dwCurContextIndex];

	D3D12_CPU_DESCRIPTOR_HANDLE srv = {};
	CBasicMeshObject* pMeshObj = (CBasicMeshObject*)pMeshObjHandle;
	pMeshObj->Draw(pCommandList, pMatWorld);
}
void CD3D12Renderer::RenderSpriteWithTex(void* pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY, const RECT* pRect, float Z, void* pTexHandle)
{
	ID3D12GraphicsCommandList* pCommandList = m_ppCommandList[m_dwCurContextIndex];
	TEXTURE_HANDLE* pTexureHandle = (TEXTURE_HANDLE*)pTexHandle;

	CSpriteObject* pSpriteObj = (CSpriteObject*)pSprObjHandle;

	XMFLOAT2 Pos = { (float)iPosX, (float)iPosY };
	XMFLOAT2 Scale = { fScaleX, fScaleY };

	if (pTexureHandle->pUploadBuffer)
	{
		if (pTexureHandle->bUpdated)
		{
			UpdateTexture(m_pD3DDevice, pCommandList, pTexureHandle->pTexResource, pTexureHandle->pUploadBuffer);
		}
		else
		{
			int a = 0;
		}
		pTexureHandle->bUpdated = FALSE;
	}
	pSpriteObj->DrawWithTex(pCommandList, &Pos, &Scale, pRect, Z, pTexureHandle);
}
void CD3D12Renderer::RenderSprite(void* pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY, float Z)
{
	ID3D12GraphicsCommandList* pCommandList = m_ppCommandList[m_dwCurContextIndex];

	CSpriteObject* pSpriteObj = (CSpriteObject*)pSprObjHandle;

	XMFLOAT2 Pos = { (float)iPosX, (float)iPosY };
	XMFLOAT2 Scale = { fScaleX, fScaleY };
	pSpriteObj->Draw(pCommandList, &Pos, &Scale, Z);
}
void CD3D12Renderer::UpdateTextureWithImage(void* pTexHandle, const BYTE* pSrcBits, UINT SrcWidth, UINT SrcHeight)
{
	TEXTURE_HANDLE* pTextureHandle = (TEXTURE_HANDLE*)pTexHandle;
	ID3D12Resource* pDestTexResource = pTextureHandle->pTexResource;
	ID3D12Resource* pUploadBuffer = pTextureHandle->pUploadBuffer;

	D3D12_RESOURCE_DESC Desc = pDestTexResource->GetDesc();
	if (SrcWidth > Desc.Width)
	{
		__debugbreak();
	}
	if (SrcHeight > Desc.Height)
	{
		__debugbreak();
	}
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint;
	UINT	Rows = 0;
	UINT64	RowSize = 0;
	UINT64	TotalBytes = 0;

	m_pD3DDevice->GetCopyableFootprints(&Desc, 0, 1, 0, &Footprint, &Rows, &RowSize, &TotalBytes);

	BYTE*	pMappedPtr = nullptr;
	CD3DX12_RANGE readRange(0, 0);

	HRESULT hr = pUploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pMappedPtr));
	if (FAILED(hr))
		__debugbreak();

	const BYTE* pSrc = pSrcBits;
	BYTE* pDest = pMappedPtr;
	for (UINT y = 0; y < SrcHeight; y++)
	{
		memcpy(pDest, pSrc, SrcWidth * 4);
		pSrc += (SrcWidth * 4);
		pDest += Footprint.Footprint.RowPitch;
	}
	// Unmap
	pUploadBuffer->Unmap(0, nullptr);

	pTextureHandle->bUpdated = TRUE;
}

void CD3D12Renderer::EndRender()
{
	ID3D12GraphicsCommandList* pCommandList = m_ppCommandList[m_dwCurContextIndex];

	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_uiRenderTargetIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	pCommandList->Close();

	ID3D12CommandList* ppCommandLists[] = { pCommandList };
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void CD3D12Renderer::Present()
{
	Fence();

	//
	// Back Buffer 화면을 Primary Buffer로 전송
	//
	//UINT m_SyncInterval = 1;	// VSync On
	UINT m_SyncInterval = 0;	// VSync Off

	UINT uiSyncInterval = m_SyncInterval;
	UINT uiPresentFlags = 0;

	if (!uiSyncInterval)
	{
		uiPresentFlags = DXGI_PRESENT_ALLOW_TEARING;
	}

	HRESULT hr = m_pSwapChain->Present(uiSyncInterval, uiPresentFlags);

	if (DXGI_ERROR_DEVICE_REMOVED == hr)
	{
		__debugbreak();
	}

	m_uiRenderTargetIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	// prepare next frame
	DWORD	dwNextContextIndex = (m_dwCurContextIndex + 1) % MAX_PENDING_FRAME_COUNT;
	WaitForFenceValue(m_pui64LastFenceValue[dwNextContextIndex]);

	// reset resources per frame
	m_ppConstBufferManager[dwNextContextIndex]->Reset();
	m_ppDescriptorPool[dwNextContextIndex]->Reset();
	m_dwCurContextIndex = dwNextContextIndex;
}

void* CD3D12Renderer::CreateBasicMeshObject()
{
	CBasicMeshObject* pMeshObj = new CBasicMeshObject;
	pMeshObj->Initialize(this);
	return pMeshObj;
}
BOOL CD3D12Renderer::BeginCreateMesh(void* pMeshObjHandle, const BasicVertex* pVertexList, DWORD dwVertexCount, DWORD dwTriGroupCount)
{
	CBasicMeshObject* pMeshObj = (CBasicMeshObject*)pMeshObjHandle;
	BOOL bResult = pMeshObj->BeginCreateMesh(pVertexList, dwVertexCount, dwTriGroupCount);
	return bResult;
}
BOOL CD3D12Renderer::InsertTriGroup(void* pMeshObjHandle, const WORD* pIndexList, DWORD dwTriCount, const WCHAR* wchTexFileName)
{
	CBasicMeshObject* pMeshObj = (CBasicMeshObject*)pMeshObjHandle;
	BOOL bResult = pMeshObj->InsertIndexedTriList(pIndexList, dwTriCount, wchTexFileName);
	return bResult;
}
void CD3D12Renderer::EndCreateMesh(void* pMeshObjHandle)
{
	CBasicMeshObject* pMeshObj = (CBasicMeshObject*)pMeshObjHandle;
	pMeshObj->EndCreateMesh();
}
void CD3D12Renderer::DeleteBasicMeshObject(void* pMeshObjHandle)
{
	// wait for all commands
	for (DWORD i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
	{
		WaitForFenceValue(m_pui64LastFenceValue[i]);
	}

	CBasicMeshObject* pMeshObj = (CBasicMeshObject*)pMeshObjHandle;
	delete pMeshObj;
}

void* CD3D12Renderer::CreateSpriteObject()
{
	CSpriteObject* pSprObj = new CSpriteObject;
	pSprObj->Initialize(this);

	return (void*)pSprObj;
}
void* CD3D12Renderer::CreateSpriteObject(const WCHAR* wchTexFileName, int PosX, int PosY, int Width, int Height)
{
	CSpriteObject* pSprObj = new CSpriteObject;

	RECT rect;
	rect.left = PosX;
	rect.top = PosY;
	rect.right = Width;
	rect.bottom = Height;
	pSprObj->Initialize(this, wchTexFileName, &rect);

	return (void*)pSprObj;
}
void CD3D12Renderer::DeleteSpriteObject(void* pSpriteObjHandle)
{
	// wait for all commands
	for (DWORD i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
	{
		WaitForFenceValue(m_pui64LastFenceValue[i]);
	}

	CSpriteObject* pSprObj = (CSpriteObject*)pSpriteObjHandle;
	delete pSprObj;
}


UINT64 CD3D12Renderer::Fence()
{
	m_ui64FenceVaule++;
	m_pCommandQueue->Signal(m_pFence, m_ui64FenceVaule);
	m_pui64LastFenceValue[m_dwCurContextIndex] = m_ui64FenceVaule;
	return m_ui64FenceVaule;
}
void CD3D12Renderer::WaitForFenceValue(UINT64 ExpectedFenceValue)
{
	// Wait until the previous frame is finished.
	if (m_pFence->GetCompletedValue() < ExpectedFenceValue)
	{
		m_pFence->SetEventOnCompletion(ExpectedFenceValue, m_hFenceEvent);
		WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void* CD3D12Renderer::CreateDynamicTexture(UINT TexWidth, UINT TexHeight)
{
	TEXTURE_HANDLE* pTexHandle = m_pTextureManager->CreateDynamicTexture(TexWidth, TexHeight);
	return pTexHandle;
}
void* CD3D12Renderer::CreateTiledTexture(UINT TexWidth, UINT TexHeight, DWORD r, DWORD g, DWORD b)
{
	DXGI_FORMAT TexFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	BYTE* pImage = (BYTE*)malloc(TexWidth * TexHeight * 4);
	memset(pImage, 0, TexWidth * TexHeight * 4);

	BOOL bFirstColorIsWhite = TRUE;

	for (UINT y = 0; y < TexHeight; y++)
	{
		for (UINT x = 0; x < TexWidth; x++)
		{

			RGBA* pDest = (RGBA*)(pImage + (x + y * TexWidth) * 4);

			if ((bFirstColorIsWhite + x) % 2)
			{
				pDest->r = r;
				pDest->g = g;
				pDest->b = b;
			}
			else
			{
				pDest->r = 0;
				pDest->g = 0;
				pDest->b = 0;
			}
			pDest->a = 255;
		}
		bFirstColorIsWhite++;
		bFirstColorIsWhite %= 2;
	}
	TEXTURE_HANDLE* pTexHandle = m_pTextureManager->CreateImmutableTexture(TexWidth, TexHeight, TexFormat, pImage);

	free(pImage);
	pImage = nullptr;

	return pTexHandle;
}
void* CD3D12Renderer::CreateTextureFromFile(const WCHAR* wchFileName)
{
	TEXTURE_HANDLE* pTexHandle = m_pTextureManager->CreateTextureFromFile(wchFileName);
	return pTexHandle;
}
void CD3D12Renderer::DeleteTexture(void* pTexHandle)
{
	// wait for all commands
	for (DWORD i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
	{
		WaitForFenceValue(m_pui64LastFenceValue[i]);
	}
	m_pTextureManager->DeleteTexture((TEXTURE_HANDLE*)pTexHandle);
}
void* CD3D12Renderer::CreateFontObject(const WCHAR* wchFontFamilyName, float fFontSize)
{
	FONT_HANDLE* pFontHandle = m_pFontManager->CreateFontObject(wchFontFamilyName, fFontSize);
	return pFontHandle;
}
void CD3D12Renderer::DeleteFontObject(void* pFontHandle)
{
	m_pFontManager->DeleteFontObject((FONT_HANDLE*)pFontHandle);
}

BOOL CD3D12Renderer::WriteTextToBitmap(BYTE* pDestImage, UINT DestWidth, UINT DestHeight, UINT DestPitch, int* piOutWidth, int* piOutHeight, void* pFontObjHandle, const WCHAR* wchString, DWORD dwLen)
{
	BOOL	bResult = m_pFontManager->WriteTextToBitmap(pDestImage, DestWidth, DestHeight, DestPitch, piOutWidth, piOutHeight, (FONT_HANDLE*)pFontObjHandle, wchString, dwLen);
	return bResult;
}

void CD3D12Renderer::CreateCommandList()
{
	for (DWORD i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
	{
		ID3D12CommandAllocator* pCommandAllocator = nullptr;
		ID3D12GraphicsCommandList* pCommandList = nullptr;

		if (FAILED(m_pD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator))))
		{
			__debugbreak();
		}

		// Create the command list.
		if (FAILED(m_pD3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator, nullptr, IID_PPV_ARGS(&pCommandList))))
		{
			__debugbreak();
		}

		// Command lists are created in the recording state, but there is nothing
		// to record yet. The main loop expects it to be closed, so close it now.
		pCommandList->Close();

		m_ppCommandAllocator[i] = pCommandAllocator;
		m_ppCommandList[i] = pCommandList;
	}
}
void CD3D12Renderer::CleanupCommandList()
{
	for (DWORD i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
	{
		ID3D12CommandAllocator* pCommandAllocator = m_ppCommandAllocator[i];
		ID3D12GraphicsCommandList* pCommandList = m_ppCommandList[i];

		if (pCommandList)
		{
			pCommandList->Release();
			pCommandList = nullptr;
		}
		if (pCommandAllocator)
		{
			pCommandAllocator->Release();
			pCommandAllocator = nullptr;
		}
		m_ppCommandAllocator[i] = nullptr;
		m_ppCommandList[i] = nullptr;
	}
}
void CD3D12Renderer::CreateFence()
{
	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	if (FAILED(m_pD3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence))))
	{
		__debugbreak();
	}

	// Create an event handle to use for frame synchronization.
	m_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}
void CD3D12Renderer::CleanupFence()
{
	if (m_hFenceEvent)
	{
		CloseHandle(m_hFenceEvent);
		m_hFenceEvent = nullptr;
	}
	if (m_pFence)
	{
		m_pFence->Release();
		m_pFence = nullptr;
	}
}
BOOL CD3D12Renderer::CreateDescriptorHeapForRTV()
{
	HRESULT hr = S_OK;

	// 렌더타겟용 디스크립터힙
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = SWAP_CHAIN_FRAME_COUNT;	// SwapChain Buffer 0	| SwapChain Buffer 1
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(m_pD3DDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRTVHeap))))
	{
		__debugbreak();
	}

	m_rtvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	return TRUE;
}

BOOL CD3D12Renderer::CreateDescriptorHeapForDSV()
{
	HRESULT hr = S_OK;

	// Describe and create a depth stencil view (DSV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;	// Default Depth Buffer
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(m_pD3DDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDSVHeap))))
	{
		__debugbreak();
	}

	m_dsvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	return TRUE;
}

void CD3D12Renderer::CleanupDescriptorHeapForRTV()
{
	if (m_pRTVHeap)
	{
		m_pRTVHeap->Release();
		m_pRTVHeap = nullptr;
	}
}


void CD3D12Renderer::CleanupDescriptorHeapForDSV()
{
	if (m_pDSVHeap)
	{
		m_pDSVHeap->Release();
		m_pDSVHeap = nullptr;
	}
}

void CD3D12Renderer::Cleanup()
{
	Fence();

	for (DWORD i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
	{
		WaitForFenceValue(m_pui64LastFenceValue[i]);
	}
	for (DWORD i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
	{
		if (m_ppConstBufferManager[i])
		{
			delete m_ppConstBufferManager[i];
			m_ppConstBufferManager[i] = nullptr;
		}
		if (m_ppDescriptorPool[i])
		{
			delete m_ppDescriptorPool[i];
			m_ppDescriptorPool[i] = nullptr;
		}
	}
	if (m_pTextureManager)
	{
		delete m_pTextureManager;
		m_pTextureManager = nullptr;
	}
	if (m_pResourceManager)
	{
		delete m_pResourceManager;
		m_pResourceManager = nullptr;
	}
	if (m_pFontManager)
	{
		delete m_pFontManager;
		m_pFontManager = nullptr;
	}
	if (m_pSingleDescriptorAllocator)
	{
		delete m_pSingleDescriptorAllocator;
		m_pSingleDescriptorAllocator = nullptr;
	}

	CleanupDescriptorHeapForRTV();
	CleanupDescriptorHeapForDSV();

	for (DWORD i = 0; i < SWAP_CHAIN_FRAME_COUNT; i++)
	{
		if (m_pRenderTargets[i])
		{
			m_pRenderTargets[i]->Release();
			m_pRenderTargets[i] = nullptr;
		}
	}
	if (m_pDepthStencil)
	{
		m_pDepthStencil->Release();
		m_pDepthStencil = nullptr;
	}
	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = nullptr;
	}

	if (m_pCommandQueue)
	{
		m_pCommandQueue->Release();
		m_pCommandQueue = nullptr;
	}

	CleanupCommandList();

	CleanupFence();

	if (m_pD3DDevice)
	{
		ULONG ref_count = m_pD3DDevice->Release();
		if (ref_count)
		{
			//resource leak!!!
			IDXGIDebug1* pDebug = nullptr;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
			{
				pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
				pDebug->Release();
			}
			__debugbreak();
		}

		m_pD3DDevice = nullptr;

	}
}
CD3D12Renderer::~CD3D12Renderer()
{
	Cleanup();
}