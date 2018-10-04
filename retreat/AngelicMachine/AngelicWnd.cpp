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

#include "AngelicWnd.h"

CAngelicWnd::CAngelicWnd(HWND parent, CRect *pRect, CRect *pWorkArea) :
CGhostWnd(parent, pRect, pWorkArea),
m_backgroundLoaded(false),
m_hParentWnd(parent),
m_showTimer(true),
m_timerPos(0, 0)
{
	CClientDC dc(m_hWnd);

	m_pTexBuffer = text_buff_ptr_t(new CLayeredTextBuffer(dc, 1));

	m_textColor = 0;
}

CAngelicWnd::~CAngelicWnd()
{
	ATLTRACE(_T("AngelicWnd has gone...\n"));
}

void CAngelicWnd::SetTimerProperties
	(
	 const TCHAR *faceName,
	 BYTE charSet,
	 int fontSize,
	 COLORREF textColor,
	 bool antialiased,
	 bool bold,
	 bool italic 
	)
{
	m_pTexBuffer->SetTextLayerProperties(
		faceName,
		charSet,
		fontSize,
		antialiased,
		bold,
		italic
		);

	m_textColor = textColor;
}

void CAngelicWnd::SetTimer(int seconds)
{
	setTimer(seconds);
}

void CAngelicWnd::SetShowTimer(bool show)
{
	m_showTimer = show;
}

void CAngelicWnd::SetTimerPos(int x, int y)
{
	m_timerPos.SetPoint(x, y);

	CClientDC dc(m_hWnd);
	m_pTexBuffer->MoveTextItem(dc, TIMER_TEXT_ITEM, x, y, false);
}

bool CAngelicWnd::LoadBackground(const TCHAR *name, bool stretch, COLORREF canvas)
{
	m_backgroundLoaded = CGhostWnd::LoadBackground(name, stretch, canvas);
	return m_backgroundLoaded;
}

void CAngelicWnd::SpecialBackground(int index)
{
	CRect windowRect;
	GetWindowRect(&windowRect);

	m_pBackBuffer->StarrySky(
		m_displayRect.Width(), m_displayRect.Height(), SPECIAL_BACKGORUND_COLOR);

	int heightDelta = (windowRect.Height() - m_displayRect.Height()) / 2;
	int widthDelta = (windowRect.Width() - m_displayRect.Width()) / 2;

	windowRect.top = heightDelta;
	windowRect.left = heightDelta;
	windowRect.bottom = m_displayRect.Height() + heightDelta;
	windowRect.right = m_displayRect.Width() + widthDelta;

	m_pBackBuffer->SetPlotRect(windowRect);

	SetTransparentColor(SPECIAL_BACKGORUND_COLOR);

	m_backgroundLoaded = true;
}

void CAngelicWnd::GrabUserInput()
{
	m_foregroundWnd = ::GetForegroundWindow();

	int foregroundWndThreadID = 
		::GetWindowThreadProcessId(m_foregroundWnd, NULL);

	int currentThreadID = ::GetCurrentThreadId();

	BOOL success = 
		::AttachThreadInput(currentThreadID, foregroundWndThreadID, TRUE);

	if (success)
	{
		::SetForegroundWindow(m_hWnd);
		::AttachThreadInput(currentThreadID, foregroundWndThreadID, FALSE);
	}
}

void CAngelicWnd::ReturnUserInput()
{
	int foregroundWndThreadID = 
		::GetWindowThreadProcessId(m_foregroundWnd, NULL);
	int currentThreadID = ::GetCurrentThreadId();

	::AttachThreadInput(currentThreadID, foregroundWndThreadID, TRUE);

	::SetForegroundWindow(m_foregroundWnd);

	::AttachThreadInput(currentThreadID, foregroundWndThreadID, FALSE);
}

LRESULT CAngelicWnd::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == VK_ADD)
	{
		AlterAlpha(10);
	}
	else if (wParam == VK_SUBTRACT)
	{
		AlterAlpha(-10);
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CAngelicWnd::OnExternalNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	m_duration -= 1;

	if (m_duration < 0)
	{
		return 0;
	}

	CClientDC dc(m_hWnd);

	m_pTexBuffer->SetItemText(dc, TIMER_TEXT_ITEM, parseSeconds(m_duration),
		m_textColor, true);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void CAngelicWnd::postPanitHook(HDC dc)
{
	if (m_showTimer)
	{
		m_pTexBuffer->RepaintTextItem(dc, TIMER_TEXT_ITEM);
	}
}

void CAngelicWnd::setTimer(int seconds)
{
	m_duration = seconds;

	WTL::CString timerText = parseSeconds(seconds);

	CSize textSize;
	m_pTexBuffer->GetTextExtent(timerText, textSize);

	// resize window if it's not fit timer
	// not resize when in fullscreen mode
	// resize only when no background and window not fits timer
	if (   !m_isFullScreen && !m_backgroundLoaded 
		&& (   m_pBackBuffer->GetWitdth() < textSize.cx
		|| m_pBackBuffer->GetHeight() < textSize.cy))
	{
		SetWindowPos(NULL, 0, 0, textSize.cx, textSize.cy, 
			SWP_NOMOVE | SWP_NOZORDER);

		m_pBackBuffer->CreateCanvas(
			textSize.cx, textSize.cy, SPECIAL_BACKGORUND_COLOR);

		SetTransparentColor(SPECIAL_BACKGORUND_COLOR);
	}

	CClientDC dc(m_hWnd);

	m_pTexBuffer->SetTextItem(
		dc,
		TIMER_TEXT_ITEM,
		m_timerPos.x,
		m_timerPos.y,
		timerText,
		m_textColor
		);

	m_pTexBuffer->GetBackground(
		m_pBackBuffer->SelectToInternalDC(), TIMER_TEXT_ITEM);
	m_pBackBuffer->DeselectFromInternalDC();

	m_pTexBuffer->RepaintTextItem(dc, TIMER_TEXT_ITEM, false);

	// window top-left point coordinates are negative in fullscreen mode
	if (m_isFullScreen)
	{
		CRect rect;
		GetWindowRect(rect);

		if (rect.top < 0 || rect.left < 0)
		{
			m_pTexBuffer->SetWindowOrg(CPoint(-rect.top, -rect.left));
		}
	}
}

void CAngelicWnd::adjustWindowSize(text_buff_ptr_t buffer, unsigned item)
{
	CSize textSize;

	buffer->GetTextExtent(item, textSize);

	SetWindowPos(NULL, 0, 0, textSize.cx, textSize.cy, 
		SWP_NOMOVE | SWP_NOZORDER);
}

WTL::CString CAngelicWnd::parseSeconds(int seconds)
{
	int sec = seconds % 60;
	int min = (seconds % 3600) / 60;
	int hr  = seconds / 3600;

	WTL::CString result;

	if (hr > 0)
	{
		result.Format(_T("%02d:%02d:%02d"), hr, min, sec);
	}
	else
	{
		result.Format(_T("%02d:%02d"), min, sec);
	}

	return result;
}

void CAngelicWnd::DoEvents()
{
	MSG msg;
	while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE) )
	{
		if (msg.message == WM_QUIT)
			return;

		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
}
