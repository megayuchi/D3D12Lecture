#pragma once

#include <d3d12.h>
#include "../Common/typedef.h"

class SimpleConstantBufferPool;

class CConstantBufferManager
{
	CSimpleConstantBufferPool* m_ppConstantBufferPool[CONSTANT_BUFFER_TYPE_COUNT] = {};
public:
	BOOL	Initialize(ID3D12Device* pD3DDevice, DWORD dwMaxCBVNum);
	void	Reset();
	CSimpleConstantBufferPool*	GetConstantBufferPool(CONSTANT_BUFFER_TYPE);
	CConstantBufferManager();
	~CConstantBufferManager();
};