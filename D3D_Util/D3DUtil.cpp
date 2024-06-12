#include "pch.h"
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dx12.h>
#include "D3DUtil.h"



void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	IDXGIAdapter1* adapter = nullptr;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter;
}
void GetSoftwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	IDXGIAdapter1* adapter = nullptr;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr)))
			{
				*ppAdapter = adapter;
				break;
			}
		}
	}
}

void SetDebugLayerInfo(ID3D12Device* pD3DDevice)
{
	ID3D12InfoQueue*	pInfoQueue = nullptr;
	pD3DDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
	if (pInfoQueue)
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

		D3D12_MESSAGE_ID hide[] =
		{
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
			// Workarounds for debug layer issues on hybrid-graphics systems
			D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = (UINT)_countof(hide);
		filter.DenyList.pIDList = hide;
		pInfoQueue->AddStorageFilterEntries(&filter);

		pInfoQueue->Release();
		pInfoQueue = nullptr;
	}
}

void SetDefaultSamplerDesc(D3D12_STATIC_SAMPLER_DESC* pOutSamperDesc, UINT RegisterIndex)
{
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	//pOutSamperDesc->Filter = D3D12_FILTER_ANISOTROPIC;
	pOutSamperDesc->Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

	pOutSamperDesc->AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pOutSamperDesc->AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pOutSamperDesc->AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pOutSamperDesc->MipLODBias = 0.0f;
	pOutSamperDesc->MaxAnisotropy = 16;
	pOutSamperDesc->ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	pOutSamperDesc->BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	pOutSamperDesc->MinLOD = -FLT_MAX;
	pOutSamperDesc->MaxLOD = D3D12_FLOAT32_MAX;
	pOutSamperDesc->ShaderRegister = RegisterIndex;
	pOutSamperDesc->RegisterSpace = 0;
	pOutSamperDesc->ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}

HRESULT CreateVertexBuffer(ID3D12Device* pDevice, UINT SizePerVertex, DWORD dwVertexNum, D3D12_VERTEX_BUFFER_VIEW* pOutVertexBufferView, ID3D12Resource **ppOutBuffer)
{
	HRESULT hr = S_OK;

	D3D12_VERTEX_BUFFER_VIEW	VertexBufferView = {};
	ID3D12Resource*	pVertexBuffer = nullptr;
	UINT		VertexBufferSize = SizePerVertex * dwVertexNum;

	// create vertexbuffer for rendering
	hr = pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&pVertexBuffer));

	if (FAILED(hr))
	{
		goto lb_return;
	}

	// Initialize the vertex buffer view.
	VertexBufferView.BufferLocation = pVertexBuffer->GetGPUVirtualAddress();
	VertexBufferView.StrideInBytes = SizePerVertex;
	VertexBufferView.SizeInBytes = VertexBufferSize;

	*pOutVertexBufferView = VertexBufferView;
	*ppOutBuffer = pVertexBuffer;

lb_return:
	return hr;
}


void UpdateTexture(ID3D12Device* pD3DDevice, ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pDestTexResource, ID3D12Resource* pSrcTexResource)
{
	const DWORD MAX_SUB_RESOURCE_NUM = 32;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint[MAX_SUB_RESOURCE_NUM] = {};
	UINT	Rows[MAX_SUB_RESOURCE_NUM] = {};
	UINT64	RowSize[MAX_SUB_RESOURCE_NUM] = {};
	UINT64	TotalBytes = 0;

	D3D12_RESOURCE_DESC Desc = pDestTexResource->GetDesc();
	if (Desc.MipLevels > (UINT)_countof(Footprint))
		__debugbreak();

	pD3DDevice->GetCopyableFootprints(&Desc, 0, Desc.MipLevels, 0, Footprint, Rows, RowSize, &TotalBytes);

	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDestTexResource, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
	for (DWORD i = 0; i < Desc.MipLevels; i++)
	{

		D3D12_TEXTURE_COPY_LOCATION	destLocation = {};
		destLocation.PlacedFootprint = Footprint[i];
		destLocation.pResource = pDestTexResource;
		destLocation.SubresourceIndex = i;
		destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION	srcLocation = {};
		srcLocation.PlacedFootprint = Footprint[i];
		srcLocation.pResource = pSrcTexResource;
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

		pCommandList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);
	}
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDestTexResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));

}