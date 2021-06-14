module;

#include "stdafx.h"

export module TimerWindow;

import <memory>;

import TranslucentWindow;
import TextBuffer;
import Shapes;

import algorithms;

#include "debug.h"

export class CTimerWindow: public CTranslucentWindow
{
	static const int TIMER_TEXT_ITEM = 0;
	static const COLORREF TRANSPARENT_BACKGORUND_COLOR = RGB(254, 0, 254);
public:

	enum
	{
		WM_TIMER_WND_NOTIFY = WM_USER + 2
	};

	CTimerWindow(HWND parent, CRect* pRect, CRect* pWorkArea);

	virtual ~CTimerWindow();

	void SetTimerProperties
	(
		const TCHAR* faceName,
		int fontSize,
		BYTE charSet,
		COLORREF textColor,
		bool antialiased,
		bool bold,
		bool italic
	);

	void SetTimer(int durationSec, bool showStar = false);
	void SetShowTimer(bool show);
	void SetTimerPos(int x, int y);

	bool LoadBackground(const TCHAR* name, bool stretch, COLORREF canvas);
	void DefaultBackground();

	void GrabUserInput();
	void ReturnUserInput();

	void DoEvents();

	//////////////////////////////////////////////////////////////////////
	// Messages
	//////////////////////////////////////////////////////////////////////

	LRESULT OnTimerNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);

	BEGIN_MSG_MAP(CTimerWindow)
		MESSAGE_HANDLER(WM_TIMER_WND_NOTIFY, OnTimerNotify)
		CHAIN_MSG_MAP(CTranslucentWindow)
	END_MSG_MAP()

protected:

	bool isBackgroundLoaded;
	bool showTimer;

	typedef std::unique_ptr<CTextBuffer> text_buff_ptr_t;
	text_buff_ptr_t pTexBuffer;
	COLORREF textColor;
	CPoint timerPos;

	int displayDuration;

	HWND hParentWnd;
	HWND hForegroundWnd;

	virtual void postPanitHook(HDC dc);

	void setTimer(int seconds, bool showStar = false);

	void adjustWindowSize(text_buff_ptr_t buffer, unsigned item);

	CString parseSeconds(int seconds);
};

module :private;

CTimerWindow::CTimerWindow(HWND parent, CRect* pRect, CRect* pWorkArea) :
	CTranslucentWindow(parent, pRect, pWorkArea),
	pTexBuffer(std::make_unique<CTextBuffer>(CClientDC(m_hWnd), 1)),
	isBackgroundLoaded(false),
	hParentWnd(parent),
	showTimer(true),
	timerPos(0, 0),
	textColor(0)
{
	
}

CTimerWindow::~CTimerWindow()
{
}

void CTimerWindow::SetTimerProperties
(
	const TCHAR* faceName,
	int fontSize,
	BYTE charSet,
	COLORREF textColor,
	bool antialiased,
	bool bold,
	bool italic
)
{
	pTexBuffer->SetTextLayerProperties(
		faceName,
		charSet,
		fontSize,
		antialiased,
		bold,
		italic
	);

	this->textColor = textColor;
}

void CTimerWindow::SetTimer(int durationSec, bool showStar)
{
	setTimer(durationSec, showStar);
}

void CTimerWindow::SetShowTimer(bool show)
{
	showTimer = show;
}

void CTimerWindow::SetTimerPos(int x, int y)
{
	timerPos.SetPoint(x, y);

	CClientDC dc(m_hWnd);
	pTexBuffer->MoveTextItem(dc, TIMER_TEXT_ITEM, x, y, false);
}

bool CTimerWindow::LoadBackground(const TCHAR* name, bool stretch, COLORREF canvas)
{
	isBackgroundLoaded = CTranslucentWindow::LoadBackground(name, stretch, canvas);
	return isBackgroundLoaded;
}

void CTimerWindow::DefaultBackground()
{
	bitmapBuffer->NewBitmap(displayRect.Width(), displayRect.Width(), TRANSPARENT_BACKGORUND_COLOR);
	bitmapBuffer->SelectBitmapToInternalDC();

	randomStars(bitmapBuffer->GetDC(), displayRect.Width(), displayRect.Height());

	bitmapBuffer->DeselectBitmapFromInternalDC();

	CRect windowRect;
	GetWindowRect(&windowRect);

	int heightDelta = (windowRect.Height() - displayRect.Height()) / 2;
	int widthDelta = (windowRect.Width() - displayRect.Width()) / 2;

	windowRect.top = heightDelta;
	windowRect.left = heightDelta;
	windowRect.bottom = displayRect.Height() + heightDelta;
	windowRect.right = displayRect.Width() + widthDelta;

	bitmapBuffer->SetPlotRect(windowRect);

	SetTransparentColor(TRANSPARENT_BACKGORUND_COLOR);

	isBackgroundLoaded = true;
}

void CTimerWindow::GrabUserInput()
{
	hForegroundWnd = ::GetForegroundWindow();

	int foregroundWndThreadID = ::GetWindowThreadProcessId(hForegroundWnd, NULL);

	int currentThreadID = ::GetCurrentThreadId();

	BOOL success = ::AttachThreadInput(currentThreadID, foregroundWndThreadID, TRUE);

	if (success)
	{
		::SetForegroundWindow(m_hWnd);
		::AttachThreadInput(currentThreadID, foregroundWndThreadID, FALSE);
	}
}

void CTimerWindow::ReturnUserInput()
{
	int foregroundWndThreadID = ::GetWindowThreadProcessId(hForegroundWnd, NULL);
	int currentThreadID = ::GetCurrentThreadId();

	::AttachThreadInput(currentThreadID, foregroundWndThreadID, TRUE);

	::SetForegroundWindow(hForegroundWnd);

	::AttachThreadInput(currentThreadID, foregroundWndThreadID, FALSE);
}

LRESULT CTimerWindow::OnTimerNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	displayDuration -= 1;

	if (displayDuration < 0)
	{
		return 0;
	}

	CClientDC dc(m_hWnd);

	pTexBuffer->SetItemText(dc, TIMER_TEXT_ITEM, parseSeconds(displayDuration), textColor, true);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void CTimerWindow::postPanitHook(HDC dc)
{
	if (showTimer)
	{
		pTexBuffer->RepaintTextItem(dc, TIMER_TEXT_ITEM);
	}
}

void CTimerWindow::setTimer(int durationSec, bool showStar)
{
	displayDuration = durationSec;

	CString timerText = parseSeconds(durationSec);

	CSize textSize;
	pTexBuffer->GetTextExtent(timerText, textSize);

	// resize window if it not fits the timer
	// do not resize when in fullscreen mode
	// resize only when there is no background and window does not fit the timer
	if (!isFullScreen && !isBackgroundLoaded
		&& (bitmapBuffer->GetWitdth() < textSize.cx
			|| bitmapBuffer->GetHeight() < textSize.cy))
	{
		long offset = 0;

		if (showStar) {
			offset = textSize.cy + 4;
			textSize.cx += offset;
			SetTimerPos(offset, 0);
			displayOffsetX = -offset;
		}

		SetWindowPos(NULL, 0, 0, textSize.cx, textSize.cy, SWP_NOMOVE | SWP_NOZORDER);

		bitmapBuffer->FillBackground(textSize.cx, textSize.cy, TRANSPARENT_BACKGORUND_COLOR);

		if (showStar) {
			bitmapBuffer->SelectBitmapToInternalDC();

			long radius = textSize.cy / 2;
			POINT2 point = { .x = radius, .y = radius };
			drawStar(
				bitmapBuffer->GetDC(),
				point,
				radius,
				80,
				getRandomShapeColor()
			);

			bitmapBuffer->DeselectBitmapFromInternalDC();
		}

		SetTransparentColor(TRANSPARENT_BACKGORUND_COLOR);
	}

	CClientDC dc(m_hWnd);

	pTexBuffer->SetTextItem(
		dc,
		TIMER_TEXT_ITEM,
		timerPos.x,
		timerPos.y,
		timerText,
		textColor
	);

	pTexBuffer->GetBackground(bitmapBuffer->SelectBitmapToInternalDC(), TIMER_TEXT_ITEM);
	bitmapBuffer->DeselectBitmapFromInternalDC();

	pTexBuffer->RepaintTextItem(dc, TIMER_TEXT_ITEM, false);

	// window top-left point coordinates are negative in fullscreen mode
	if (isFullScreen)
	{
		CRect rect;
		GetWindowRect(rect);

		if (rect.top < 0 || rect.left < 0)
		{
			pTexBuffer->SetWindowOrigin(CPoint(-rect.top, -rect.left));
		}
	}
}

void CTimerWindow::adjustWindowSize(text_buff_ptr_t buffer, unsigned item)
{
	CSize textSize;

	buffer->GetTextExtent(item, textSize);

	SetWindowPos(NULL, 0, 0, textSize.cx, textSize.cy, SWP_NOMOVE | SWP_NOZORDER);
}

CString CTimerWindow::parseSeconds(int seconds)
{
	int sec = seconds % 60;
	int min = (seconds % 3600) / 60;
	int hr = seconds / 3600;

	CString result;

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

void CTimerWindow::DoEvents()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
