#include "pch.h"
#include "QueryPerfCounter.h"
#include <Windows.h>

LARGE_INTEGER	g_Frequency = {};

void QCInit()
{
	QueryPerformanceFrequency(&g_Frequency);
}
LARGE_INTEGER QCGetCounter()
{
	LARGE_INTEGER CurCounter;
	QueryPerformanceCounter(&CurCounter);
	return CurCounter;
}
float QCMeasureElapsedTick(LARGE_INTEGER CurCounter, LARGE_INTEGER PrvCounter)
{
#ifdef _DEBUG
	if (!g_Frequency.QuadPart)
		__debugbreak();
#endif

	UINT64 ElapsedCounter = CurCounter.QuadPart - PrvCounter.QuadPart;
	float ElapsedSec = ((float)ElapsedCounter / (float)g_Frequency.QuadPart);
	float ElapsedMilSec = ElapsedSec * 1000.0f;

	return ElapsedMilSec;
}
LARGE_INTEGER QCCounterAddTick(LARGE_INTEGER Counter, float Tick)
{
#ifdef _DEBUG
	if (!g_Frequency.QuadPart)
		__debugbreak();
#endif
	LARGE_INTEGER	result = Counter;

	float	Sec = Tick / 1000.0f;
	UINT64 ElapsedCounter = (UINT64)(Sec * (float)g_Frequency.QuadPart);
	result.QuadPart += ElapsedCounter;

	return result;
}

LARGE_INTEGER QCCounterSubTick(LARGE_INTEGER Counter, float Tick)
{
#ifdef _DEBUG
	if (!g_Frequency.QuadPart)
		__debugbreak();
#endif
	LARGE_INTEGER	result = Counter;

	float	Sec = Tick / 1000.0f;
	UINT64 ElapsedCounter = (UINT64)(Sec * (float)g_Frequency.QuadPart);
	result.QuadPart -= ElapsedCounter;

	return result;
}
