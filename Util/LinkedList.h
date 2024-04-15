#pragma once

struct SORT_LINK
{
	SORT_LINK*		pPrv;
	SORT_LINK*		pNext;
	void*			pItem;
};

void LinkToSortLink(SORT_LINK** ppHead,SORT_LINK** ppTail,SORT_LINK* pNew);
void LinkToSortLinkFIFO(SORT_LINK** ppHead,SORT_LINK** ppTail,SORT_LINK* pNew);
void UnLinkFromSortLink(SORT_LINK** ppHead,SORT_LINK** ppTail,SORT_LINK* pDel);