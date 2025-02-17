#include "pch.h"
#include <Windows.h>
#include <DirectXMath.h>
#include "../Common/typedef.h"
#include "../Common/IRenderer.h"
#include "../Util/LinkedList.h"
#include "../Util/VertexUtil.h"
#include "Game.h"
#include "GameObject.h"

CGameObject::CGameObject()
{
	m_LinkInGame.pItem = this;
	m_LinkInGame.pNext = nullptr;
	m_LinkInGame.pPrv = nullptr;

	m_matScale = XMMatrixIdentity();
	m_matRot = XMMatrixIdentity();
	m_matTrans = XMMatrixIdentity();
	m_matWorld = XMMatrixIdentity();
}
BOOL CGameObject::Initialize(CGame* pGame)
{
	BOOL bResult = FALSE;
	CGame* m_pGame = pGame;
	m_pRenderer = pGame->INL_GetRenderer();

	m_pMeshObj = CreateBoxMeshObject();
	if (m_pMeshObj)
	{
		bResult = TRUE;
	}
	return bResult;
	
}
void CGameObject::UpdateTransform()
{
	// world matrix = scale x rotation x trasnlation
	m_matWorld = XMMatrixMultiply(m_matScale, m_matRot);
	m_matWorld = XMMatrixMultiply(m_matWorld, m_matTrans);
}
void CGameObject::SetPosition(float x, float y, float z)
{
	m_Pos.m128_f32[0] = x;
	m_Pos.m128_f32[1] = y;
	m_Pos.m128_f32[2] = z;

	m_matTrans = XMMatrixTranslation(x, y, z);
	
	m_bUpdateTransform = TRUE;
}
void CGameObject::SetScale(float x, float y, float z)
{
	m_Scale.m128_f32[0] = x;
	m_Scale.m128_f32[1] = y;
	m_Scale.m128_f32[2] = z;

	m_matScale = XMMatrixScaling(x, y, z);
	
	m_bUpdateTransform = TRUE;
}
void CGameObject::SetRotationY(float fRotY)
{
	m_fRotY = fRotY;
	m_matRot = XMMatrixRotationY(fRotY);

	m_bUpdateTransform = TRUE;
}
	
void CGameObject::Run()
{
	// per 30FPS or 60 FPS
	if (m_bUpdateTransform)
	{
		UpdateTransform();
		m_bUpdateTransform = FALSE;
	}
	else
	{
		int a = 0;
	}
}
void CGameObject::Render()
{
	if (m_pMeshObj)
	{
		m_pRenderer->RenderMeshObject(m_pMeshObj, &m_matWorld);
	}
}

IMeshObject* CGameObject::CreateBoxMeshObject()
{
	IMeshObject* pMeshObj = nullptr;

	// create box mesh
	// create vertices and indices
	WORD	pIndexList[36] = {};
	BasicVertex* pVertexList = nullptr;
	DWORD dwVertexCount = CreateBoxMesh(&pVertexList, pIndexList, (DWORD)_countof(pIndexList), 0.25f);

	// create BasicMeshObject from Renderer
	pMeshObj = m_pRenderer->CreateBasicMeshObject();

	const WCHAR* wchTexFileNameList[6] =
	{
		L"./Data/tex_00.dds",
		L"./Data/tex_01.dds",
		L"./Data/tex_02.dds",
		L"./Data/tex_03.dds",
		L"./Data/tex_04.dds",
		L"./Data/tex_05.dds"
	};

	// Set meshes to the BasicMeshObject
	pMeshObj->BeginCreateMesh(pVertexList, dwVertexCount, 6);	// 박스의 6면-1면당 삼각형 2개-인덱스 6개
	for (DWORD i = 0; i < 6; i++)
	{
		pMeshObj->InsertTriGroup(pIndexList + i * 6, 2, wchTexFileNameList[i]);
	}
	pMeshObj->EndCreateMesh();

	// delete vertices and indices
	if (pVertexList)
	{
		DeleteBoxMesh(pVertexList);
		pVertexList = nullptr;
	}
	return pMeshObj;
}
IMeshObject* CGameObject::CreateQuadMesh()
{
	IMeshObject* pMeshObj = m_pRenderer->CreateBasicMeshObject();

	// Set meshes to the BasicMeshObject
	BasicVertex pVertexList[] =
	{
		{ { -0.25f, 0.25f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { 0.25f, 0.25f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { 0.25f, -0.25f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
	};

	WORD pIndexList[] =
	{
		0, 1, 2,
		0, 2, 3
	};


	pMeshObj->BeginCreateMesh(pVertexList, (DWORD)_countof(pVertexList), 1);	
	pMeshObj->InsertTriGroup(pIndexList, 2, L"tex_06.dds");
	pMeshObj->EndCreateMesh();
	return pMeshObj;
}
void CGameObject::Cleanup()
{
	if (m_pMeshObj)
	{
		m_pMeshObj->Release();
		m_pMeshObj = nullptr;
	}
}
CGameObject::~CGameObject()
{
	Cleanup();
}
