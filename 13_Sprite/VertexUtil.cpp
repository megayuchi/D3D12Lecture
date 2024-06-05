#include "pch.h"
#include <Windows.h>
#include "typedef.h"


DWORD AddVertex(BasicVertex* pVertexList, DWORD dwMaxVertexCount, DWORD* pdwInOutVertexCount, const BasicVertex* pVertex);

DWORD CreateBoxMesh(BasicVertex** ppOutVertexList, WORD* pOutIndexList, DWORD dwMaxBufferCount, float fHalfBoxLen)
{
	const DWORD INDEX_COUNT = 36;
	if (dwMaxBufferCount < INDEX_COUNT)
		__debugbreak();

	const WORD pIndexList[INDEX_COUNT] =
	{
		// +z
		3, 0, 1,
		3, 1, 2,

		// -z
		4, 7, 6,
		4, 6, 5,

		// -x
		0, 4, 5,
		0, 5, 1,

		// +x
		7, 3, 2,
		7, 2, 6,

		// +y
		0, 3, 7,
		0, 7, 4,

		// -y
		2, 1, 5,
		2, 5, 6
	};
	
	TVERTEX pTexCoordList[INDEX_COUNT] =
	{
		// +z
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
		
		// -z
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},

		// -x
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},

		// +x
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},

		// +y
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
		
		// -y
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
	};
	
	FLOAT3 pWorldPosList[8];
	pWorldPosList[0] = { -fHalfBoxLen, fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[1] = { -fHalfBoxLen, -fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[2] = { fHalfBoxLen, -fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[3] = { fHalfBoxLen, fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[4] = { -fHalfBoxLen, fHalfBoxLen, -fHalfBoxLen };
	pWorldPosList[5] = { -fHalfBoxLen, -fHalfBoxLen, -fHalfBoxLen };
	pWorldPosList[6] = { fHalfBoxLen, -fHalfBoxLen, -fHalfBoxLen };
	pWorldPosList[7] = { fHalfBoxLen, fHalfBoxLen, -fHalfBoxLen };

	const DWORD MAX_WORKING_VERTEX_COUNT = 65536;
	BasicVertex* pWorkingVertexList = new BasicVertex[MAX_WORKING_VERTEX_COUNT];
	memset(pWorkingVertexList, 0, sizeof(BasicVertex)*MAX_WORKING_VERTEX_COUNT);
	DWORD dwBasicVertexCount = 0;

	for (DWORD i = 0; i < INDEX_COUNT; i++)
	{
		BasicVertex v;
		v.color = { 1.0f, 1.0f, 1.0f, 1.0f };
		v.position = { pWorldPosList[pIndexList[i]].x, pWorldPosList[pIndexList[i]].y, pWorldPosList[pIndexList[i]].z };
		v.texCoord = { pTexCoordList[i].u, pTexCoordList[i].v };

		pOutIndexList[i] = AddVertex(pWorkingVertexList, MAX_WORKING_VERTEX_COUNT, &dwBasicVertexCount, &v);
	}
	BasicVertex* pNewVertexList = new BasicVertex[dwBasicVertexCount];
	memcpy(pNewVertexList, pWorkingVertexList, sizeof(BasicVertex) * dwBasicVertexCount);

	*ppOutVertexList = pNewVertexList;

	delete[] pWorkingVertexList;
	pWorkingVertexList = nullptr;

	return dwBasicVertexCount;
}

void DeleteBoxMesh(BasicVertex* pVertexList)
{
	delete[] pVertexList;
}
DWORD AddVertex(BasicVertex* pVertexList, DWORD dwMaxVertexCount, DWORD* pdwInOutVertexCount, const BasicVertex* pVertex)
{
	DWORD dwFoundIndex = -1;
	DWORD dwExistVertexCount = *pdwInOutVertexCount;
	for (DWORD i = 0; i < dwExistVertexCount; i++)
	{
		const BasicVertex* pExistVertex = pVertexList + i;
		if (!memcmp(pExistVertex, pVertex, sizeof(BasicVertex)))
		{
			dwFoundIndex = i;
			goto lb_return;
		}
	}
	if (dwExistVertexCount + 1 > dwMaxVertexCount)
	{
		__debugbreak();
		goto lb_return;
	}
	// 새로운 vertex추가
	dwFoundIndex = dwExistVertexCount;
	pVertexList[dwFoundIndex] = *pVertex;
	*pdwInOutVertexCount = dwExistVertexCount + 1;
lb_return:
	return dwFoundIndex;
}
