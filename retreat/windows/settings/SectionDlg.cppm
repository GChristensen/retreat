module;

#include "stdafx.h"

export module SectionDlg;

import <memory>;
import <string>;

import Settings;

#include "tstring.h"

export class CSectionDlg: public CDialogImpl<CSectionDlg>
{
public:

	int IDD;

	CSectionDlg(int idd) :
		IDD(idd),
		m_accepted(false)
	{
	}

	virtual ~CSectionDlg()
	{
	}

	const CString &GetOptionsCategoryName()
	{
		return m_optionsCategoryName;
	}

	void SetOptionsCategoryName(const CString &name)
	{
		m_optionsCategoryName = name;
	}

	void SetAccepted(bool accepted) { m_accepted = accepted; }
	bool Accepted() { return m_accepted; }

	virtual void read(Settings &settings)
	{}

	virtual void write(Settings &settings)
	{}

	DECLARE_EMPTY_MSG_MAP()

protected:

	bool m_accepted;
	CString m_optionsCategoryName;

};

export using SectionPtr = std::shared_ptr<CSectionDlg>;