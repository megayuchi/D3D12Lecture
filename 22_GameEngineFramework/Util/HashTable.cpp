#include "pch.h"
#include <Windows.h>
#include "LinkedList.h"
#include "HashTable.h"


CHashTable::CHashTable()
{
}

BOOL CHashTable::Initialize(DWORD dwMaxBucketNum, DWORD dwMaxKeySize, DWORD dwMaxItemNum)
{
	m_dwMaxKeyDataSize = dwMaxKeySize;
	m_dwMaxBucketNum = dwMaxBucketNum;

	m_pBucketDescTable = new BUCKET_DESC[dwMaxBucketNum];
	memset(m_pBucketDescTable, 0, sizeof(BUCKET_DESC) * dwMaxBucketNum);

	return TRUE;
}


DWORD CHashTable::CreateKey(const void* pData, DWORD dwSize, DWORD dwBucketNum)
{
	DWORD	dwKeyData = 0;

	const char*	pEntry = (char*)pData;
	if (dwSize & 0x00000001)
	{
		dwKeyData += (DWORD)(*(BYTE*)pEntry);
		pEntry++;
		dwSize--;
	}
	if (!dwSize)
		goto lb_exit;


	if (dwSize & 0x00000002)
	{
		dwKeyData += (DWORD)(*(WORD*)pEntry);
		pEntry += 2;
		dwSize -= 2;
	}
	if (!dwSize)
		goto lb_exit;

	dwSize = (dwSize >> 2);

	for (DWORD i = 0; i < dwSize; i++)
	{
		dwKeyData += *(DWORD*)pEntry;
		pEntry += 4;
	}

lb_exit:
	DWORD	dwIndex = dwKeyData % dwBucketNum;

	return dwIndex;
}
DWORD CHashTable::Select(void** ppOutItemList, DWORD dwMaxItemNum, const void* pKeyData, DWORD dwSize)
{

	DWORD			dwSelectedItemNum = 0;
	DWORD			dwIndex = CreateKey(pKeyData, dwSize, m_dwMaxBucketNum);
	BUCKET_DESC*	pBucketDesc = m_pBucketDescTable + dwIndex;

	SORT_LINK*		pCur = pBucketDesc->pBucketLinkHead;

	VB_BUCKET*		pBucket;

	while (pCur)
	{
		if (!dwMaxItemNum)
			goto lb_return;

		pBucket = (VB_BUCKET*)pCur->pItem;

		if (pBucket->dwSize != dwSize)
			goto lb_next;

		if (memcmp(pBucket->pKeyData, pKeyData, dwSize))
			goto lb_next;


		dwMaxItemNum--;

		ppOutItemList[dwSelectedItemNum] = (void*)pBucket->pItem;
		dwSelectedItemNum++;
	lb_next:
		pCur = pCur->pNext;
	}

lb_return:
	return dwSelectedItemNum;
}
void* CHashTable::Insert(const void* pItem, const void* pKeyData, DWORD dwSize)
{
	void*	pSearchHandle = nullptr;

	if (dwSize > m_dwMaxKeyDataSize)
	{
		__debugbreak();
		goto lb_return;
	}

	DWORD dwBucketMemSize = (DWORD)(sizeof(VB_BUCKET) - sizeof(char)) + m_dwMaxKeyDataSize;
	VB_BUCKET*	pBucket = (VB_BUCKET*)malloc(dwBucketMemSize);

	DWORD			dwIndex = CreateKey(pKeyData, dwSize, m_dwMaxBucketNum);
	BUCKET_DESC*	pBucketDesc = m_pBucketDescTable + dwIndex;

	pBucket->pItem = pItem;
	pBucket->dwSize = dwSize;
	pBucket->pBucketDesc = pBucketDesc;
	pBucket->sortLink.pPrv = nullptr;
	pBucket->sortLink.pNext = nullptr;
	pBucket->sortLink.pItem = pBucket;
	pBucketDesc->dwLinkNum++;

	memcpy(pBucket->pKeyData, pKeyData, dwSize);


	LinkToLinkedListFIFO(&pBucketDesc->pBucketLinkHead, &pBucketDesc->pBucketLinkTail, &pBucket->sortLink);

	m_dwItemNum++;
	pSearchHandle = pBucket;

lb_return:
	return pSearchHandle;



}
void CHashTable::Delete(const void* pSearchHandle)
{

	VB_BUCKET*		pBucket = (VB_BUCKET*)pSearchHandle;
	BUCKET_DESC*	pBucketDesc = pBucket->pBucketDesc;

	UnLinkFromLinkedList(&pBucketDesc->pBucketLinkHead, &pBucketDesc->pBucketLinkTail, &pBucket->sortLink);

	pBucketDesc->dwLinkNum--;

	free(pBucket);
	m_dwItemNum--;

}


DWORD	CHashTable::GetMaxBucketNum() const
{
	return m_dwMaxBucketNum;
}


void CHashTable::DeleteAll()
{

	VB_BUCKET*		pBucket;

	for (DWORD i = 0; i < m_dwMaxBucketNum; i++)
	{
		while (m_pBucketDescTable[i].pBucketLinkHead)
		{
			pBucket = (VB_BUCKET*)m_pBucketDescTable[i].pBucketLinkHead->pItem;
			Delete(pBucket);
		}
	}
}

DWORD CHashTable::GetAllItems(void** ppOutItemList, DWORD dwMaxItemNum, BOOL* pbOutInsufficient) const
{
	VB_BUCKET*		pBucket;
	SORT_LINK*		pCur;

	*pbOutInsufficient = FALSE;
	DWORD			dwItemNum = 0;

	for (DWORD i = 0; i < m_dwMaxBucketNum; i++)
	{
		pCur = m_pBucketDescTable[i].pBucketLinkHead;
		while (pCur)
		{
			pBucket = (VB_BUCKET*)pCur->pItem;

			if (dwItemNum >= dwMaxItemNum)
			{
				*pbOutInsufficient = TRUE;
				goto lb_return;
			}


			ppOutItemList[dwItemNum] = (void*)pBucket->pItem;
			dwItemNum++;

			pCur = pCur->pNext;
		}
	}
lb_return:
	return dwItemNum;
}

DWORD CHashTable::GetKeyPtrAndSize(void** ppOutKeyPtr, const void* pSearchHandle) const
{

	*ppOutKeyPtr = ((VB_BUCKET*)pSearchHandle)->pKeyData;
	DWORD	dwSize = ((VB_BUCKET*)pSearchHandle)->dwSize;

	return dwSize;
}



DWORD CHashTable::GetItemNum() const
{
	return m_dwItemNum;
}

void CHashTable::ResourceCheck() const
{
	if (m_dwItemNum)
		__debugbreak();
}
void CHashTable::Cleanup()
{
	ResourceCheck();

	DeleteAll();
	if (m_pBucketDescTable)
	{
		delete[] m_pBucketDescTable;
		m_pBucketDescTable = nullptr;
	}
}
CHashTable::~CHashTable()
{
	Cleanup();
}