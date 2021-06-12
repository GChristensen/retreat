module;

#include "stdafx.h"

export module TextBuffer;

import <memory>;
import <vector>;

export class CTextBuffer
{
public:

	CTextBuffer(HDC compatibleDC, int nTextItems);
	~CTextBuffer();

	void SetTextLayerProperties
	(
		const TCHAR* faceName,
		BYTE charSet,
		int fontSize,
		bool antialiased,
		bool bold,
		bool italic
	);

	void SetTextItem
	(
		HDC dc,
		unsigned item,
		int x,
		int y,
		const TCHAR* text,
		COLORREF textColor,
		bool doublebuffered = true,
		bool draw = false
	);

	void MoveTextItem
	(
		HDC dc,
		unsigned item,
		int x,
		int y,
		bool redraw = true
	);

	void SetItemText
	(
		HDC dc,
		unsigned item,
		const TCHAR* text,
		COLORREF textColor,
		bool redraw = true
	);

	void RepaintTextItem
	(
		HDC dc,
		unsigned textItem,
		bool useStored = true
	);

	void GetBackground
	(
		HDC dc,
		unsigned textItem
	);

	void EraseTextItem(HDC dc, unsigned item);

	void GetTextExtent(unsigned item, CSize& size);
	void GetTextExtent(const TCHAR* text, CSize& size);

	void SetWindowOrigin(const CPoint& org);

private:

	bool m_isItalic;

	CFont m_textLayerFont;
	CDC m_memDC;
	CDC m_dbDC;

	CPoint m_origin;

	typedef std::shared_ptr<CBitmap> bitmap_ptr_t;

	typedef struct TEXT_ITEM
	{
		bitmap_ptr_t pBackgroundCopy;
		bitmap_ptr_t pBackgroundCopy2;
		COLORREF textColor;
		CSize textExtent;
		CString text;
		int x, y;
	};

	std::vector<TEXT_ITEM> m_textItems;

	void strictTextOut(HDC dc, TEXT_ITEM& textItem);
	void drawItemText(HDC dc, TEXT_ITEM& textItem, bool useStored = false);
	void eraseTextItem(HDC dc, TEXT_ITEM& textItem);
	void getTextSize(const TCHAR* text, CSize& size);

};

module :private;

CTextBuffer::CTextBuffer(HDC compatibleDC, int nTextItems) :
	m_textItems(nTextItems),
	m_isItalic(false),
	m_origin(0, 0)
{
	m_memDC.CreateCompatibleDC(compatibleDC);
	m_dbDC.CreateCompatibleDC(compatibleDC);
}

CTextBuffer::~CTextBuffer()
{
}

void CTextBuffer::SetTextLayerProperties
(
	const TCHAR* faceName,
	BYTE charSet,
	int fontSize,
	bool antialiased,
	bool bold,
	bool italic
)
{
	if ((HFONT)m_textLayerFont)
	{
		m_textLayerFont.DeleteObject();
	}

	// calc font height in pixels
	int ppi = m_memDC.GetDeviceCaps(LOGPIXELSY);
	int fontHeight = -MulDiv(fontSize, ppi, 72);

	// set font properties
	m_textLayerFont.CreateFont(
		fontHeight,
		0, 0, 0,
		bold ? FW_BOLD : FW_NORMAL,
		italic ? TRUE : FALSE,
		FALSE,
		0,
		charSet,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		antialiased ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		faceName
	);

	m_isItalic = italic;

}

void CTextBuffer::SetTextItem
(
	HDC dc,
	unsigned item,
	int x,
	int y,
	const TCHAR* text,
	COLORREF textColor,
	bool doublebuffered /* = true */,
	bool draw /* = false */
)
{
	ATLASSERT(item < m_textItems.size());

	if (item >= m_textItems.size())
	{
		return;
	}

	TEXT_ITEM& textItem = m_textItems[item];

	textItem.x = x;
	textItem.y = y;
	textItem.text = text;
	textItem.textColor = textColor;
	getTextSize(text, textItem.textExtent);

	// create bitmap to keep background piece under text item
	bitmap_ptr_t pBackgroundCopy = std::make_shared<CBitmap>();

	pBackgroundCopy->CreateCompatibleBitmap(dc, textItem.textExtent.cx, textItem.textExtent.cy);

	// copy backgound piece
	HBITMAP hOldBitmap = m_memDC.SelectBitmap(*pBackgroundCopy);

	CPoint origin;
	m_memDC.GetWindowOrg(&origin);

	m_memDC.BitBlt(0, 0, textItem.textExtent.cx, textItem.textExtent.cy, dc, 
		x + m_origin.x, y + m_origin.y, SRCCOPY);

	m_memDC.SelectBitmap(hOldBitmap);

	textItem.pBackgroundCopy = pBackgroundCopy;

	// create bitmap for doublebuffering
	if (doublebuffered)
	{
		textItem.pBackgroundCopy2 = std::make_shared<CBitmap>();

		textItem.pBackgroundCopy2->CreateCompatibleBitmap(
			dc, textItem.textExtent.cx, textItem.textExtent.cy);
	}
	else
	{
		textItem.pBackgroundCopy2 = bitmap_ptr_t();
	}

	if (draw)
	{
		drawItemText(dc, textItem);
	}
}

void CTextBuffer::MoveTextItem
(
	HDC dc,
	unsigned item,
	int x,
	int y,
	bool redraw /* = true */
)
{
	ATLASSERT(item < m_textItems.size());

	if (item >= m_textItems.size())
	{
		return;
	}

	TEXT_ITEM& textItem = m_textItems[item];

	if (textItem.pBackgroundCopy == NULL)
	{
		return;
	}

	// draw old backround on previous text plase
	HBITMAP hOldBitmap = m_memDC.SelectBitmap(*textItem.pBackgroundCopy);

	::BitBlt(dc, textItem.x + m_origin.x, textItem.y + m_origin.y,
		textItem.textExtent.cx, textItem.textExtent.cy, m_memDC, 0, 0, SRCCOPY);

	// copy new background piece from outer DC
	m_memDC.BitBlt(0, 0, textItem.textExtent.cx, textItem.textExtent.cy,
		dc, x + m_origin.x, y + m_origin.y, SRCCOPY);

	m_memDC.SelectBitmap(hOldBitmap);

	textItem.x = x;
	textItem.y = y;

	if (redraw)
	{
		drawItemText(dc, textItem);
	}
}

void CTextBuffer::SetItemText
(
	HDC dc,
	unsigned item,
	const TCHAR* text,
	COLORREF textColor,
	bool redraw /* = true */
)
{
	ATLASSERT(item < m_textItems.size());

	if (item >= m_textItems.size())
	{
		return;
	}

	TEXT_ITEM& textItem = m_textItems[item];

	if (textItem.pBackgroundCopy == NULL)
	{
		return;
	}

	bool textExtentChanged = false;

	if (text != NULL)
	{
		// calc new text size
		CSize newTextSize;
		getTextSize(text, newTextSize);

		textExtentChanged = newTextSize.cx > textItem.textExtent.cx
			|| newTextSize.cy > textItem.textExtent.cy;

		// refresh old background copy if new text size exceeds old one
		if (textExtentChanged)
		{
			eraseTextItem(dc, textItem);

			textItem.pBackgroundCopy->DeleteObject();

			textItem.pBackgroundCopy->CreateCompatibleBitmap(
				dc, newTextSize.cx, newTextSize.cy);

			HBITMAP hOldBitmap = m_memDC.SelectBitmap(*textItem.pBackgroundCopy);

			m_memDC.BitBlt(0, 0, newTextSize.cx, newTextSize.cy,
				dc, textItem.x + m_origin.x, textItem.y + m_origin.y, SRCCOPY);

			m_memDC.SelectBitmap(hOldBitmap);

			if (textItem.pBackgroundCopy2 != NULL)
			{
				textItem.pBackgroundCopy2->DeleteObject();

				textItem.pBackgroundCopy2->CreateCompatibleBitmap(
					dc, newTextSize.cx, newTextSize.cy);
			}
		}

		textItem.text = text;
		textItem.textExtent = newTextSize;
	}

	textItem.textColor = textColor;

	if (redraw)
	{
		if (textExtentChanged)
		{
			strictTextOut(dc, textItem);
		}
		else
		{
			drawItemText(dc, textItem);
		}
	}

}

void CTextBuffer::EraseTextItem(HDC dc, unsigned item)
{
	ATLASSERT(item < m_textItems.size());

	if (item >= m_textItems.size())
	{
		return;
	}

	TEXT_ITEM& textItem = m_textItems[item];

	if (textItem.pBackgroundCopy == NULL)
	{
		return;
	}

	eraseTextItem(dc, textItem);
}

void CTextBuffer::drawItemText
(
	HDC dc,
	TEXT_ITEM& textItem,
	bool useStored /* = false */
)
{
	if (textItem.pBackgroundCopy2 != NULL)
	{
		HBITMAP hOldDbBitmap = m_dbDC.SelectBitmap(*textItem.pBackgroundCopy2);

		if (!useStored)
		{
			HBITMAP hOldBitmap = m_memDC.SelectBitmap(*textItem.pBackgroundCopy);

			// draw stored background piece to buffer bitmap
			m_dbDC.BitBlt(0, 0, textItem.textExtent.cx, textItem.textExtent.cy,
				m_memDC, 0, 0, SRCCOPY);

			m_memDC.SelectBitmap(hOldBitmap);

			// out text to buffer bitmap
			m_dbDC.SetBkMode(TRANSPARENT);
			m_dbDC.SetTextColor(textItem.textColor);

			HFONT old_font = m_dbDC.SelectFont(m_textLayerFont);
			m_dbDC.TextOut(0, 0, textItem.text, textItem.text.GetLength());
			m_dbDC.SelectFont(old_font);
		}

		// draw buffer bitmap on outer context
		::BitBlt(dc, textItem.x + m_origin.x, textItem.y + m_origin.y,
			textItem.textExtent.cx, textItem.textExtent.cy, m_dbDC, 0, 0, SRCCOPY);

		m_dbDC.SelectBitmap(hOldDbBitmap);
	}
	else
	{
		eraseTextItem(dc, textItem);
		strictTextOut(dc, textItem);
	}

}

void CTextBuffer::eraseTextItem(HDC dc, TEXT_ITEM& textItem)
{
	HBITMAP hOldBitmap = m_memDC.SelectBitmap(*textItem.pBackgroundCopy);

	::BitBlt(dc, textItem.x + m_origin.x, textItem.y + m_origin.y,
		textItem.textExtent.cx, textItem.textExtent.cy, m_memDC, 0, 0, SRCCOPY);

	m_memDC.SelectBitmap(hOldBitmap);
}

void CTextBuffer::getTextSize(const TCHAR* text, CSize& size)
{
	CSize textSize;
	CSize lastCharSize;

	unsigned text_len = _tcslen(text);

	HFONT old_font = m_memDC.SelectFont(m_textLayerFont);

	// calculate text extent
	m_memDC.GetTextExtent(text, text_len, &textSize);

	// last char size, to fit italic font into rect
	m_memDC.GetTextExtent(text + (text_len - 1), 1, &lastCharSize);

	m_memDC.SelectFont(old_font);

	size.SetSize(textSize.cx + (m_isItalic ? (lastCharSize.cx / 2) : 0), textSize.cy);
}

void CTextBuffer::strictTextOut(HDC dc, TEXT_ITEM& textItem)
{
	int old_bk_mode = ::GetBkMode(dc);
	int old_text_color = ::GetTextColor(dc);

	::SetBkMode(dc, TRANSPARENT);
	::SetTextColor(dc, textItem.textColor);

	HGDIOBJ old_font = ::SelectObject(dc, (HGDIOBJ)(HFONT)m_textLayerFont);
	::TextOut(dc, textItem.x + m_origin.x, textItem.y + m_origin.y,
		textItem.text, textItem.text.GetLength());
	::SelectObject(dc, old_font);

	::SetBkMode(dc, old_bk_mode);
	::SetTextColor(dc, old_text_color);
}

void CTextBuffer::GetTextExtent(unsigned item, CSize& size)
{
	ATLASSERT(item < m_textItems.size());

	if (item >= m_textItems.size())
	{
		return;
	}

	size = m_textItems[item].textExtent;
}

void CTextBuffer::GetTextExtent(const TCHAR* text, CSize& size)
{
	getTextSize(text, size);
}

void CTextBuffer::RepaintTextItem
(
	HDC dc,
	unsigned item,
	bool useStored /* = true */
)
{
	ATLASSERT(item < m_textItems.size());

	if (item >= m_textItems.size())
	{
		return;
	}

	TEXT_ITEM& textItem = m_textItems[item];

	if (textItem.pBackgroundCopy == NULL)
	{
		return;
	}

	drawItemText(dc, textItem, useStored);
}

void CTextBuffer::GetBackground(HDC dc, unsigned item)
{
	ATLASSERT(item < m_textItems.size());

	if (item >= m_textItems.size())
	{
		return;
	}

	TEXT_ITEM& textItem = m_textItems[item];

	if (textItem.pBackgroundCopy == NULL)
	{
		return;
	}

	// copy backgound piece
	HBITMAP hOldBitmap = m_memDC.SelectBitmap(*textItem.pBackgroundCopy);

	m_memDC.BitBlt(0, 0, textItem.textExtent.cx, textItem.textExtent.cy,
		dc, textItem.x + m_origin.x, textItem.y + m_origin.y, SRCCOPY);

	m_memDC.SelectBitmap(hOldBitmap);
}

void CTextBuffer::SetWindowOrigin(const CPoint& origin)
{
	m_origin = origin;
}