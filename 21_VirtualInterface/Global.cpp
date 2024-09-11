#include "pch.h"
#include "Global.h"
#include <Windows.h>
#include "typedef.h"
#include "IRenderer.h"
#include "D3D12Renderer.h"

HRESULT CreateRendererInstance(IRenderer** ppOutRenderer)
{
	*ppOutRenderer = new CD3D12Renderer;
	return S_OK;
}