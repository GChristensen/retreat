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

#include "stdafx.h"

#include "GhostWnd.h"

CGhostWnd::CGhostWnd(HWND parent, CRect *pRect, CRect *pWorkArea):
m_isUsingTransparency(false),
m_isFullScreen(pWorkArea == NULL),
m_bAlpha(255)
{
	Create(parent, pRect, NULL, m_isFullScreen? WS_MAXIMIZE: 0);
	ModifyStyle(WS_CAPTION, 0, SWP_FRAMECHANGED);

	if (pWorkArea)
	{
		m_workAreaRect.CopyRect(*pWorkArea);
	}

	if (pRect)
		m_displayRect.CopyRect(pRect);

	CClientDC clientDC(m_hWnd);
	m_pBackBuffer = getBackBuffer(&clientDC);
}

CGhostWnd::~CGhostWnd()
{
	delete m_pBackBuffer;
}

void CGhostWnd::SetTransparentColor(COLORREF crTransparentColor)
{
	m_isUsingTransparency = true;
	m_transparentColor = crTransparentColor;
}

void CGhostWnd::SetTransparentColorFromImageLeftUpperPixel()
{
	if (m_pBackBuffer->m_hBitmap != NULL)
	{
		m_isUsingTransparency = true;

		m_transparentColor = 
			::GetPixel(m_pBackBuffer->SelectToInternalDC(), 0, 0);

		m_pBackBuffer->DeselectFromInternalDC();
	}
}

void CGhostWnd::SetAlpha(BYTE alpha)
{
	m_bAlpha = alpha;
}

void CGhostWnd::ApplyLayeredWindowAttributes()
{
	DWORD flags = LWA_ALPHA | (m_isUsingTransparency? LWA_COLORKEY: 0);
	SetLayeredWindowAttributes(m_hWnd, m_transparentColor, m_bAlpha, flags);
}

BYTE CGhostWnd::GetAlpha()
{
	return m_bAlpha;
}

void CGhostWnd::AlterAlpha(int delta)
{
	if (m_bAlpha + delta < 0)
	{
		m_bAlpha = 0;
	}
	else if (m_bAlpha + delta > 255)
	{
		m_bAlpha = 255;
	}
	else
	{
		m_bAlpha += delta;
	}

	DWORD flags = LWA_ALPHA | (m_isUsingTransparency? LWA_COLORKEY: 0);
	SetLayeredWindowAttributes(m_hWnd, m_transparentColor, m_bAlpha, flags);
}

bool CGhostWnd::LoadBackground(const TCHAR *name, bool stretch, COLORREF canvas)
{
	if (name && *name && SUCCEEDED(m_pBackBuffer->LoadFromFile(name)))
	{
		if (m_isFullScreen)
		{
			AdjustToScreen(m_pBackBuffer, stretch, canvas);
		}
		else
		{
			AdjustToVPort(m_pBackBuffer);
		}

		return true;
	}

	return false;
}

void CGhostWnd::PlaceWindowOnWorkArea(int x, int y)
{
	if (m_isFullScreen)
	{
		return;
	}

	SetWindowPos(
		NULL, 
		m_workAreaRect.left + x, 
		m_workAreaRect.top + y, 
		0, 0, SWP_NOZORDER | SWP_NOSIZE
		);
}

void CGhostWnd::CenterWindowOnWorkArea()
{
	if (m_isFullScreen)
	{
		return;
	}

	CRect clientRect;
	GetClientRect(&clientRect);

	SetWindowPos(
		NULL, 
		m_workAreaRect.left 
		+ (m_workAreaRect.Width() - clientRect.Width()) / 2,
		m_workAreaRect.top
		+ (m_workAreaRect.Height() - clientRect.Height()) / 2,
		0, 0, SWP_NOZORDER | SWP_NOSIZE
		);
}

void CGhostWnd::RandomlyPlaceWindowOnWorkArea()
{
	if (m_isFullScreen)
	{
		return;
	}

	CRect clientRect;
	GetClientRect(&clientRect);

	int x = randRange(
		m_workAreaRect.left, 
		m_workAreaRect.right - clientRect.Width()
		);

	int y = randRange(
		m_workAreaRect.top, 
		m_workAreaRect.bottom - clientRect.Height()
		);

	SetWindowPos(NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

LRESULT CGhostWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	DestroyWindow();
	return 0;
}

LRESULT CGhostWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return 0;
}

LRESULT CGhostWnd::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return 0;
}

LRESULT CGhostWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	CPaintDC dc(m_hWnd);

	m_pBackBuffer->Draw(dc);

	postPanitHook(dc);

	return 0;
}

LRESULT CGhostWnd::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return 0;
}

CBackBuffer * CGhostWnd::getBackBuffer(CDC *pDC)
{
	return new CBackBuffer(pDC->m_hDC);
}

void CGhostWnd::postPanitHook(HDC dc)
{

}

void CGhostWnd::AdjustDimensions
	(
	 const CSize &bounds, 
	 const CSize &oldSize, 
	 CSize &newSize
	)
{
	int viewPortWidth = bounds.cx;
	int viewPortHeight = bounds.cy;

	int imageWidth = oldSize.cx;
	int imageHeight = oldSize.cy;

	// imageWidth1 - image width corrected value in case when image height 
	// is equal to view port height
	int imageWidth1 = MulDiv(imageWidth, viewPortHeight, imageHeight);

	// imageHeight1 - image height corrected value in case when image width
	// is equal to view port width
	int imageHeight1 = MulDiv(imageHeight, viewPortWidth, imageWidth);

	// guess that height of resized image is equal to view port height
	if (imageWidth1 <= viewPortWidth)
	{ // right choice
		imageHeight1 = viewPortHeight;
	}
	else
	{ // make resized image width equal to view port width then
		imageWidth1 = viewPortWidth;
	}

	newSize.cx = imageWidth1;
	newSize.cy = imageHeight1;
}

void CGhostWnd::AdjustImageToVPort
	(
	 CBackBuffer *pBackBuffer, 
	 const CSize &vpSize, 
	 bool stretch /*= false*/
	)
{
	int imageWidth = pBackBuffer->GetWitdth();
	int imageHeight = pBackBuffer->GetHeight();

	int vpWidth = vpSize.cx;
	int vpHeight = vpSize.cy;

	CSize imageNewSize(imageWidth, imageHeight);

	// get image new size if it should be stretched or if image dimensions
	// do not fit work area
	if (   stretch
		|| imageWidth > vpWidth
		|| imageHeight > vpHeight)
	{
		AdjustDimensions(
			CSize(vpWidth, vpHeight),
			CSize(imageWidth, imageHeight),
			imageNewSize);
	}

	pBackBuffer->Resize(imageNewSize);
}

void CGhostWnd::AdjustToVPort(CBackBuffer *pBackBuffer)
{
	CSize targetSize;
	targetSize.SetSize(m_workAreaRect.Width(), m_workAreaRect.Height());
	AdjustImageToVPort(pBackBuffer, targetSize, false);

	SetWindowPos(NULL, 0, 0, pBackBuffer->GetWitdth(),
		pBackBuffer->GetHeight(), SWP_NOZORDER | SWP_NOMOVE);

	pBackBuffer->SetPlotRect(
		CRect(
		0, 0, 
		pBackBuffer->GetWitdth(), 
		pBackBuffer->GetHeight()
		)
		);
}

void CGhostWnd::AdjustToScreen
	(
	 CBackBuffer *pBackBuffer, 
	 bool stretch, 
	 COLORREF canvas
	)
{
	CRect windowRect;
	GetWindowRect(windowRect);

	// fullscreen window rect top left coordinates are negative
	CSize targetSize(m_displayRect.Width(),	m_displayRect.Height());
	AdjustImageToVPort(pBackBuffer, targetSize, stretch);

	// create canvas if image does not fills entrie screen in fullscreen
	// mode
	if (pBackBuffer->GetSize() != targetSize)
	{
		pBackBuffer->CreateCanvas(targetSize.cx, targetSize.cy, canvas);
	}

	int heightDelta = (windowRect.Height() - m_displayRect.Height()) / 2;
	int widthDelta = (windowRect.Width() - m_displayRect.Width()) / 2;

	windowRect.top = heightDelta;
	windowRect.left = heightDelta;
	windowRect.bottom = m_displayRect.Height() + heightDelta;
	windowRect.right = m_displayRect.Width() + widthDelta;

	pBackBuffer->SetPlotRect(windowRect);
}

int CGhostWnd::randRange(int min_, int max_)
{
	return min_ + rand() % (max_ - min_);
}