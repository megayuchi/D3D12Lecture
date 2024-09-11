#pragma once

#include <Windows.h>
#include "typedef.h"
#include <combaseapi.h>


interface IMeshObject : public IUnknown
{
	virtual BOOL	__stdcall BeginCreateMesh(const BasicVertex* pVertexList, DWORD dwVertexNum, DWORD dwTriGroupCount) = 0;
	virtual BOOL	__stdcall InsertTriGroup(const WORD* pIndexList, DWORD dwTriCount, const WCHAR* wchTexFileName) = 0;
	virtual void	__stdcall EndCreateMesh() = 0;
};
interface ISprite : public IUnknown
{

};
interface IRenderer : public IUnknown
{
	virtual BOOL	__stdcall Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV, const WCHAR* wchShaderPath) = 0;
	virtual void	__stdcall BeginRender() = 0;
	virtual void	__stdcall EndRender() = 0;
	virtual void	__stdcall Present() = 0;
	virtual BOOL	__stdcall UpdateWindowSize(DWORD dwBackBufferWidth, DWORD dwBackBufferHeight) = 0;

	virtual IMeshObject* __stdcall CreateBasicMeshObject() = 0;

	virtual ISprite* __stdcall CreateSpriteObject() = 0;
	virtual ISprite* __stdcall CreateSpriteObject(const WCHAR* wchTexFileName, int PosX, int PosY, int Width, int Height) = 0;

	virtual void*	__stdcall CreateTiledTexture(UINT TexWidth, UINT TexHeight, DWORD r, DWORD g, DWORD b) = 0;
	virtual void*	__stdcall CreateDynamicTexture(UINT TexWidth, UINT TexHeight) = 0;
	virtual void*	__stdcall CreateTextureFromFile(const WCHAR* wchFileName) = 0;
	virtual void	__stdcall DeleteTexture(void* pTexHandle) = 0;

	virtual void*	__stdcall CreateFontObject(const WCHAR* wchFontFamilyName, float fFontSize) = 0;
	virtual void	__stdcall DeleteFontObject(void* pFontHandle) = 0;
	virtual BOOL	__stdcall WriteTextToBitmap(BYTE* pDestImage, UINT DestWidth, UINT DestHeight, UINT DestPitch, int* piOutWidth, int* piOutHeight, void* pFontObjHandle, const WCHAR* wchString, DWORD dwLen) = 0;

	virtual void	__stdcall RenderMeshObject(IMeshObject* pMeshObj, const XMMATRIX* pMatWorld) = 0;
	virtual void	__stdcall RenderSpriteWithTex(void* pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY, const RECT* pRect, float Z, void* pTexHandle) = 0;
	virtual void	__stdcall RenderSprite(void* pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY, float Z) = 0;
	virtual void	__stdcall UpdateTextureWithImage(void* pTexHandle, const BYTE* pSrcBits, UINT SrcWidth, UINT SrcHeight) = 0;

	virtual void	__stdcall SetCameraPos(float x, float y, float z) = 0;
	virtual void	__stdcall MoveCamera(float x, float y, float z) = 0;
	virtual void	__stdcall GetCameraPos(float* pfOutX, float* pfOutY, float* pfOutZ) = 0;
	virtual void	__stdcall SetCamera(const XMVECTOR* pCamPos, const XMVECTOR* pCamDir, const XMVECTOR* pCamUp) = 0;
	virtual DWORD	__stdcall GetCommandListCount() = 0;
};