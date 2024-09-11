#include "pch.h"
#include <Windows.h>
#include <process.h>
#include "D3D12Renderer.h"
#include "RenderThread.h"

class CD3D12Renderer;

UINT WINAPI RenderThread(void* pArg)
{
	RENDER_THREAD_DESC* pDesc = (RENDER_THREAD_DESC*)pArg;
	CD3D12Renderer* pRenderer = pDesc->pRenderer;
	DWORD dwThreadIndex = pDesc->dwThreadIndex;
	const HANDLE* phEventList = pDesc->hEventList;
	while (1)
	{
		DWORD dwEventIndex = WaitForMultipleObjects(RENDER_THREAD_EVENT_TYPE_COUNT, phEventList, FALSE, INFINITE);

		switch (dwEventIndex)
		{
			case RENDER_THREAD_EVENT_TYPE_PROCESS:
				pRenderer->ProcessByThread(dwThreadIndex);
				break;
			case RENDER_THREAD_EVENT_TYPE_DESTROY:
				goto lb_exit;
			default:
				__debugbreak();
		}
	}
lb_exit:
	_endthreadex(0);
	return 0;
}