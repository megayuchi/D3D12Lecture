#include "pch.h"
#include "D3D12Renderer.h"
#include "FontManager.h"

using namespace D2D1;

CFontManager::CFontManager()
{
}
BOOL CFontManager::Initialize(CD3D12Renderer* pRenderer, ID3D12CommandQueue* pCommandQueue, UINT Width, UINT Height, BOOL bEnableDebugLayer)
{
	ID3D12Device* pD3DDevice = pRenderer->INL_GetD3DDevice();
	CreateD2D(pD3DDevice, pCommandQueue, bEnableDebugLayer);
	
	float fDPI = pRenderer->INL_GetDPI();
	CreateDWrite(pD3DDevice, Width, Height, fDPI);

	return TRUE;
}
BOOL CFontManager::CreateD2D(ID3D12Device* pD3DDevice, ID3D12CommandQueue* pCommandQueue, BOOL bEnableDebugLayer)
{

	// Create D3D11 on D3D12
	// Create an 11 device wrapped around the 12 device and share
	// 12's command queue.
	UINT	d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

	ID2D1Factory3*	pD2DFactory = nullptr;
	ID3D11Device*	pD3D11Device = nullptr;
	ID3D11DeviceContext*	pD3D11DeviceContext = nullptr;
	ID3D11On12Device*		pD3D11On12Device = nullptr;
	if (bEnableDebugLayer)
	{
		d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
	d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

	if (FAILED(D3D11On12CreateDevice(pD3DDevice,
		d3d11DeviceFlags,
		nullptr,
		0,
		(IUnknown**)&pCommandQueue,
		1,
		0,
		&pD3D11Device,
		&pD3D11DeviceContext,
		nullptr
		)))
	{
		__debugbreak();
	}
	if (FAILED(pD3D11Device->QueryInterface(IID_PPV_ARGS(&pD3D11On12Device))))
		__debugbreak();

	// Create D2D/DWrite components.
	D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, (void**)&pD2DFactory)))
	{
		__debugbreak();
	}

	IDXGIDevice*	pDXGIDevice = nullptr;
	if (FAILED(pD3D11On12Device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice))))
	{
		__debugbreak();
	}
	if (FAILED(pD2DFactory->CreateDevice(pDXGIDevice, &m_pD2DDevice)))
	{
		__debugbreak();
	}


	if (FAILED(m_pD2DDevice->CreateDeviceContext(deviceOptions, &m_pD2DDeviceContext)))
	{
		__debugbreak();
	}

	if (pD3D11Device)
	{
		pD3D11Device->Release();
		pD3D11Device = nullptr;
	}
	if (pDXGIDevice)
	{
		pDXGIDevice->Release();
		pDXGIDevice = nullptr;
	}
	if (pD2DFactory)
	{
		pD2DFactory->Release();
		pD2DFactory = nullptr;
	}
	if (pD3D11On12Device)
	{
		pD3D11On12Device->Release();
		pD3D11On12Device = nullptr;
	}
	if (pD3D11DeviceContext)
	{
		pD3D11DeviceContext->Release();
		pD3D11DeviceContext = nullptr;
	}
	return TRUE;
}

BOOL CFontManager::CreateDWrite(ID3D12Device* pD3DDevice, UINT TexWidth, UINT TexHeight, float fDPI)
{
	BOOL		bResult = FALSE;

	m_D2DBitmapWidth = TexWidth;
	m_D2DBitmapHeight = TexHeight;

	//InitCustomFont(pCustomFontList, dwCustomFontNum);

	D2D1_SIZE_U	size;
	size.width = TexWidth;
	size.height = TexHeight;

	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
		fDPI,
		fDPI
		);

	if (FAILED(m_pD2DDeviceContext->CreateBitmap(size, nullptr, 0, &bitmapProperties, &m_pD2DTargetBitmap)))
		__debugbreak();

	bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_CPU_READ;
	if (FAILED(m_pD2DDeviceContext->CreateBitmap(size, nullptr, 0, &bitmapProperties, &m_pD2DTargetBitmapRead)))
		__debugbreak();

	if (FAILED(m_pD2DDeviceContext->CreateSolidColorBrush(ColorF(ColorF::White), &m_pWhiteBrush)))
		__debugbreak();

	HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory5), (IUnknown**)&m_pDWFactory);
	if (FAILED(hr))
		__debugbreak();

	bResult = TRUE;
lb_return:
	return bResult;
}
FONT_HANDLE* CFontManager::CreateFontObject(const WCHAR* wchFontFamilyName, float fFontSize)
{
	IDWriteTextFormat* pTextFormat = nullptr;
	IDWriteFactory5*	pDWFactory = m_pDWFactory;
	IDWriteFontCollection1*	pFontCollection = nullptr;

	// The logical size of the font in DIP("device-independent pixel") units.A DIP equals 1 / 96 inch.

	if (pDWFactory)
	{
		if (FAILED(pDWFactory->CreateTextFormat(
			wchFontFamilyName,
			pFontCollection,                       // Font collection (nullptr sets it to use the system font collection).
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			fFontSize,
			DEFULAT_LOCALE_NAME,
			&pTextFormat
			)))
		{
			__debugbreak();
		}
	}
	FONT_HANDLE* pFontHandle = new FONT_HANDLE;
	memset(pFontHandle, 0, sizeof(FONT_HANDLE));
	wcscpy_s(pFontHandle->wchFontFamilyName, wchFontFamilyName);
	pFontHandle->fFontSize = fFontSize;

	if (pTextFormat)
	{
		if (FAILED(pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)))
			__debugbreak();


		if (FAILED(pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)))
			__debugbreak();
	}

	pFontHandle->pTextFormat = pTextFormat;

	return pFontHandle;
}
void CFontManager::DeleteFontObject(FONT_HANDLE* pFontHandle)
{
	if (pFontHandle->pTextFormat)
	{
		pFontHandle->pTextFormat->Release();
		pFontHandle->pTextFormat = nullptr;
	}
	delete pFontHandle;

}


BOOL CFontManager::WriteTextToBitmap(BYTE* pDestImage, UINT DestWidth, UINT DestHeight, UINT DestPitch, int* piOutWidth, int* piOutHeight, FONT_HANDLE* pFontHandle, const WCHAR* wchString, DWORD dwLen)
{
	int iTextWidth = 0;
	int iTextHeight = 0;

	BOOL bResult = CreateBitmapFromText(&iTextWidth, &iTextHeight, pFontHandle->pTextFormat, wchString, dwLen);
	if (bResult)
	{
		if (iTextWidth > (int)DestWidth)
			iTextWidth = (int)DestWidth;

		if (iTextHeight > (int)DestHeight)
			iTextHeight = (int)DestHeight;

		D2D1_MAPPED_RECT	mappedRect;
		if (FAILED(m_pD2DTargetBitmapRead->Map(D2D1_MAP_OPTIONS_READ, &mappedRect)))
			__debugbreak();

		BYTE*	pDest = pDestImage;
		char*	pSrc = (char*)mappedRect.bits;

		for (DWORD y = 0; y < (DWORD)iTextHeight; y++)
		{
			memcpy(pDest, pSrc, iTextWidth * 4);
			pDest += DestPitch;
			pSrc += mappedRect.pitch;
		}
		m_pD2DTargetBitmapRead->Unmap();
	}
	*piOutWidth = iTextWidth;
	*piOutHeight = iTextHeight;
	return bResult;
}

BOOL CFontManager::CreateBitmapFromText(int* piOutWidth, int* piOutHeight, IDWriteTextFormat* pTextFormat, const WCHAR* wchString, DWORD dwLen)
{
	BOOL	bResult = FALSE;

	ID2D1DeviceContext*	pD2DDeviceContext = m_pD2DDeviceContext;
	IDWriteFactory5*	pDWFactory = m_pDWFactory;
	D2D1_SIZE_F max_size = pD2DDeviceContext->GetSize();
	max_size.width = (float)m_D2DBitmapWidth;
	max_size.height = (float)m_D2DBitmapHeight;

	IDWriteTextLayout*	pTextLayout = nullptr;
	if (pDWFactory && pTextFormat)
	{
		if (FAILED(pDWFactory->CreateTextLayout(wchString, dwLen, pTextFormat, max_size.width, max_size.height, &pTextLayout)))
			__debugbreak();
	}
	DWRITE_TEXT_METRICS metrics = {};
	if (pTextLayout)
	{
		pTextLayout->GetMetrics(&metrics);

		// 타겟설정
		pD2DDeviceContext->SetTarget(m_pD2DTargetBitmap);

		// 안티앨리어싱모드 설정
		pD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

		// 텍스트 렌더링
		pD2DDeviceContext->BeginDraw();

		pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::Black));
		pD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

		pD2DDeviceContext->DrawTextLayout(D2D1::Point2F(0.0f, 0.0f), pTextLayout, m_pWhiteBrush);

		// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
		// is lost. It will be handled during the next call to Present.
		pD2DDeviceContext->EndDraw();

		// 안티앨리어싱 모드 복구    
		pD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_DEFAULT);
		pD2DDeviceContext->SetTarget(nullptr);

		// 레이아웃 오브젝트 필요없으니 Release
		pTextLayout->Release();
		pTextLayout = nullptr;
	}
	int width = (int)ceil(metrics.width);
	int height = (int)ceil(metrics.height);

	D2D1_POINT_2U	destPos = {};
	D2D1_RECT_U		srcRect = { 0, 0, width, height };
	if (FAILED(m_pD2DTargetBitmapRead->CopyFromBitmap(&destPos, m_pD2DTargetBitmap, &srcRect)))
		__debugbreak();

	*piOutWidth = width;
	*piOutHeight = height;

	bResult = TRUE;

lb_return:
	return bResult;
}

void CFontManager::CleanupDWrite()
{
	if (m_pD2DTargetBitmap)
	{
		m_pD2DTargetBitmap->Release();
		m_pD2DTargetBitmap = nullptr;
	}
	if (m_pD2DTargetBitmapRead)
	{
		m_pD2DTargetBitmapRead->Release();
		m_pD2DTargetBitmapRead = nullptr;
	}
	if (m_pWhiteBrush)
	{
		m_pWhiteBrush->Release();
		m_pWhiteBrush = nullptr;
	}
	if (m_pDWFactory)
	{
		m_pDWFactory->Release();
		m_pDWFactory = nullptr;
	}
}
void CFontManager::CleanupD2D()
{
	if (m_pD2DDeviceContext)
	{
		m_pD2DDeviceContext->Release();
		m_pD2DDeviceContext = nullptr;
	}
	if (m_pD2DDevice)
	{
		m_pD2DDevice->Release();
		m_pD2DDevice = nullptr;
	}
}
void CFontManager::Cleanup()
{
	CleanupDWrite();
	CleanupD2D();
}
CFontManager::~CFontManager()
{
	Cleanup();
}