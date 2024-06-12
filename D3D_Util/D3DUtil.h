#pragma once

void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
void GetSoftwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
void SetDebugLayerInfo(ID3D12Device* pD3DDevice);
HRESULT CreateVertexBuffer(ID3D12Device* pDevice, UINT SizePerVertex, DWORD dwVertexNum, D3D12_VERTEX_BUFFER_VIEW* pOutVertexBufferView, ID3D12Resource **ppOutBuffer);
void SetDefaultSamplerDesc(D3D12_STATIC_SAMPLER_DESC* pOutSamperDesc, UINT RegisterIndex);

void UpdateTexture(ID3D12Device* pD3DDevice, ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pDestTexResource, ID3D12Resource* pSrcTexResource);
inline size_t AlignConstantBufferSize(size_t size)
{
	size_t aligned_size = (size + 255) & (~255);
	return aligned_size;
}