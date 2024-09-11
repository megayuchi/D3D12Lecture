#include "pch.h"
#include <Windows.h>
#include <d3d12.h>
#include "../Common/typedef.h"
#include "Renderer_typedef.h"
#include "../Util/HashTable.h"
#include "SingleDescriptorAllocator.h"
#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "TextureManager.h"

CTextureManager::CTextureManager()
{
}
BOOL CTextureManager::Initialize(CD3D12Renderer* pRenderer, DWORD dwMaxBucketNum, DWORD dwMaxFileNum)
{
	m_pRenderer = pRenderer;
	m_pResourceManager = pRenderer->INL_GetResourceManager();

	m_pHashTable = new CHashTable;
	m_pHashTable->Initialize(dwMaxBucketNum, _MAX_PATH, dwMaxFileNum);

	return TRUE;
}
TEXTURE_HANDLE* CTextureManager::CreateTextureFromFile(const WCHAR* wchFileName)
{
	ID3D12Device* pD3DDevice = m_pRenderer->INL_GetD3DDevice();
	CSingleDescriptorAllocator* pSingleDescriptorAllocator = m_pRenderer->INL_GetSingleDescriptorAllocator();
	
	ID3D12Resource* pTexResource = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE srv = {};
	D3D12_RESOURCE_DESC	desc = {};
	TEXTURE_HANDLE* pTexHandle = nullptr;

	DWORD dwFileNameLen = (DWORD)wcslen(wchFileName);
	DWORD dwKeySize = dwFileNameLen * sizeof(WCHAR);
	if (m_pHashTable->Select((void**)&pTexHandle, 1, wchFileName, dwKeySize))
	{
		pTexHandle->dwRefCount++;
	}
	else
	{
		if (m_pResourceManager->CreateTextureFromFile(&pTexResource, &desc, wchFileName))
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.Format = desc.Format;
			SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = desc.MipLevels;

			if (pSingleDescriptorAllocator->AllocDescriptorHandle(&srv))
			{
				pD3DDevice->CreateShaderResourceView(pTexResource, &SRVDesc, srv);

				pTexHandle = AllocTextureHandle();
				pTexHandle->pTexResource = pTexResource;
				pTexHandle->bFromFile = TRUE;
				pTexHandle->srv = srv;

				pTexHandle->pSearchHandle = m_pHashTable->Insert((void*)pTexHandle, wchFileName, dwKeySize);
				if (!pTexHandle->pSearchHandle)
					__debugbreak();

			}
			else
			{
				pTexResource->Release();
				pTexResource = nullptr;
			}
		}
	}
	return pTexHandle;
}


TEXTURE_HANDLE* CTextureManager::CreateDynamicTexture(UINT TexWidth, UINT TexHeight)
{
	ID3D12Device* pD3DDevice = m_pRenderer->INL_GetD3DDevice();
	CSingleDescriptorAllocator* pSingleDescriptorAllocator = m_pRenderer->INL_GetSingleDescriptorAllocator();
	TEXTURE_HANDLE* pTexHandle = nullptr;

	ID3D12Resource* pTexResource = nullptr;
	ID3D12Resource* pUploadBuffer = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE srv = {};


	DXGI_FORMAT TexFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (m_pResourceManager->CreateTexturePair(&pTexResource, &pUploadBuffer, TexWidth, TexHeight, TexFormat))
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = TexFormat;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		if (pSingleDescriptorAllocator->AllocDescriptorHandle(&srv))
		{
			pD3DDevice->CreateShaderResourceView(pTexResource, &SRVDesc, srv);

			pTexHandle = AllocTextureHandle();
			pTexHandle->pTexResource = pTexResource;
			pTexHandle->pUploadBuffer = pUploadBuffer;
			pTexHandle->srv = srv;
		}
		else
		{
			pTexResource->Release();
			pTexResource = nullptr;

			pUploadBuffer->Release();
			pUploadBuffer = nullptr;
		}
	}

	return pTexHandle;
}
TEXTURE_HANDLE* CTextureManager::CreateImmutableTexture(UINT TexWidth, UINT TexHeight, DXGI_FORMAT format, const BYTE* pInitImage)
{
	ID3D12Device* pD3DDevice = m_pRenderer->INL_GetD3DDevice();
	CSingleDescriptorAllocator* pSingleDescriptorAllocator = m_pRenderer->INL_GetSingleDescriptorAllocator();
	TEXTURE_HANDLE* pTexHandle = nullptr;

	ID3D12Resource* pTexResource = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE srv = {};

	if (m_pResourceManager->CreateTexture(&pTexResource, TexWidth, TexHeight, format, pInitImage))
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = format;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		if (pSingleDescriptorAllocator->AllocDescriptorHandle(&srv))
		{
			pD3DDevice->CreateShaderResourceView(pTexResource, &SRVDesc, srv);

			pTexHandle = AllocTextureHandle();
			pTexHandle->pTexResource = pTexResource;
			pTexHandle->srv = srv;
		}
		else
		{
			pTexResource->Release();
			pTexResource = nullptr;
		}
	}

	return pTexHandle;
}
void CTextureManager::DeleteTexture(TEXTURE_HANDLE* pTexHandle)
{
	FreeTextureHandle(pTexHandle);
}
TEXTURE_HANDLE* CTextureManager::AllocTextureHandle()
{
	TEXTURE_HANDLE* pTexHandle = new TEXTURE_HANDLE;
	memset(pTexHandle, 0, sizeof(TEXTURE_HANDLE));
	pTexHandle->Link.pItem = pTexHandle;
	LinkToLinkedListFIFO(&m_pTexLinkHead, &m_pTexLinkTail, &pTexHandle->Link);
	pTexHandle->dwRefCount = 1;
	return pTexHandle;
}
DWORD CTextureManager::FreeTextureHandle(TEXTURE_HANDLE* pTexHandle)
{
	ID3D12Device* pD3DDevice = m_pRenderer->INL_GetD3DDevice();
	CSingleDescriptorAllocator* pSingleDescriptorAllocator = m_pRenderer->INL_GetSingleDescriptorAllocator();

	if (!pTexHandle->dwRefCount)
		__debugbreak();

	DWORD ref_count = --pTexHandle->dwRefCount;
	if (!ref_count)
	{
		if (pTexHandle->pTexResource)
		{
			pTexHandle->pTexResource->Release();
			pTexHandle->pTexResource = nullptr;
		}
		if (pTexHandle->pUploadBuffer)
		{
			pTexHandle->pUploadBuffer->Release();
			pTexHandle->pUploadBuffer = nullptr;
		}
		if (pTexHandle->srv.ptr)
		{
			pSingleDescriptorAllocator->FreeDescriptorHandle(pTexHandle->srv);
			pTexHandle->srv = {};
		}

		if (pTexHandle->pSearchHandle)
		{
			m_pHashTable->Delete(pTexHandle->pSearchHandle);
			pTexHandle->pSearchHandle = nullptr;
		}
		UnLinkFromLinkedList(&m_pTexLinkHead, &m_pTexLinkTail, &pTexHandle->Link);

		delete pTexHandle;
	}
	else
	{
		int a = 0;
	}
	return ref_count;
}

void CTextureManager::Cleanup()
{
	if (m_pTexLinkHead)
	{
		// texture resource leak!!!
		__debugbreak();
	}
	if (m_pHashTable)
	{
		delete m_pHashTable;
		m_pHashTable = nullptr;
	}
}
CTextureManager::~CTextureManager()
{
	Cleanup();
}