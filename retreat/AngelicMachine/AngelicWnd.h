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

#include <exception>

#include "boost/shared_ptr.hpp"

#include "../GhostWnd.h"
#include "../utility/LayeredTextBuffer.h"

// this is highly specialized window
// it can show background image with timer or only timer

class CAngelicWnd: public CGhostWnd
{
	static const COLORREF SPECIAL_BACKGORUND_COLOR = RGB(254, 0, 254);
	static const int ANGELIC_WINDOW_CONTROL_WM = WM_USER + 328;
	static const int TIMER_TEXT_ITEM = 0;

public:

	enum
	{
		WM_ANGELIC_WND_NOTIFY = ANGELIC_WINDOW_CONTROL_WM + 1
	};

	enum SPECIAL
	{
		STARS
	};

	CAngelicWnd(HWND parent, CRect *pRect, CRect *pWorkArea);

	~CAngelicWnd();

	void SetTimerProperties
		(
		 const TCHAR *faceName,
		 BYTE charSet,
		 int fontSize,
		 COLORREF textColor,
		 bool antialiased,
		 bool bold,
		 bool italic 
		);

	void SetTimer(int seconds);
	void SetShowTimer(bool show);
	void SetTimerPos(int x, int y);

	bool LoadBackground(const TCHAR *name, bool stretch, COLORREF canvas);
	void SpecialBackground(int index);

	void GrabUserInput();
	void ReturnUserInput();

	void DoEvents();

	//////////////////////////////////////////////////////////////////////
	// Messages
	//////////////////////////////////////////////////////////////////////

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnExternalNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);

	BEGIN_MSG_MAP(CAngelicWnd)
		MESSAGE_HANDLER(WM_ANGELIC_WND_NOTIFY, OnExternalNotify)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		CHAIN_MSG_MAP(CGhostWnd)
	END_MSG_MAP()

protected:

	bool m_backgroundLoaded;
	bool m_showTimer;

	typedef boost::shared_ptr<CLayeredTextBuffer> text_buff_ptr_t;
	text_buff_ptr_t m_pTexBuffer;
	COLORREF m_textColor;
	CPoint m_timerPos;

	int m_duration;

	HWND m_hParentWnd;
	HWND m_foregroundWnd;

	virtual void postPanitHook(HDC dc);

	void setTimer(int seconds);

	void adjustWindowSize(text_buff_ptr_t buffer, unsigned item);

	WTL::CString parseSeconds(int seconds);
};