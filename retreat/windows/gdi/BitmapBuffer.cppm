module;

#include "stdafx.h"
#include <atlimage.h>

export module BitmapBuffer;

// keeps a bitmap that can be drawn on and could be drawn on a window DC
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

protected:

	CSize m_imageSize;
	CDC m_memDC;

	HBITMAP m_hOldBitmap;

	CRect m_plotRect;
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

	CImage image;
	HRESULT result = image.Load(fileName);

	if (SUCCEEDED(result)) {
		int nWidth = image.GetWidth();
		int nHeight = image.GetHeight();

		m_imageSize.SetSize(nWidth, nHeight);

		Attach(image.Detach());
	}

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


