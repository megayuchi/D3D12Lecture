#pragma once

class CHashTable;
class CD3D12Renderer;
class CD3D12ResourceManager;
class CTextureManager
{
	CD3D12Renderer*	m_pRenderer = nullptr;
	CD3D12ResourceManager* m_pResourceManager = nullptr;
	CHashTable*	m_pHashTable = nullptr;
	
	SORT_LINK*	m_pTexLinkHead = nullptr;
	SORT_LINK*	m_pTexLinkTail = nullptr;

	TEXTURE_HANDLE* AllocTextureHandle();
	DWORD	FreeTextureHandle(TEXTURE_HANDLE* pTexHandle);
	void	Cleanup();
public:
	BOOL	Initialize(CD3D12Renderer* pRenderer, DWORD dwMaxBucketNum, DWORD dwMaxFileNum);
	TEXTURE_HANDLE* CreateTextureFromFile(const WCHAR* wchFileName);
	TEXTURE_HANDLE* CreateDynamicTexture(UINT TexWidth, UINT TexHeight);
	TEXTURE_HANDLE* CreateImmutableTexture(UINT TexWidth, UINT TexHeight, DXGI_FORMAT format, const BYTE* pInitImage);
	void	DeleteTexture(TEXTURE_HANDLE* pTexHandle);
	CTextureManager();
	~CTextureManager();
};