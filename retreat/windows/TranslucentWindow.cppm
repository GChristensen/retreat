module;

#include "stdafx.h"

export module TranslucentWindow;

import BitmapBuffer;

using WindowTraits = CWinTraits<0, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW>;

export class CTranslucentWindow: public CWindowImpl<CTranslucentWindow, CWindow, WindowTraits>
{
public:

	CTranslucentWindow(HWND parent, CRect *pRect, CRect *pWorkArea);
	~CTranslucentWindow();

	// option determines if there should be totally transparent regions 
	// in the window
	void SetTransparentColor(COLORREF crTransparentColor);
	void SetTransparentColorFromImage();

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

	DECLARE_WND_CLASS_EX(_T("EnsoRetreatTimerWindow"), 0, NULL)

	BEGIN_MSG_MAP(CTranslucentWindow)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	END_MSG_MAP()

protected:

	BYTE windowAlpha;
	bool isUsingTransparency;

	bool isFullScreen;

	CRect displayRect;
	CRect workAreaRect;
	CBitmapBuffer *bitmapBuffer;

	COLORREF transparentColor;

	virtual CBitmapBuffer *getBitmapBuffer(CDC *pDC);

	virtual void postPanitHook(HDC dc);

	// make image dimensions to fit viewport dimensions
	// image bounds ratio will be preserved
	static void AdjustDimensions(const CSize& bounds, const CSize& oldSize, CSize& newSize);

	// image dimensions should not exceed specified size
	static void AdjustImageToVPort(CBitmapBuffer *pBackBuffer, const CSize& vpSize, bool stretch = false);

	void AdjustToVPort(CBitmapBuffer *pBackBuffer);
	void AdjustToScreen(CBitmapBuffer *pBackBuffer, bool stretch, COLORREF canvas);

	// random number in range from min_ to max_ - 1
	static int randRange(int min_, int max_);

};

module :private;

CTranslucentWindow::CTranslucentWindow(HWND parent, CRect *pRect, CRect *pWorkArea) :
	isUsingTransparency(false),
	isFullScreen(pWorkArea == nullptr),
	windowAlpha(255)
{
	Create(parent, pRect, NULL, isFullScreen ? WS_MAXIMIZE : 0);
	ModifyStyle(WS_CAPTION, 0, SWP_FRAMECHANGED);

	if (pWorkArea)
	{
		workAreaRect.CopyRect(*pWorkArea);
	}

	if (pRect)
		displayRect.CopyRect(pRect);

	CClientDC clientDC(m_hWnd);
	bitmapBuffer = getBitmapBuffer(&clientDC);
}

CTranslucentWindow::~CTranslucentWindow()
{
	delete bitmapBuffer;
}

void CTranslucentWindow::SetTransparentColor(COLORREF crTransparentColor)
{
	isUsingTransparency = true;
	transparentColor = crTransparentColor;
}

void CTranslucentWindow::SetTransparentColorFromImage()
{
	if (bitmapBuffer->m_hBitmap != NULL)
	{
		isUsingTransparency = true;

		transparentColor = ::GetPixel(bitmapBuffer->SelectBitmapToInternalDC(), 0, 0);

		bitmapBuffer->DeselectBitmapFromInternalDC();
	}
}

void CTranslucentWindow::SetAlpha(BYTE alpha)
{
	windowAlpha = alpha;
}

void CTranslucentWindow::ApplyLayeredWindowAttributes()
{
	DWORD flags = LWA_ALPHA | (isUsingTransparency ? LWA_COLORKEY : 0);
	SetLayeredWindowAttributes(m_hWnd, transparentColor, windowAlpha, flags);
}

BYTE CTranslucentWindow::GetAlpha()
{
	return windowAlpha;
}

void CTranslucentWindow::AlterAlpha(int delta)
{
	if (windowAlpha + delta < 0)
	{
		windowAlpha = 0;
	}
	else if (windowAlpha + delta > 255)
	{
		windowAlpha = 255;
	}
	else
	{
		windowAlpha += delta;
	}

	DWORD flags = LWA_ALPHA | (isUsingTransparency ? LWA_COLORKEY : 0);
	SetLayeredWindowAttributes(m_hWnd, transparentColor, windowAlpha, flags);
}

bool CTranslucentWindow::LoadBackground(const TCHAR *name, bool stretch, COLORREF canvas)
{
	if (name && *name && SUCCEEDED(bitmapBuffer->LoadFromFile(name)))
	{
		if (isFullScreen)
		{
			AdjustToScreen(bitmapBuffer, stretch, canvas);
		}
		else
		{
			AdjustToVPort(bitmapBuffer);
		}

		return true;
	}

	return false;
}

void CTranslucentWindow::PlaceWindowOnWorkArea(int x, int y)
{
	if (isFullScreen)
	{
		return;
	}

	SetWindowPos(
		NULL,
		workAreaRect.left + x,
		workAreaRect.top + y,
		0, 0, SWP_NOZORDER | SWP_NOSIZE
	);
}

void CTranslucentWindow::CenterWindowOnWorkArea()
{
	if (isFullScreen)
	{
		return;
	}

	CRect clientRect;
	GetClientRect(&clientRect);

	SetWindowPos(
		NULL,
		workAreaRect.left + (workAreaRect.Width() - clientRect.Width()) / 2,
		workAreaRect.top + (workAreaRect.Height() - clientRect.Height()) / 2,
		0, 0, SWP_NOZORDER | SWP_NOSIZE
	);
}

void CTranslucentWindow::RandomlyPlaceWindowOnWorkArea()
{
	if (isFullScreen)
	{
		return;
	}

	CRect clientRect;
	GetClientRect(&clientRect);

	int x = randRange(
		workAreaRect.left,
		workAreaRect.right - clientRect.Width()
	);

	int y = randRange(
		workAreaRect.top,
		workAreaRect.bottom - clientRect.Height()
	);

	SetWindowPos(NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

LRESULT CTranslucentWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	DestroyWindow();
	return 0;
}

LRESULT CTranslucentWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return 0;
}

LRESULT CTranslucentWindow::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return 0;
}

LRESULT CTranslucentWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	CPaintDC dc(m_hWnd);

	bitmapBuffer->Draw(dc);

	postPanitHook(dc);

	return 0;
}

LRESULT CTranslucentWindow::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return 0;
}

CBitmapBuffer *CTranslucentWindow::getBitmapBuffer(CDC *pDC)
{
	return new CBitmapBuffer(pDC->m_hDC);
}

void CTranslucentWindow::postPanitHook(HDC dc)
{

}

void CTranslucentWindow::AdjustDimensions
(
	const CSize& bounds,
	const CSize& oldSize,
	CSize& newSize
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

void CTranslucentWindow::AdjustImageToVPort
(
	CBitmapBuffer *pBackBuffer,
	const CSize& vpSize,
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
	if (stretch || imageWidth > vpWidth || imageHeight > vpHeight)
	{
		AdjustDimensions(
			CSize(vpWidth, vpHeight),
			CSize(imageWidth, imageHeight),
			imageNewSize);
	}

	pBackBuffer->Resize(imageNewSize);
}

void CTranslucentWindow::AdjustToVPort(CBitmapBuffer *pBackBuffer)
{
	CSize targetSize;
	targetSize.SetSize(workAreaRect.Width(), workAreaRect.Height());
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

void CTranslucentWindow::AdjustToScreen
(
	CBitmapBuffer *pBitmapBuffer,
	bool stretch,
	COLORREF backgroundColor
)
{
	CRect windowRect;
	GetWindowRect(windowRect);

	// fullscreen window rect top left coordinates are negative
	CSize targetSize(displayRect.Width(), displayRect.Height());
	AdjustImageToVPort(pBitmapBuffer, targetSize, stretch);

	// create backgroundColor if image does not fills entrie screen in fullscreen mode
	if (pBitmapBuffer->GetSize() != targetSize)
	{
		pBitmapBuffer->FillBackground(targetSize.cx, targetSize.cy, backgroundColor);
	}

	int heightDelta = (windowRect.Height() - displayRect.Height()) / 2;
	int widthDelta = (windowRect.Width() - displayRect.Width()) / 2;

	windowRect.top = heightDelta;
	windowRect.left = heightDelta;
	windowRect.bottom = displayRect.Height() + heightDelta;
	windowRect.right = displayRect.Width() + widthDelta;

	pBitmapBuffer->SetPlotRect(windowRect);
}

int CTranslucentWindow::randRange(int min_, int max_)
{
	return min_ + rand() % (max_ - min_);
}