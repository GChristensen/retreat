#pragma once

#include <atlctrls.h>
#include <atlctrlx.h>

class CBitmapButtonEx: public CBitmapButtonImpl<CBitmapButtonEx>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_CBitmapButtonEx"), GetWndClassName())

	CBitmapButtonEx(DWORD dwExtendedStyle = BMPBTN_AUTOSIZE, HIMAGELIST hImageList = NULL): 
		CBitmapButtonImpl<CBitmapButtonEx>(dwExtendedStyle, hImageList)
	{}

	BEGIN_MSG_MAP(CBitmapButtonEx)
		CHAIN_MSG_MAP(CBitmapButtonImpl<CBitmapButtonEx>)
	END_MSG_MAP()

	void DoPaint(CDCHandle dc)
	{
		RECT rc;
		GetClientRect(&rc);
		dc.FillRect(&rc, (HBRUSH)(COLOR_BTNFACE + 1));

		CBitmapButtonImpl<CBitmapButtonEx>::DoPaint(dc); 
	}
};
