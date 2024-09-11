#include "pch.h"
#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <dxgidebug.h>
#include <d3dx12.h>
#include "../D3D_Util/D3DUtil.h"
#include "DescriptorPool.h"


CDescriptorPool::CDescriptorPool()
{

}
BOOL CDescriptorPool::Initialize(ID3D12Device5* pD3DDevice, UINT MaxDescriptorCount)
{

	BOOL bResult = FALSE;
	m_pD3DDevice = pD3DDevice;
	
	m_MaxDescriptorCount = MaxDescriptorCount;
	m_srvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// create descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC commonHeapDesc = {};
	commonHeapDesc.NumDescriptors = m_MaxDescriptorCount;
	commonHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	commonHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (FAILED(m_pD3DDevice->CreateDescriptorHeap(&commonHeapDesc, IID_PPV_ARGS(&m_pDescritorHeap))))
	{
		__debugbreak();
		goto lb_return;
	}
	m_cpuDescriptorHandle = m_pDescritorHeap->GetCPUDescriptorHandleForHeapStart();
	m_gpuDescriptorHandle = m_pDescritorHeap->GetGPUDescriptorHandleForHeapStart();
	bResult = TRUE;
lb_return:
	return bResult;

}
BOOL CDescriptorPool::AllocDescriptorTable(D3D12_CPU_DESCRIPTOR_HANDLE* pOutCPUDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE* pOutGPUDescriptor, UINT DescriptorCount)
{
	BOOL bResult = FALSE;
	if (m_AllocatedDescriptorCount + DescriptorCount > m_MaxDescriptorCount)
	{
		goto lb_return;
	}
	UINT offset = m_AllocatedDescriptorCount + DescriptorCount;

	*pOutCPUDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cpuDescriptorHandle, m_AllocatedDescriptorCount, m_srvDescriptorSize);
	*pOutGPUDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_gpuDescriptorHandle, m_AllocatedDescriptorCount, m_srvDescriptorSize);
	m_AllocatedDescriptorCount += DescriptorCount;
	bResult = TRUE;
lb_return:
	return bResult;
}
void CDescriptorPool::Reset()
{
	m_AllocatedDescriptorCount = 0;
}

void CDescriptorPool::Cleanup()
{
	if (m_pDescritorHeap)
	{
		m_pDescritorHeap->Release();
		m_pDescritorHeap = nullptr;
	}
}
CDescriptorPool::~CDescriptorPool()
{
	Cleanup();
}