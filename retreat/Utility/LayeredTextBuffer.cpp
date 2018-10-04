/*
 * Angelic Retreat
 * Copyright (C) 2008 G. Christensen
 * 
 * Angelic Retreat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Angelic Retreat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "StdAfx.h"
#include "LayeredTextBuffer.h"

CLayeredTextBuffer::CLayeredTextBuffer(HDC compatibleDC, int nTextItems):
m_textItems(nTextItems),
m_isItalic(false),
m_origin(0, 0)
{
	m_memDC.CreateCompatibleDC(compatibleDC);
	m_dbDC.CreateCompatibleDC(compatibleDC);
}

CLayeredTextBuffer::~CLayeredTextBuffer()
{
}

void CLayeredTextBuffer::SetTextLayerProperties
	(
	 const TCHAR *faceName,
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
	int fontHeight = -MulDiv(fontSize,	ppi, 72);

	// set font properties
	m_textLayerFont.CreateFont(
		fontHeight, 
		0, 0, 0, 
		bold? FW_BOLD: FW_NORMAL, 
		italic? TRUE: FALSE, 
		FALSE, 
		0, 
		charSet, 
		OUT_DEFAULT_PRECIS, 
		CLIP_DEFAULT_PRECIS, 
		antialiased? ANTIALIASED_QUALITY: NONANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, 
		faceName
		);

	m_isItalic = italic;

}

void CLayeredTextBuffer::SetTextItem
	(
	 HDC dc,
	 unsigned item, 
	 int x, 
	 int y, 
	 const TCHAR *text, 
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

	TEXT_ITEM &textItem = m_textItems[item];

	textItem.x = x;
	textItem.y = y;
	textItem.text = text;
	textItem.textColor = textColor;
	getTextSize(text, textItem.textExtent);
	
	// create bitmap to keep background piece under text item
	bitmap_ptr_t pBackgroundCopy = bitmap_ptr_t(new CBitmap);
	
	pBackgroundCopy->CreateCompatibleBitmap(
		dc, textItem.textExtent.cx, textItem.textExtent.cy);

	// copy backgound piece
	HBITMAP hOldBitmap = m_memDC.SelectBitmap(*pBackgroundCopy);
	
	CPoint origin;
	m_memDC.GetWindowOrg(&origin);

	m_memDC.BitBlt(0, 0, textItem.textExtent.cx, textItem.textExtent.cy,
		dc, x + m_origin.x, y + m_origin.y, SRCCOPY);

	m_memDC.SelectBitmap(hOldBitmap);

	textItem.pBackgroundCopy = pBackgroundCopy;

	// create bitmap for doublebuffering
	if (doublebuffered)
	{
		textItem.pBackgroundCopy2 = bitmap_ptr_t(new CBitmap);

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

void CLayeredTextBuffer::MoveTextItem
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

	TEXT_ITEM &textItem = m_textItems[item];

	if (textItem.pBackgroundCopy == NULL)
	{
		return;
	}

	// draw old backround on previous text plase
	HBITMAP hOldBitmap = m_memDC.SelectBitmap(*textItem.pBackgroundCopy);

	::BitBlt(dc, textItem.x + m_origin.x, textItem.y + m_origin.y, 
		textItem.textExtent.cx, textItem.textExtent.cy,	m_memDC, 0, 0, SRCCOPY);

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

void CLayeredTextBuffer::SetItemText
	(
	 HDC dc,
	 unsigned item, 
	 const TCHAR *text,
	 COLORREF textColor,
	 bool redraw /* = true */
	)
{
	ATLASSERT(item < m_textItems.size());

	if (item >= m_textItems.size())
	{
		return;
	}

	TEXT_ITEM &textItem = m_textItems[item];

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

		textExtentChanged =    newTextSize.cx > textItem.textExtent.cx
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

void CLayeredTextBuffer::EraseTextItem(HDC dc, unsigned item)
{
	ATLASSERT(item < m_textItems.size());

	if (item >= m_textItems.size())
	{
		return;
	}

	TEXT_ITEM &textItem = m_textItems[item];

	if (textItem.pBackgroundCopy == NULL)
	{
		return;
	}

	eraseTextItem(dc, textItem);
}

void CLayeredTextBuffer::drawItemText
	(
	 HDC dc, 
	 TEXT_ITEM &textItem, 
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
		::BitBlt(dc, textItem.x + m_origin.x, textItem.y  + m_origin.y, 
			textItem.textExtent.cx, textItem.textExtent.cy, m_dbDC, 0, 0, SRCCOPY);

		m_dbDC.SelectBitmap(hOldDbBitmap);
	}
	else
	{
		eraseTextItem(dc, textItem);
		strictTextOut(dc, textItem);
	}

}

void CLayeredTextBuffer::eraseTextItem(HDC dc, TEXT_ITEM &textItem)
{
	HBITMAP hOldBitmap = m_memDC.SelectBitmap(*textItem.pBackgroundCopy);

	::BitBlt(dc, textItem.x + m_origin.x, textItem.y + m_origin.y, 
		textItem.textExtent.cx,	textItem.textExtent.cy, m_memDC, 0, 0, SRCCOPY);

	m_memDC.SelectBitmap(hOldBitmap);
}

void CLayeredTextBuffer::getTextSize(const TCHAR *text, CSize &size)
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

	size.SetSize(
		textSize.cx + (m_isItalic? (lastCharSize.cx / 2): 0), textSize.cy);
}

void CLayeredTextBuffer::strictTextOut(HDC dc, TEXT_ITEM &textItem)
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

void CLayeredTextBuffer::GetTextExtent(unsigned item, CSize &size)
{
	ATLASSERT(item < m_textItems.size());

	if (item >= m_textItems.size())
	{
		return;
	}

	size = m_textItems[item].textExtent;
}

void CLayeredTextBuffer::GetTextExtent(const TCHAR *text, CSize &size)
{
	getTextSize(text, size);
}

void CLayeredTextBuffer::RepaintTextItem
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

	TEXT_ITEM &textItem = m_textItems[item];

	if (textItem.pBackgroundCopy == NULL)
	{
		return;
	}

	drawItemText(dc, textItem, useStored);
}

void CLayeredTextBuffer::GetBackground(HDC dc, unsigned item)
{
	ATLASSERT(item < m_textItems.size());

	if (item >= m_textItems.size())
	{
		return;
	}

	TEXT_ITEM &textItem = m_textItems[item];

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

void CLayeredTextBuffer::SetWindowOrg(const CPoint &org)
{
	m_origin = org;
}