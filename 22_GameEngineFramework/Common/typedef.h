#pragma once

#include <DirectXMath.h>
#include "../Util/LinkedList.h"

using namespace DirectX;

struct BasicVertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
	XMFLOAT2 texCoord;
};

union RGBA
{
	struct
	{
		BYTE	r;
		BYTE	g;
		BYTE	b;
		BYTE	a;
	};
	BYTE		bColorFactor[4];
};

struct TVERTEX
{
	float u;
	float v;
};
struct FLOAT3
{
	float x;
	float y;
	float z;
};

#define DEFULAT_LOCALE_NAME		L"ko-kr"

HRESULT typedef (__stdcall *CREATE_INSTANCE_FUNC)(void* ppv);