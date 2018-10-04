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

#pragma once

#include <vector>
#include "boost/smart_ptr.hpp"

#include <atlstr.h>
#include <atlmisc.h>

class CLayeredTextBuffer
{
public:

	CLayeredTextBuffer(HDC compatibleDC, int nTextItems);
	~CLayeredTextBuffer();

	void SetTextLayerProperties
		(
		 const TCHAR *faceName,
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
		 const TCHAR *text, 
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
		 const TCHAR *text,
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

	void GetTextExtent(unsigned item, CSize &size);
	void GetTextExtent(const TCHAR *text, CSize &size);

	void SetWindowOrg(const CPoint &org);

private:

	bool m_isItalic;

	CFont m_textLayerFont;
	CDC m_memDC;
	CDC m_dbDC;

	CPoint m_origin;

	typedef boost::shared_ptr<CBitmap> bitmap_ptr_t;

	typedef struct TEXT_ITEM
	{
		bitmap_ptr_t pBackgroundCopy;
		bitmap_ptr_t pBackgroundCopy2;
		COLORREF textColor;
		CSize textExtent;
		WTL::CString text;
		int x, y;
	};

	std::vector<TEXT_ITEM> m_textItems;

	void strictTextOut(HDC dc, TEXT_ITEM &textItem);
	void drawItemText(HDC dc, TEXT_ITEM &textItem,	bool useStored = false);
	void eraseTextItem(HDC dc, TEXT_ITEM &textItem);
	void getTextSize(const TCHAR *text, CSize &size);
	
};
