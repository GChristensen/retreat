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

#include <atlbase.h>
#include <atlwin.h>

#include "utility/BackBuffer.h"

#define GHOST_WND_CLASS _T("GhostWindow")

typedef CWinTraits<0,  WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW>
			GhostTraits;

class CGhostWnd: public CWindowImpl<CGhostWnd, CWindow, GhostTraits>
{
	const static int GW_DEFAULT_BACKGROUND_COLOR = RGB(255, 255, 255);

public:

	CGhostWnd(HWND parent, CRect *pRect, CRect *pWorkArea);
	~CGhostWnd();

	// option determines if there should be totally transparent regions 
	// in the window
	void SetTransparentColor(COLORREF crTransparentColor);
	void SetTransparentColorFromImageLeftUpperPixel();

	void SetAlpha(BYTE alpha);
	void AlterAlpha(int delta);
	BYTE GetAlpha();
	
	void ApplyLayeredWindowAttributes();

	bool LoadBackground(const TCHAR *name, bool stretch, COLORREF canvas);

	void PlaceWindowOnWorkArea(int x, int y);
	void CenterWindowOnWorkArea();
	void RandomlyPlaceWindowOnWorkArea();

	//////////////////////////////////////////////////////////////////////
	// Messages
	//////////////////////////////////////////////////////////////////////

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	DECLARE_WND_CLASS_EX(GHOST_WND_CLASS, 0, NULL)
	
	BEGIN_MSG_MAP(CGhostWnd)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	END_MSG_MAP()

protected:

	BYTE m_bAlpha;
	bool m_isUsingTransparency;
	
	bool m_isFullScreen;

	CRect m_displayRect;
	CRect m_workAreaRect;
	CBackBuffer *m_pBackBuffer;

	COLORREF m_transparentColor;

	virtual CBackBuffer *getBackBuffer(CDC *pDC);

	virtual void postPanitHook(HDC dc);

	// make image dimensions to fit viewport dimensions
	// image bounds ratio will be preserved
	static void AdjustDimensions(const CSize &bounds, const CSize &oldSize, 
		CSize &newSize);

	// image dimensions should not exceed specified size
	static void AdjustImageToVPort(CBackBuffer *pBackBuffer, 
							const CSize &vpSize, bool stretch = false);

	void AdjustToVPort(CBackBuffer *pBackBuffer);
	void AdjustToScreen(CBackBuffer *pBackBuffer, bool stretch, COLORREF canvas);

	// random number in range from min_ to max_ - 1
	static int randRange(int min_, int max_);

};