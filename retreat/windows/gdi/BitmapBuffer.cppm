module;

#include "stdafx.h"

export module BitmapBuffer;

export class CBitmapBuffer: public CBitmap
{
public:
	CBitmapBuffer(HDC compatibleDC);
	virtual ~CBitmapBuffer();

	HRESULT LoadFromFile(const TCHAR* name);

	int GetHeight();
	int GetWitdth();
	const CSize& GetSize();
	CDC &GetDC();

	void SetPlotRect(const CRect& rect);

	void NewBitmap(int cx, int cy, COLORREF color);
	void FillBackground(int width, int height, COLORREF color);
	void Resize(const CSize& newSize);
	void Draw(HDC dc);

	HDC SelectBitmapToInternalDC();
	void DeselectBitmapFromInternalDC();

	//void StarrySky(int cx, int cy, COLORREF background);

protected:

	CSize m_imageSize;
	CDC m_memDC;

	HBITMAP m_hOldBitmap;

	CRect m_plotRect;
	
	//void drawStar2(HDC dc, CPoint c, int r, int ang, COLORREF color);
	//void fillPolygon(HDC dc, POINT* points, int npoints, COLORREF color);
};

module :private;

#define DEFAULT_BPP 32

CBitmapBuffer::CBitmapBuffer(HDC compatibleDC) :
	m_hOldBitmap(NULL)
{
	m_memDC.CreateCompatibleDC(compatibleDC);
}

CBitmapBuffer::~CBitmapBuffer()
{
	if (m_hOldBitmap)
		DeselectBitmapFromInternalDC();
}

HRESULT CBitmapBuffer::LoadFromFile(const TCHAR* fileName)
{
	ATLASSERT(m_hOldBitmap == NULL);

	if (m_hBitmap)
	{
		DeleteObject();
	}

	m_imageSize.SetSize(0, 0);

	CComPtr<IPicture> pPicture;
	CComBSTR file_path = fileName;

	// load image
	HRESULT hr = ::OleLoadPicturePath(
		file_path,
		0, 0, 0,
		IID_IPicture,
		(void**)&pPicture
	);

	if (FAILED(hr))
		return hr;

	// get image dimensions
	long hmWidth = 0;
	long hmHeight = 0;

	pPicture->get_Width(&hmWidth);
	pPicture->get_Height(&hmHeight);

	int nWidth = MulDiv(hmWidth, m_memDC.GetDeviceCaps(LOGPIXELSX), HIMETRIC_INCH);
	int nHeight = MulDiv(hmHeight, m_memDC.GetDeviceCaps(LOGPIXELSY), HIMETRIC_INCH);

	m_imageSize.SetSize(nWidth, nHeight);

	// render image
	CreateBitmap(nWidth, nHeight, 1, DEFAULT_BPP, NULL);
	HBITMAP hOldBitmap = m_memDC.SelectBitmap(m_hBitmap);

	HRESULT result = E_FAIL;

	if (pPicture)
	{
		result = pPicture->Render(
			m_memDC.m_hDC,
			0, 0,
			nWidth,
			nHeight,
			0,
			hmHeight,
			hmWidth,
			-hmHeight,
			NULL
		);
	}

	m_memDC.SelectBitmap(hOldBitmap);

	return result;
}

int CBitmapBuffer::GetHeight()
{
	return m_imageSize.cy;
}

int CBitmapBuffer::GetWitdth()
{
	return m_imageSize.cx;
}

const CSize& CBitmapBuffer::GetSize()
{
	return m_imageSize;
}

CDC &CBitmapBuffer::GetDC() 
{ 
	return m_memDC; 
}

void CBitmapBuffer::SetPlotRect(const CRect& rect)
{
	m_plotRect.CopyRect(rect);
}

void CBitmapBuffer::NewBitmap(int cx, int cy, COLORREF color) {
	ATLASSERT(m_hOldBitmap == NULL);

	if (m_hBitmap)
		DeleteObject();

	CreateBitmap(cx, cy, 1, DEFAULT_BPP, NULL);

	m_imageSize.SetSize(cx, cy);

	HBITMAP hOldBitmap = m_memDC.SelectBitmap(m_hBitmap);

	CBrush back;
	back.CreateSolidBrush(color);
	m_memDC.FillRect(CRect(CPoint(0, 0), CSize(cx + 1, cy + 1)), back);

	m_memDC.SelectBitmap(hOldBitmap);
}

void CBitmapBuffer::FillBackground(int width, int height, COLORREF color)
{
	ATLASSERT(m_hOldBitmap == NULL);

	HBITMAP hContent = Detach();
	CSize contentSize = m_imageSize;

	CreateBitmap(width, height, 1, DEFAULT_BPP, NULL);
	m_imageSize.SetSize(width, height);

	HBITMAP hOldBitmap = m_memDC.SelectBitmap(m_hBitmap);

	CBrush bgBrush;
	bgBrush.CreateSolidBrush(color);

	HBRUSH hOldBrush = m_memDC.SelectBrush(bgBrush);

	m_memDC.PatBlt(0, 0, width, height, PATCOPY);

	m_memDC.SelectBrush(hOldBrush);

	if (hContent)
	{
		CDC memDC;
		memDC.CreateCompatibleDC(m_memDC);

		HBITMAP hOldBitmap = memDC.SelectBitmap(hContent);

		int xDest = (m_imageSize.cx - contentSize.cx) / 2;
		int yDest = (m_imageSize.cy - contentSize.cy) / 2;

		m_memDC.BitBlt(xDest, yDest, contentSize.cx, contentSize.cy, memDC, 0, 0, SRCCOPY);

		memDC.SelectBitmap(hOldBitmap);

		::DeleteObject(hContent);
	}

	m_memDC.SelectBitmap(hOldBitmap);

}

void CBitmapBuffer::Resize(const CSize& newSize)
{
	ATLASSERT(m_hOldBitmap == NULL);

	if (!m_hBitmap || m_imageSize == newSize)
	{
		return;
	}

	CDC resizeDC;
	resizeDC.CreateCompatibleDC(m_memDC);

	CBitmap resizedBitmap;
	resizedBitmap.CreateBitmap(newSize.cx, newSize.cy, 1, DEFAULT_BPP, NULL);

	HBITMAP resizeDCOldBitmap = resizeDC.SelectBitmap(resizedBitmap);
	HBITMAP hOldBitmap = m_memDC.SelectBitmap(m_hBitmap);

	resizeDC.SetStretchBltMode(HALFTONE);
	resizeDC.StretchBlt(0, 0, newSize.cx, newSize.cy, m_memDC, 0, 0, m_imageSize.cx, m_imageSize.cy, SRCCOPY);

	resizeDC.SelectBitmap(resizeDCOldBitmap);
	m_memDC.SelectBitmap(hOldBitmap);

	m_imageSize.SetSize(newSize.cx, newSize.cy);

	::DeleteObject(Detach());
	Attach(resizedBitmap.Detach());
}

HDC CBitmapBuffer::SelectBitmapToInternalDC()
{
	if (!m_hOldBitmap)
		m_hOldBitmap = m_memDC.SelectBitmap(m_hBitmap);

	return m_memDC.m_hDC;
}

void CBitmapBuffer::DeselectBitmapFromInternalDC()
{
	if (m_hOldBitmap)
		m_memDC.SelectBitmap(m_hOldBitmap);

	m_hOldBitmap = NULL;
}

void CBitmapBuffer::Draw(HDC dc)
{
	if (!m_hBitmap)
	{
		return;
	}

	HBITMAP hOldBitmap = NULL;

	if (!m_hOldBitmap)
		HBITMAP hOldBitmap = m_memDC.SelectBitmap(m_hBitmap);

	::BitBlt(
		dc,
		m_plotRect.left,
		m_plotRect.top,
		m_imageSize.cx, m_imageSize.cy, m_memDC,
		0, 0, SRCCOPY
	);

	if (!m_hOldBitmap)
		m_memDC.SelectBitmap(hOldBitmap);
}


