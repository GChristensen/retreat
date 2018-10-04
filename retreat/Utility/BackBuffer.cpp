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

#define _USE_MATH_DEFINES
#include "StdAfx.h"
#include "BackBuffer.h"

#include <cmath>
#include <cassert>

#include <set>
#include <list>
#include <vector>
#include <algorithm>

#include "boost/lambda/lambda.hpp"
#include "boost/lambda/bind.hpp"
#include "boost/shared_ptr.hpp"

#define DEFAULT_BPP 32
#define SLICE_SIDE  120

#define round_int(x) ((int)floor((x) + 0.5))

CBackBuffer::CBackBuffer(HDC compatibleDC):
m_hOldBitmap(NULL)
{
	m_memDC.CreateCompatibleDC(compatibleDC);
}

CBackBuffer::~CBackBuffer()
{
	if (m_hOldBitmap)
		DeselectFromInternalDC();
}

HRESULT CBackBuffer::LoadFromFile(const TCHAR *fileName)
{
	assert(m_hOldBitmap == NULL);

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

	int nWidth = 
		MulDiv(hmWidth, m_memDC.GetDeviceCaps(LOGPIXELSX), HIMETRIC_INCH);
	int nHeight = 
		MulDiv(hmHeight, m_memDC.GetDeviceCaps(LOGPIXELSY), HIMETRIC_INCH);

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

int CBackBuffer::GetHeight()
{
	return m_imageSize.cy;
}

int CBackBuffer::GetWitdth()
{
	return m_imageSize.cx;
}

const CSize &CBackBuffer::GetSize()
{
	return m_imageSize;
}

void CBackBuffer::SetPlotRect(const CRect &rect)
{
	m_plotRect.CopyRect(rect);
}

void CBackBuffer::CreateCanvas(int width, int height, COLORREF color)
{
	assert(m_hOldBitmap == NULL);

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

		m_memDC.BitBlt(
			xDest, yDest, contentSize.cx, contentSize.cy, memDC, 0, 0, SRCCOPY);

		memDC.SelectBitmap(hOldBitmap);

		::DeleteObject(hContent);
	}

	m_memDC.SelectBitmap(hOldBitmap);

}

void CBackBuffer::Resize(const CSize &newSize)
{
	assert(m_hOldBitmap == NULL);

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
	resizeDC.StretchBlt(0, 0, newSize.cx, newSize.cy,
		m_memDC, 0, 0, m_imageSize.cx, m_imageSize.cy, SRCCOPY);

	resizeDC.SelectBitmap(resizeDCOldBitmap);
	m_memDC.SelectBitmap(hOldBitmap);

	m_imageSize.SetSize(newSize.cx, newSize.cy);
	
	::DeleteObject(Detach());
	Attach(resizedBitmap.Detach());
}

HDC CBackBuffer::SelectToInternalDC()
{
	if (!m_hOldBitmap)
		m_hOldBitmap = m_memDC.SelectBitmap(m_hBitmap);

	return m_memDC.m_hDC;
}

void CBackBuffer::DeselectFromInternalDC()
{
	if (m_hOldBitmap)
		m_memDC.SelectBitmap(m_hOldBitmap);

	m_hOldBitmap = NULL;
}

void CBackBuffer::Draw(HDC dc)
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

// zvezdochki
void CBackBuffer::StarrySky(int cx, int cy, COLORREF background)
{
	assert(m_hOldBitmap == NULL);

	if (m_hBitmap)
		DeleteObject();

	CreateBitmap(cx, cy, 1, DEFAULT_BPP, NULL);

	m_imageSize.SetSize(cx, cy);

	HBITMAP hOldBitmap = m_memDC.SelectBitmap(m_hBitmap);

	CBrush back;
	back.CreateSolidBrush(background);
	m_memDC.FillRect(CRect(CPoint(0, 0), CSize(cx + 1, cy + 1)), back);

	COLORREF colors[] =
	{
		RGB( 31, 176, 255),
		RGB(141, 255,  65),
		RGB(252,  75, 105),
		RGB(189, 161, 255),
		RGB(243, 159,  58),
		RGB(250, 255,  91),
		RGB(255, 173, 207)
	};

	int slices_x = cx / SLICE_SIDE;
	int slices_y = cy / SLICE_SIDE;
	int slice_cx = cx / slices_x;
	int slice_cy = cy / slices_y;

	for (int i = 0; i < slices_x; ++i)
		for (int j = 0; j < slices_y; ++j)
		{
			drawStar(
				m_memDC,
				CPoint(
					randRange(slice_cx * i, slice_cx * (i + 1)),
					randRange(slice_cy * j, slice_cy * (j + 1))
					),
				randRange(20, 90),
				rand() % 360,
				colors[rand() % 7]
				);
		}

	m_memDC.SelectBitmap(hOldBitmap);
}

// draws star with circumcircle radius *r*, at center point *c*, initial vertex
// angle *ang*, filled by color *color* on appropriate DC *dc*
void CBackBuffer::drawStar(HDC dc, CPoint c, int r, int ang, COLORREF color)
{
	const int vertexes = 10;
	const double rad = M_PI / 180;
	const double rcp_fi2 = 0.38197; // 1/fi^2, magic constant (fi - golden ratio)
	const int section = 360 / vertexes;

	int inner_cricle = round_int(r * rcp_fi2);

	POINT points[vertexes + 1];

	// find vertexes of star
	for (int i = 0; i < vertexes; ++i)
	{
		double pt_angle = ((ang + i * section) % 360) * rad;
		int pt_r = (i % 2 == 0)? r: inner_cricle;
		
		points[i].x = round_int(pt_r * cos(pt_angle)) + c.x;
		points[i].y = round_int(pt_r * sin(pt_angle)) + c.y;
	}
	
	points[vertexes] = points[0];

	fillPolygon(dc, points, vertexes + 1, color);

	// stroke star
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, color);
	HGDIOBJ old_obj = ::SelectObject(dc, pen);

	Polyline(dc, points, vertexes + 1);

	::SelectObject(dc, old_obj);
}

// variation of fast scan-conversion fill algorithm
void CBackBuffer::fillPolygon(HDC dc, POINT* points, int npoints, COLORREF color)
{
	using namespace std;
	using namespace boost::lambda;
	using boost::shared_ptr;

	list<EDGE> edges;

	assert(npoints);
	int first_line = points[0].y;
	int last_line = points[0].y;

	// find edges
	for (int i = 0; i < npoints - 1; ++i)
	{
		EDGE edge;
		double Xu;

		int next = i + 1;

		if (points[i].y < points[next].y)
		{
			edge.Yl = points[i].y;
			edge.Yu = points[next].y;
			edge.Xl = points[i].x;
			Xu = points[next].x;
		}
		else
		{
			edge.Yl = points[next].y;
			edge.Yu = points[i].y;
			edge.Xl = points[next].x;
			Xu = points[i].x;
		}

		if (edge.Yl < first_line)
			first_line = edge.Yl;

		if (edge.Yu > last_line)
			last_line = edge.Yu;

		if (!edge.horisontal()) 
			// reciprocal slope of edge
			edge.w = (Xu - edge.Xl) / (edge.Yu - edge.Yl);

		edges.push_back(edge);
	}

	int lines = last_line - first_line + 1;

	typedef list<EDGE> edge_list_t;
	typedef shared_ptr<edge_list_t> edge_list_ptr_t;
	vector<edge_list_ptr_t> edge_table(lines);

	edge_list_t active_edge_table;

	// fill edge table
	foreach(EDGE &edge, edges)
	{
		if (!edge.horisontal())
		{
			int y = edge.Yl - first_line;

			edge_list_ptr_t edge_list = edge_table[y];

			if (edge_list == NULL)
			{
				edge_list = edge_list_ptr_t(new edge_list_t);
				edge_table[y] = edge_list;
			}

			edge_list->insert(edge_list->end(), edge);
		}
	}

	CPen pen;
	pen.CreatePen(PS_SOLID, 1, color);

	HGDIOBJ old_obj = ::SelectObject(dc, pen);

	for (int y = 0; y < lines; ++y)
	{
		edge_list_ptr_t edge_list = edge_table[y];
		int scanline = y + first_line;

		// add edges to active edge table
		if (edge_list != NULL)
		{
			active_edge_table.insert(
				active_edge_table.end(),
				edge_list->begin(), 
				edge_list->end()
				);

			// sort by x
			active_edge_table.sort(EDGE::edge_x_sort_p);

			// then put w in ascending order within elements with equal x
			// actually these two steps are some kind of bucket sort
			// probably it's possible to perform bucket sort with one operation
			// using stl
			stable_sort(
				active_edge_table.begin(), 
				active_edge_table.end(),
				EDGE::edge_xw_sort_p
				);
		}

		using namespace boost::lambda;
		// remove rendered edges from active edge table
		active_edge_table.remove_if(boost::lambda::bind(&EDGE::Yu, _1) == scanline);

		if (active_edge_table.empty())
		{
			break;
		}

		edge_list_t::iterator current = active_edge_table.begin();
		edge_list_t::iterator next = ++active_edge_table.begin();
		int k = 0;

		// draw scanline parts
		do 
		{
			if (k++ % 2 == 0)
			{
				::MoveToEx(dc, round_int(current->Xl), scanline, NULL);
				::LineTo(dc, round_int(next->Xl), scanline);
			}

			current = next;
			++next;
		} while (next != active_edge_table.end());

		// increment current x value for each edge
		foreach (EDGE &edge, active_edge_table)
		{
			edge.Xl += edge.w;
		}
	}

	::SelectObject(dc, old_obj);
}
