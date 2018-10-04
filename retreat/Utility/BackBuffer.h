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

#include <atlgdi.h>
#include <atlmisc.h>

class CBackBuffer: public CBitmap
{
public:
	CBackBuffer(HDC compatibleDC);
	virtual ~CBackBuffer();

	HRESULT LoadFromFile(const TCHAR *name);

	int GetHeight();
	int GetWitdth();
	const CSize &GetSize();

	void SetPlotRect(const CRect &rect);

	void CreateCanvas(int width, int height, COLORREF color);
	void Resize(const CSize &newSize);
	void Draw(HDC dc);

	HDC SelectToInternalDC();
	void DeselectFromInternalDC();

	void StarrySky(int cx, int cy, COLORREF background);

protected:

	CSize m_imageSize;
	CDC m_memDC;
	
	HBITMAP m_hOldBitmap;

	CRect m_plotRect;

	struct EDGE
	{
		int Yl;		// lower y point
		int Yu;		// upper y point
		double Xl;	// current x intersection
		double w;	// 1/slope of edge

		bool horisontal() {return Yl == Yu;}

		static bool edge_x_sort_p(const EDGE &e1, const EDGE &e2)
		{
			return e1.Xl < e2.Xl;
		}

		static bool edge_xw_sort_p(const EDGE &e1, const EDGE &e2)
		{
			return e1.Xl <= e2.Xl && e1.w < e2.w;
		}
	};

	void drawStar(HDC dc, CPoint c, int r, int ang, COLORREF color);
	void fillPolygon(HDC dc, POINT* points, int npoints, COLORREF color);
	
	// random number in range from min_ to max_ - 1
	static int randRange(int min_, int max_)
	{
		return min_ + rand() % (max_ - min_);
	}
};
