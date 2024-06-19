#include "pch.h"
#include <d3d12.h>
#include <d3dx12.h>
#include "D3D12Renderer.h"
#include "SimpleConstantBufferPool.h"


CSimpleConstantBufferPool::CSimpleConstantBufferPool()
{
}


BOOL CSimpleConstantBufferPool::Initialize(ID3D12Device* pD3DDevice, CONSTANT_BUFFER_TYPE type, UINT SizePerCBV, UINT MaxCBVNum)
{
	m_ConstantBufferType = type;
	m_MaxCBVNum = MaxCBVNum;
	m_SizePerCBV = SizePerCBV;
	UINT ByteWidth = SizePerCBV * m_MaxCBVNum;

	// Create the constant buffer.
	if (FAILED(pD3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(ByteWidth),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pResource))))
	{
		__debugbreak();
	}


	// create descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = m_MaxCBVNum;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(pD3DDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pCBVHeap))))
	{
		__debugbreak();
	}
	CD3DX12_RANGE writeRange(0, 0);		// We do not intend to write from this resource on the CPU.
	m_pResource->Map(0, &writeRange, reinterpret_cast<void**>(&m_pSystemMemAddr));


	m_pCBContainerList = new CB_CONTAINER[m_MaxCBVNum];



	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_pResource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = m_SizePerCBV;

	UINT8*	pSystemMemPtr = m_pSystemMemAddr;
	CD3DX12_CPU_DESCRIPTOR_HANDLE	heapHandle(m_pCBVHeap->GetCPUDescriptorHandleForHeapStart());

	UINT	DescriptorSize = pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (DWORD i = 0; i < m_MaxCBVNum; i++)
	{
		pD3DDevice->CreateConstantBufferView(&cbvDesc, heapHandle);

		m_pCBContainerList[i].CBVHandle = heapHandle;
		m_pCBContainerList[i].pGPUMemAddr = cbvDesc.BufferLocation;
		m_pCBContainerList[i].pSystemMemAddr = pSystemMemPtr;

		heapHandle.Offset(1, DescriptorSize);
		cbvDesc.BufferLocation += m_SizePerCBV;
		pSystemMemPtr += m_SizePerCBV;
	}
	return TRUE;

}



CB_CONTAINER* CSimpleConstantBufferPool::Alloc()
{
	CB_CONTAINER*	pCB = nullptr;

	if (m_AllocatedCBVNum >= m_MaxCBVNum)
		goto lb_return;

	pCB = m_pCBContainerList + m_AllocatedCBVNum;
	m_AllocatedCBVNum++;
lb_return:
	return pCB;
}
void CSimpleConstantBufferPool::Reset()
{
	m_AllocatedCBVNum = 0;
}
void CSimpleConstantBufferPool::Cleanup()
{
	if (m_pCBContainerList)
	{

		delete[] m_pCBContainerList;
		m_pCBContainerList = nullptr;
	}
	if (m_pResource)
	{
		m_pResource->Release();
		m_pResource = nullptr;
	}
	if (m_pCBVHeap)
	{
		m_pCBVHeap->Release();
		m_pCBVHeap = nullptr;
	}
}
CSimpleConstantBufferPool::~CSimpleConstantBufferPool()
{
	Cleanup();

}
