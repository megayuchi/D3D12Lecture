#pragma once

#include <DirectXMath.h>

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
	XMFLOAT4	offset;
};

struct TEXTURE_HANDLE
{
	ID3D12Resource*	pTexResource;
	D3D12_CPU_DESCRIPTOR_HANDLE srv;
};