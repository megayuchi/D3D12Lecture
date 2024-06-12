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


struct CONSTANT_BUFFER_DEFAULT
{
	XMMATRIX	matWorld;
	XMMATRIX	matView;
	XMMATRIX	matProj;
};
struct CONSTANT_BUFFER_SPRITE
{
	XMFLOAT2 ScreenRes;
	XMFLOAT2 Pos;
	XMFLOAT2 Scale;
	XMFLOAT2 TexSize;
	XMFLOAT2 TexSampePos;
	XMFLOAT2 TexSampleSize;
	float	Z;
	float	Alpha;
	float	Reserved0;
	float	Reserved1;
};


enum CONSTANT_BUFFER_TYPE
{
	CONSTANT_BUFFER_TYPE_DEFAULT,
	CONSTANT_BUFFER_TYPE_SPRITE,
	CONSTANT_BUFFER_TYPE_COUNT
};

struct CONSTANT_BUFFER_PROPERTY
{
	CONSTANT_BUFFER_TYPE type;
	UINT Size;
};


struct TEXTURE_HANDLE
{
	ID3D12Resource*	pTexResource;
	ID3D12Resource*	pUploadBuffer;
	D3D12_CPU_DESCRIPTOR_HANDLE srv;
	BOOL	bUpdated;
	SORT_LINK	Link;
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