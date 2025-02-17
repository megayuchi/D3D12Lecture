#pragma once

struct BUCKET_DESC
{
	SORT_LINK*					pBucketLinkHead;
	SORT_LINK*					pBucketLinkTail;
	DWORD						dwLinkNum;
};

struct VB_BUCKET
{
	const void*			pItem;
	BUCKET_DESC*		pBucketDesc;
	SORT_LINK			sortLink;
	DWORD				dwSize;
	char				pKeyData[1];
};


class CHashTable
{

	BUCKET_DESC*	m_pBucketDescTable = nullptr;
	DWORD	m_dwMaxBucketNum = 0;
	DWORD	m_dwMaxKeyDataSize = 0;
	DWORD	m_dwItemNum = 0;

	DWORD		CreateKey(const void* pData, DWORD dwSize, DWORD dwBucketNum);

public:
	BOOL	Initialize(DWORD dwMaxBucketNum, DWORD dwMaxKeySize, DWORD dwMaxItemNum);
	DWORD	Select(void** ppOutItemList, DWORD dwMaxItemNum, const void* pKeyData, DWORD dwSize);
	void*	Insert(const void* pItem, const void* pKeyData, DWORD dwSize);
	void	Delete(const void* pSearchHandle);
	DWORD	GetMaxBucketNum() const;
	void	DeleteAll();
	DWORD	GetAllItems(void** ppOutItemList, DWORD dwMaxItemNum, BOOL* pbOutInsufficient) const;
	DWORD	GetKeyPtrAndSize(void** ppOutKeyPtr, const void* pSearchHandle) const;
	DWORD	GetItemNum() const;
	void	ResourceCheck() const;

	void	Cleanup();

	CHashTable();
	~CHashTable();


};


