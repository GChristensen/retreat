#include "stdafx.h"

#include "boost/foreach.hpp"

#define foreach BOOST_FOREACH

#include "RestrictionOptionsDlg.h"

CRestrictionOptionsDlg::CRestrictionOptionsDlg() :
COptionDialog(IDD_RESTRICTIONS),
m_restrictionList(this, 1)
{
}

void CRestrictionOptionsDlg::read(SettingTable *settings)
{
	m_restrictions.read(settings);

	foreach (tstring &proc, m_restrictions.processList)
	{
		m_restrictionList.AddString(proc.c_str());
	}

	// no more than ten minutes activity tracking is allowed set trough UI
	m_spinBtn.SetRange(0, 10);
	m_spinBtn.SetPos(m_restrictions.CheckForUserActivityBeforeMin);

	DoDataExchange(FALSE);
}

void CRestrictionOptionsDlg::write(SettingTable *settings)
{
	DoDataExchange(TRUE);

	m_restrictions.processList.clear();

	int count = m_restrictionList.GetCount();

	m_restrictions.processList.reserve(count);

	for (int i = 0; i < count; ++i)
	{
		WTL::CString text;
		m_restrictionList.GetText(i, text);
		m_restrictions.processList.push_back((LPCTSTR)text);
	}

	m_restrictions.write(settings);
}

LRESULT CRestrictionOptionsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	m_wndAddBtn.Attach(GetDlgItem(IDC_ADD_PROCESS));
	m_wndDeleteBtn.Attach(GetDlgItem(IDC_DELETE_PROCESS));

	m_restrictionList.SubclassWindow(GetDlgItem(IDC_RESTRICT_PROCESSES));

	m_spinBtn.Attach(GetDlgItem(IDC_ACTIVITY_CHECK_SPIN));

	return 0;
}

LRESULT CRestrictionOptionsDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	return 0;
}

LRESULT CRestrictionOptionsDlg::OnRestrictProcessSelect(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	return 0;
}

LRESULT CRestrictionOptionsDlg::OnAddProcess(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CFileDialog fileDlg(TRUE);

	WTL::CString filter(MAKEINTRESOURCE(IDS_RESTRICTION_LIST_FILTER));

	int length = filter.GetLength();
	TCHAR *szBuffer = filter.GetBuffer(length);

	for (int i = 0; i < length; ++i)
	{
		if (szBuffer[i] == _T('|'))
			szBuffer[i] = _T('\0');
	}

	fileDlg.m_ofn.hwndOwner = GetParent();
	fileDlg.m_ofn.lpstrFilter = szBuffer;
	fileDlg.m_ofn.nFilterIndex = 1;

	if (fileDlg.DoModal() == IDOK)
	{
		m_restrictionList.AddString(fileDlg.m_szFileName);
		m_restrictionList.SetCurSel(m_restrictionList.GetCount() - 1);
	}

	filter.ReleaseBuffer();

	return 0;
}

LRESULT CRestrictionOptionsDlg::OnDeleteProcess(WPARAM wParam, LPARAM lParam, HWND wnd)
{
	int sel = m_restrictionList.GetCurSel();

	if (sel >= 0)
	{
		WTL::CString message(MAKEINTRESOURCE(IDS_CONFIRM_DELETION_MESSAGE));
		WTL::CString caption(MAKEINTRESOURCE(IDS_APP_NAME));

		if (::MessageBox(
			GetParent(), message, caption, MB_ICONQUESTION | MB_YESNO)
			== IDYES)
		{
			m_restrictionList.DeleteString(sel);
		}
	}

	return 0;
}