#include "pch.h"
#include "typedef.h"
#include "SimpleConstantBufferPool.h"
#include "ConstantBufferManager.h"

CONSTANT_BUFFER_PROPERTY g_pConstBufferPropList[] =
{
	CONSTANT_BUFFER_TYPE_DEFAULT, sizeof(CONSTANT_BUFFER_DEFAULT),
	CONSTANT_BUFFER_TYPE_SPRITE, sizeof(CONSTANT_BUFFER_SPRITE)
};

CConstantBufferManager::CConstantBufferManager()
{


}
BOOL CConstantBufferManager::Initialize(ID3D12Device* pD3DDevice, DWORD dwMaxCBVNum)
{
	for (DWORD i = 0; i < CONSTANT_BUFFER_TYPE_COUNT; i++)
	{
		m_ppConstantBufferPool[i] = new CSimpleConstantBufferPool;
		m_ppConstantBufferPool[i]->Initialize(pD3DDevice, (CONSTANT_BUFFER_TYPE)i, AlignConstantBufferSize(g_pConstBufferPropList[i].Size), dwMaxCBVNum);
	}
	return TRUE;
}
void CConstantBufferManager::Reset()
{
	for (DWORD i = 0; i < CONSTANT_BUFFER_TYPE_COUNT; i++)
	{
		m_ppConstantBufferPool[i]->Reset();
	}
}
CSimpleConstantBufferPool* CConstantBufferManager::GetConstantBufferPool(CONSTANT_BUFFER_TYPE type)
{
	if (type >= CONSTANT_BUFFER_TYPE_COUNT)
		__debugbreak();

	return m_ppConstantBufferPool[type];
}
CConstantBufferManager::~CConstantBufferManager()
{
	for (DWORD i = 0; i < CONSTANT_BUFFER_TYPE_COUNT; i++)
	{
		if (m_ppConstantBufferPool[i])
		{
			delete m_ppConstantBufferPool[i];
			m_ppConstantBufferPool[i] = nullptr;
		}
	}
}


