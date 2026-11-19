module;

#include "wtl.h"
#include "resource.h"

#include <algorithm>
#include <bitset>
#include <format>
#include <optional>
#include <string>
#include <vector>
#include "tstring.h"

export module ScheduleSettingsSection;


import Settings;
import SettingsModel;
import SectionDlg;
import SettingsControls;

static const int DAY_IDS[7] = { IDC_DAY_MON, IDC_DAY_TUE, IDC_DAY_WED, IDC_DAY_THU, IDC_DAY_FRI, IDC_DAY_SAT, IDC_DAY_SUN };

static const struct { int id; int value; } DURATION_PRESETS[] = {
	{ IDC_DUR_P10, 10 }, { IDC_DUR_P15, 15 }, { IDC_DUR_P30, 30 }, { IDC_DUR_P60, 60 } };

// Editor controls that need a schedule the visual editor understands.
static const int TIME_CONTROLS[] = {
	IDC_DAY_MON, IDC_DAY_TUE, IDC_DAY_WED, IDC_DAY_THU, IDC_DAY_FRI, IDC_DAY_SAT, IDC_DAY_SUN,
	IDC_EVERY_DAY, IDC_WEEKDAYS, IDC_CRON_HOUR, IDC_CRON_HOUR_SPIN, IDC_CRON_MINUTE, IDC_CRON_MINUTE_SPIN };

static const int OTHER_EDITOR_CONTROLS[] = {
	IDC_SCHEDULED_BREAK_DURATION, IDC_DURATION_SPIN, IDC_DUR_P10, IDC_DUR_P15, IDC_DUR_P30, IDC_DUR_P60,
	IDC_SCHEDULED_BREAK_MESSAGE, IDC_DELETE_CRON, IDC_EDITOR_GROUP };

enum Column { COL_WHEN, COL_DURATION, COL_MESSAGE, COL_CRON };

struct ScheduledBreak {
	tstring cron;
	std::optional<SimpleSchedule> simple;  // empty when the cron expression is too complex to edit visually
	int durationMin = 10;
	tstring message;

	tstring when() const {
		return simple ? describeSchedule(*simple) : _T("Custom: ") + cron;
	}
};

export class CScheduleSettingsSection : public CSectionDlg
{
public:
	CScheduleSettingsSection();

	virtual void read(Settings &settings) override;
	virtual void write(Settings &settings) override;

	BEGIN_MSG_MAP(CScheduleSettingsSection)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_ADD_CRON, OnAdd)
		COMMAND_ID_HANDLER(IDC_SCHEDULE_ENABLE, OnEnable)
		COMMAND_ID_HANDLER(IDC_DELETE_CRON, OnDelete)
		COMMAND_ID_HANDLER(IDC_EVERY_DAY, OnDayPreset)
		COMMAND_ID_HANDLER(IDC_WEEKDAYS, OnDayPreset)
		COMMAND_RANGE_HANDLER(IDC_DAY_MON, IDC_DAY_SUN, OnEditorChanged)
		COMMAND_RANGE_HANDLER(IDC_DUR_P10, IDC_DUR_P60, OnDurationPreset)
		COMMAND_HANDLER(IDC_WEEK_STRIP, WSN_SELECT, OnStripSelect)
		COMMAND_CODE_HANDLER(EN_CHANGE, OnEditorChanged)
		NOTIFY_HANDLER(IDC_CRON_LIST, LVN_ITEMCHANGED, OnListItemChanged)
		NOTIFY_HANDLER(IDC_CRON_LIST, LVN_KEYDOWN, OnListKeyDown)
		CHAIN_MSG_MAP(CSectionDlg)
	END_MSG_MAP()

private:
	std::vector<ScheduledBreak> m_breaks;
	int m_selected = -1;

	CListViewCtrl m_list;
	CWeekStrip m_weekStrip;

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnAdd(WORD, WORD, HWND, BOOL&);
	LRESULT OnEnable(WORD, WORD, HWND, BOOL&);
	LRESULT OnDelete(WORD, WORD, HWND, BOOL&);
	LRESULT OnDayPreset(WORD, WORD, HWND, BOOL&);
	LRESULT OnDurationPreset(WORD, WORD, HWND, BOOL&);
	LRESULT OnEditorChanged(WORD, WORD, HWND, BOOL&);
	LRESULT OnStripSelect(WORD, WORD, HWND, BOOL&);
	LRESULT OnListItemChanged(int, LPNMHDR, BOOL&);
	LRESULT OnListKeyDown(int, LPNMHDR, BOOL&);

	void fillList();
	void updateRow(int index);
	void updateStrip();
	void select(int index);
	void loadEditor();
	void storeEditor();
};

module :private;


CScheduleSettingsSection::CScheduleSettingsSection() :
	CSectionDlg(IDD_SCHEDULE)
{
	SetOptionsCategoryName(CString(MAKEINTRESOURCE(IDS_SCHEDULE_OPTIONS_SECTION_NAME)));
}

LRESULT CScheduleSettingsSection::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	InitSectionHeader();
	MarkMuted(IDC_CUSTOM_NOTE);

	m_weekStrip.SubclassWindow(GetDlgItem(IDC_WEEK_STRIP));

	m_list.Attach(GetDlgItem(IDC_CRON_LIST));
	m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	CRect rc;
	m_list.GetClientRect(&rc);
	int width = rc.Width() - ::GetSystemMetrics(SM_CXVSCROLL);

	m_list.InsertColumn(COL_WHEN, CString(MAKEINTRESOURCE(IDS_SCHEDULED_BREAK_WHEN_COLUMN)), LVCFMT_LEFT, width * 34 / 100);
	m_list.InsertColumn(COL_DURATION, CString(MAKEINTRESOURCE(IDS_SCHEDULED_BREAK_DURATION_COLUMN)), LVCFMT_RIGHT, width * 18 / 100);
	m_list.InsertColumn(COL_MESSAGE, CString(MAKEINTRESOURCE(IDS_SCHEDULED_BREAK_MESSAGE_COLUMN)), LVCFMT_LEFT, width * 30 / 100);
	m_list.InsertColumn(COL_CRON, CString(MAKEINTRESOURCE(IDS_SCHEDULED_BREAK_CRONTAB_COLUMN)), LVCFMT_LEFT, width * 18 / 100);

	InitSpin(IDC_CRON_HOUR_SPIN, 12, 0, 23);
	InitSpin(IDC_CRON_MINUTE_SPIN, 0, 0, 59, 5);
	InitSpin(IDC_DURATION_SPIN, Settings::DEFAULT_BREAK_DURATION, 1, 240);

	CEdit(GetDlgItem(IDC_SCHEDULED_BREAK_MESSAGE)).SetCueBannerText(L"Shown on the break screen");

	return TRUE;
}

void CScheduleSettingsSection::read(Settings &settings)
{
	m_breaks.clear();

	SetChecked(IDC_SCHEDULE_ENABLE, settings.getBoolean(Settings::SCHEDULE_ENABLE, Settings::DEFAULT_SCHEDULE_ENABLE));

	for (auto &schedule : settings.getSectionValues(Settings::CRON)) {
		auto parts = settings.split(schedule);

		ScheduledBreak entry;
		entry.cron = parts.size() > 0 ? parts[0] : tstring();
		entry.simple = parseSimpleSchedule(entry.cron);
		try {
			entry.durationMin = parts.size() > 1 ? std::stoi(parts[1]) : Settings::DEFAULT_BREAK_DURATION;
		}
		catch (std::exception&) {
			entry.durationMin = Settings::DEFAULT_BREAK_DURATION;
		}
		entry.message = parts.size() > 2 ? parts[2] : tstring();

		m_breaks.push_back(entry);
	}

	fillList();
	select(m_breaks.empty() ? -1 : 0);
}

void CScheduleSettingsSection::write(Settings &settings)
{
	settings.setBoolean(Settings::SCHEDULE_ENABLE, IsChecked(IDC_SCHEDULE_ENABLE));
	settings.clearSection(Settings::CRON);

	for (size_t i = 0; i < m_breaks.size(); ++i) {
		const ScheduledBreak& entry = m_breaks[i];
		tstring value = settings.join({ entry.cron, to_tstring(entry.durationMin), entry.message });
		settings.setString(Settings::CRON + std::format(_T("{:02}"), i), value);
	}
}

void CScheduleSettingsSection::fillList()
{
	m_list.SetRedraw(FALSE);
	m_list.DeleteAllItems();
	for (size_t i = 0; i < m_breaks.size(); ++i) {
		m_list.InsertItem((int)i, _T(""));
		updateRow((int)i);
	}
	m_list.SetRedraw(TRUE);
	m_list.Invalidate();
	updateStrip();
}

void CScheduleSettingsSection::updateRow(int index)
{
	const ScheduledBreak& entry = m_breaks[index];
	m_list.SetItemText(index, COL_WHEN, entry.when().c_str());
	m_list.SetItemText(index, COL_DURATION, std::format(_T("{} min"), entry.durationMin).c_str());
	m_list.SetItemText(index, COL_MESSAGE, entry.message.c_str());
	m_list.SetItemText(index, COL_CRON, entry.cron.c_str());
}

void CScheduleSettingsSection::updateStrip()
{
	std::vector<WeekItem> items;
	for (const ScheduledBreak& entry : m_breaks) {
		WeekItem item;
		if (entry.simple) {
			item.days = entry.simple->days;
			item.hour = entry.simple->hour;
			item.minute = entry.simple->minute;
		}
		item.durationMin = entry.durationMin;
		items.push_back(item);
	}
	m_weekStrip.SetItems(items, m_selected);
}

void CScheduleSettingsSection::select(int index)
{
	m_selected = index;

	if (index >= 0 && m_list.GetSelectedIndex() != index) {
		m_loading = true;
		m_list.SelectItem(index);
		m_loading = false;
	}

	loadEditor();
	updateStrip();
}

void CScheduleSettingsSection::loadEditor()
{
	m_loading = true;

	bool on = IsChecked(IDC_SCHEDULE_ENABLE);
	bool has = on && m_selected >= 0 && m_selected < (int)m_breaks.size();
	bool simple = has && m_breaks[m_selected].simple.has_value();

	for (int id : TIME_CONTROLS)
		Enable(id, simple);
	for (int id : OTHER_EDITOR_CONTROLS)
		Enable(id, has);
	for (int id : { IDC_CRON_LIST, IDC_WEEK_STRIP, IDC_ADD_CRON })
		Enable(id, on);

	if (has) {
		const ScheduledBreak& entry = m_breaks[m_selected];
		SimpleSchedule s = entry.simple.value_or(SimpleSchedule{});

		for (int d = 0; d < 7; ++d)
			SetChecked(DAY_IDS[d], simple && s.days.test(d));

		CUpDownCtrl(GetDlgItem(IDC_CRON_HOUR_SPIN)).SetPos32(s.hour);
		CUpDownCtrl(GetDlgItem(IDC_CRON_MINUTE_SPIN)).SetPos32(s.minute);
		SetDlgItemText(IDC_CRON_MINUTE, std::format(_T("{:02}"), s.minute).c_str());
		CUpDownCtrl(GetDlgItem(IDC_DURATION_SPIN)).SetPos32(entry.durationMin);
		SetDlgItemText(IDC_SCHEDULED_BREAK_MESSAGE, entry.message.c_str());

		for (const auto& preset : DURATION_PRESETS)
			SetChecked(preset.id, preset.value == entry.durationMin);

		SetDlgItemText(IDC_CUSTOM_NOTE, simple ? _T("")
			: _T("Custom schedule: only the length and message can be changed here."));
	}
	else {
		for (int id : DAY_IDS)
			SetChecked(id, false);
		for (const auto& preset : DURATION_PRESETS)
			SetChecked(preset.id, false);
		SetDlgItemText(IDC_CRON_HOUR, _T(""));
		SetDlgItemText(IDC_CRON_MINUTE, _T(""));
		SetDlgItemText(IDC_SCHEDULED_BREAK_DURATION, _T(""));
		SetDlgItemText(IDC_SCHEDULED_BREAK_MESSAGE, _T(""));
		SetDlgItemText(IDC_CUSTOM_NOTE, !on ? _T("Scheduled breaks are off.")
			: m_breaks.empty() ? _T("No scheduled breaks yet.") : _T(""));
	}

	m_loading = false;
}

void CScheduleSettingsSection::storeEditor()
{
	if (m_selected < 0 || m_selected >= (int)m_breaks.size())
		return;

	ScheduledBreak& entry = m_breaks[m_selected];

	if (entry.simple) {
		SimpleSchedule s;
		for (int d = 0; d < 7; ++d)
			s.days.set(d, IsChecked(DAY_IDS[d]));
		s.hour = std::clamp(GetInt(IDC_CRON_HOUR, entry.simple->hour), 0, 23);
		s.minute = std::clamp(GetInt(IDC_CRON_MINUTE, entry.simple->minute), 0, 59);
		if (s.days.none())
			s.days = entry.simple->days.none() ? std::bitset<7>().set() : entry.simple->days;
		entry.simple = s;
		entry.cron = toCron(s);
	}

	entry.durationMin = std::clamp(GetInt(IDC_SCHEDULED_BREAK_DURATION, entry.durationMin), 1, 240);

	CString message;
	GetDlgItemText(IDC_SCHEDULED_BREAK_MESSAGE, message);
	message.Trim();
	message.Remove(_T('|'));  // the field separator of the stored value
	entry.message = (LPCTSTR)message;

	for (const auto& preset : DURATION_PRESETS)
		SetChecked(preset.id, preset.value == entry.durationMin);

	updateRow(m_selected);
	updateStrip();
	NotifyChanged();
}

LRESULT CScheduleSettingsSection::OnAdd(WORD, WORD, HWND, BOOL&)
{
	ScheduledBreak entry;
	SimpleSchedule s;
	s.hour = 15;
	s.minute = 0;
	s.days = std::bitset<7>(0b0011111);  // weekdays
	entry.simple = s;
	entry.cron = toCron(s);
	entry.durationMin = Settings::DEFAULT_BREAK_DURATION;

	m_breaks.push_back(entry);
	m_loading = true;
	m_list.InsertItem((int)m_breaks.size() - 1, _T(""));
	m_loading = false;
	updateRow((int)m_breaks.size() - 1);
	select((int)m_breaks.size() - 1);
	m_list.EnsureVisible(m_selected, FALSE);

	GotoDlgCtrl(GetDlgItem(IDC_CRON_HOUR));
	NotifyChanged();
	return 0;
}

LRESULT CScheduleSettingsSection::OnEnable(WORD, WORD, HWND, BOOL&)
{
	loadEditor();
	m_weekStrip.Invalidate();
	NotifyChanged();
	return 0;
}

LRESULT CScheduleSettingsSection::OnDelete(WORD, WORD, HWND, BOOL&)
{
	if (m_selected < 0 || m_selected >= (int)m_breaks.size())
		return 0;

	CString message(MAKEINTRESOURCE(IDS_CONFIRM_DELETION_MESSAGE));
	CString caption(MAKEINTRESOURCE(IDS_APP_NAME));

	if (::MessageBox(GetParent(), message, caption, MB_ICONQUESTION | MB_YESNO) != IDYES)
		return 0;

	int index = m_selected;
	m_breaks.erase(m_breaks.begin() + index);

	m_loading = true;
	fillList();
	m_loading = false;

	select(m_breaks.empty() ? -1 : (std::min)(index, (int)m_breaks.size() - 1));
	NotifyChanged();
	return 0;
}

LRESULT CScheduleSettingsSection::OnDayPreset(WORD, WORD id, HWND, BOOL&)
{
	if (m_selected < 0 || !m_breaks[m_selected].simple)
		return 0;

	m_loading = true;
	for (int d = 0; d < 7; ++d)
		SetChecked(DAY_IDS[d], id == IDC_EVERY_DAY || d < 5);
	m_loading = false;

	storeEditor();
	return 0;
}

LRESULT CScheduleSettingsSection::OnDurationPreset(WORD, WORD id, HWND, BOOL&)
{
	if (m_selected < 0)
		return 0;

	for (const auto& preset : DURATION_PRESETS)
		if (preset.id == id) {
			m_loading = true;
			CUpDownCtrl(GetDlgItem(IDC_DURATION_SPIN)).SetPos32(preset.value);
			m_loading = false;
		}

	storeEditor();
	return 0;
}

LRESULT CScheduleSettingsSection::OnEditorChanged(WORD, WORD, HWND, BOOL&)
{
	if (!m_loading)
		storeEditor();
	return 0;
}

LRESULT CScheduleSettingsSection::OnStripSelect(WORD, WORD, HWND, BOOL&)
{
	int index = m_weekStrip.GetClickedItem();
	if (index >= 0 && index < (int)m_breaks.size()) {
		select(index);
		m_list.EnsureVisible(index, FALSE);
	}
	return 0;
}

LRESULT CScheduleSettingsSection::OnListItemChanged(int, LPNMHDR hdr, BOOL&)
{
	LPNMLISTVIEW info = (LPNMLISTVIEW)hdr;

	if (m_loading || !(info->uChanged & LVIF_STATE))
		return 0;

	if ((info->uNewState & LVIS_SELECTED) && !(info->uOldState & LVIS_SELECTED))
		select(info->iItem);
	else if (!(info->uNewState & LVIS_SELECTED) && (info->uOldState & LVIS_SELECTED) && m_list.GetSelectedIndex() < 0)
		select(-1);

	return 0;
}

LRESULT CScheduleSettingsSection::OnListKeyDown(int, LPNMHDR hdr, BOOL&)
{
	if (((LPNMLVKEYDOWN)hdr)->wVKey == VK_DELETE)
		PostMessage(WM_COMMAND, IDC_DELETE_CRON);
	return 0;
}
