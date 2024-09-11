#include "pch.h"
#include <d3d12.h>
#include <d3dx12.h>
#include "SingleDescriptorAllocator.h"
#include "../Util/IndexCreator.h"

CSingleDescriptorAllocator::CSingleDescriptorAllocator()
{
}

BOOL CSingleDescriptorAllocator::Initialize(ID3D12Device* pDevice, DWORD dwMaxCount, D3D12_DESCRIPTOR_HEAP_FLAGS Flags)
{
	m_pD3DDevice = pDevice;
	m_pD3DDevice->AddRef();

	//D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	D3D12_DESCRIPTOR_HEAP_DESC commonHeapDesc = {};
	commonHeapDesc.NumDescriptors = dwMaxCount;
	commonHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	commonHeapDesc.Flags = Flags;

	if (FAILED(m_pD3DDevice->CreateDescriptorHeap(&commonHeapDesc, IID_PPV_ARGS(&m_pHeap))))
	{
		__debugbreak();
	}

	m_IndexCreator.Initialize(dwMaxCount);

	m_DescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//CD3DX12_CPU_DESCRIPTOR_HANDLE commonHeapHandle(m_pHeap->GetCPUDescriptorHandleForHeapStart());

	//for (DWORD i=0; i<dwMaxCount; i++)
	//{
	//	commonHeapHandle.Offset(1, m_DescriptorSize);	
	//}


	return TRUE;
}
BOOL CSingleDescriptorAllocator::AllocDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE* pOutCPUHandle)
{
	BOOL	bResult = FALSE;

	DWORD	dwIndex = m_IndexCreator.Alloc();
	if (-1 != dwIndex)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHandle(m_pHeap->GetCPUDescriptorHandleForHeapStart(), dwIndex, m_DescriptorSize);
		*pOutCPUHandle = DescriptorHandle;
		bResult = TRUE;
	}
	return bResult;
}

BOOL CSingleDescriptorAllocator::Check(D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle)
{
	BOOL	bResult = TRUE;
	D3D12_CPU_DESCRIPTOR_HANDLE base = m_pHeap->GetCPUDescriptorHandleForHeapStart();
	if (base.ptr > DescriptorHandle.ptr)
	{
		__debugbreak();
		bResult = FALSE;
	}
	return bResult;
}
void CSingleDescriptorAllocator::FreeDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle)
{

	D3D12_CPU_DESCRIPTOR_HANDLE base = m_pHeap->GetCPUDescriptorHandleForHeapStart();
#ifdef _DEBUG
	if (base.ptr > DescriptorHandle.ptr)
		__debugbreak();
#endif
	DWORD dwIndex = (DWORD)(DescriptorHandle.ptr - base.ptr) / m_DescriptorSize;
	m_IndexCreator.Free(dwIndex);
}
D3D12_GPU_DESCRIPTOR_HANDLE CSingleDescriptorAllocator::GetGPUHandleFromCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
{
	BOOL	bResult = FALSE;

	D3D12_CPU_DESCRIPTOR_HANDLE base = m_pHeap->GetCPUDescriptorHandleForHeapStart();
#ifdef _DEBUG
	if (base.ptr > cpuHandle.ptr)
		__debugbreak();
#endif
	DWORD dwIndex = (DWORD)(cpuHandle.ptr - base.ptr) / m_DescriptorSize;
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_pHeap->GetGPUDescriptorHandleForHeapStart(), dwIndex, m_DescriptorSize);

	return gpuHandle;
}

void CSingleDescriptorAllocator::Cleanup()
{
#ifdef _DEBUG
	m_IndexCreator.Check();
#endif
	if (m_pHeap)
	{
		m_pHeap->Release();
		m_pHeap = nullptr;
	}
	if (m_pD3DDevice)
	{
		m_pD3DDevice->Release();
		m_pD3DDevice = nullptr;
	}
}


CSingleDescriptorAllocator::~CSingleDescriptorAllocator()
{
	Cleanup();
}
