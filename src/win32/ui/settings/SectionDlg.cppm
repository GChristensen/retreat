module;

#include "wtl.h"

#include <memory>
#include <string>
#include "tstring.h"

export module SectionDlg;


import Settings;


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