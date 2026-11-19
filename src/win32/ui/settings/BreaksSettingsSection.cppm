module;

#include "wtl.h"
#include "resource.h"

#include <algorithm>
#include <format>
#include <iterator>
#include <string>
#include "tstring.h"

export module BreaksSettingsSection;

import Settings;
import SettingsModel;
import SectionDlg;
import SettingsControls;

struct Preset { int id; int value; };

static const Preset PERIOD_PRESETS[] = {
	{ IDC_PERIOD_P30, 30 }, { IDC_PERIOD_P45, 45 }, { IDC_PERIOD_P60, 60 }, { IDC_PERIOD_P90, 90 } };
static const Preset BREAK_PRESETS[] = {
	{ IDC_BREAK_P5, 5 }, { IDC_BREAK_P10, 10 }, { IDC_BREAK_P15, 15 } };
static const Preset ALERT_PRESETS[] = {
	{ IDC_ALERT_P1, 1 }, { IDC_ALERT_P2, 2 }, { IDC_ALERT_P5, 5 } };

// Controls that only matter for periodic breaks. The warning and postponing
// settings also apply to scheduled breaks, so they stay enabled.
static const int PERIODIC_CONTROLS[] = {
	IDC_CYCLE_BAR,
	IDC_PERIOD, IDC_PERIOD_SPIN, IDC_PERIOD_P30, IDC_PERIOD_P45, IDC_PERIOD_P60, IDC_PERIOD_P90,
	IDC_BREAK, IDC_BREAK_SPIN, IDC_BREAK_P5, IDC_BREAK_P10, IDC_BREAK_P15,
	IDC_SYNC };

export class CBreaksSettingsSection : public CSectionDlg
{
public:
	CBreaksSettingsSection();

	virtual void read(Settings &settings) override;
	virtual void write(Settings &settings) override;

	BEGIN_MSG_MAP(CBreaksSettingsSection)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_CODE_HANDLER(EN_CHANGE, OnEditChange)
		COMMAND_CODE_HANDLER(EN_KILLFOCUS, OnEditKillFocus)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
		CHAIN_MSG_MAP(CSectionDlg)
	END_MSG_MAP()

private:
	CCycleBar m_cycleBar;

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnEditChange(WORD, WORD, HWND, BOOL&);
	LRESULT OnEditKillFocus(WORD, WORD, HWND, BOOL&);
	LRESULT OnButtonClicked(WORD, WORD, HWND, BOOL&);

	CycleTiming currentTiming();
	void updateView();
	void updatePresets(const Preset* presets, size_t count, int value);
	bool applyPreset(const Preset* presets, size_t count, int id, int editId);
};

module :private;

CBreaksSettingsSection::CBreaksSettingsSection() :
	CSectionDlg(IDD_BREAKS)
{
	SetOptionsCategoryName(CString(MAKEINTRESOURCE(IDS_BREAKS_SECTION_NAME)));
}

LRESULT CBreaksSettingsSection::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	InitSectionHeader();
	MarkMuted(IDC_CYCLE_SUMMARY);
	MarkMuted(IDC_PERIOD_HINT);
	MarkMuted(IDC_BREAK_HINT);
	MarkMuted(IDC_ALERT_HINT);

	m_cycleBar.SubclassWindow(GetDlgItem(IDC_CYCLE_BAR));
	return TRUE;
}

void CBreaksSettingsSection::read(Settings &settings)
{
	m_loading = true;

	CycleTiming t = clampCycle(
		settings.getInt(Settings::PERIOD_DURATION, Settings::DEFAULT_PERIOD_DURATION),
		settings.getInt(Settings::BREAK_DURATION, Settings::DEFAULT_BREAK_DURATION),
		settings.getInt(Settings::ALERT_DURATION, Settings::DEFAULT_ALERT_DURATION));

	InitSpin(IDC_PERIOD_SPIN, t.periodMin, MIN_PERIOD_MIN, MAX_PERIOD_MIN, 5);
	InitSpin(IDC_BREAK_SPIN, t.breakMin, 1, MAX_PERIOD_MIN - MIN_WORK_MIN);
	InitSpin(IDC_ALERT_SPIN, t.alertMin, 1, 60);
	InitSpin(IDC_DELAY_LIMIT_SPIN, settings.getInt(Settings::DELAY_AMOUNT, Settings::DEFAULT_DELAY_AMOUNT), 0, 5);
	InitSpin(IDC_DELAY_SPIN, settings.getInt(Settings::DELAY_DURATION, Settings::DEFAULT_DELAY_DURATION), 1, 60);
	InitSpin(IDC_DISABLE_FOR_SPIN, settings.getInt(Settings::SUSPENDED_DURATION, Settings::DEFAULT_SUSPENDED_DURATION), 1, 24);

	SetChecked(IDC_PERIODIC_ENABLE, settings.getBoolean(Settings::PERIOD_ENABLE, Settings::DEFAULT_PERIOD_ENABLE));
	SetChecked(IDC_SYNC, !settings.getBoolean(Settings::PERIOD_FROM_LAUNCH, Settings::DEFAULT_PERIOD_FROM_LAUNCH));
	SetChecked(IDC_BEEP, settings.getBoolean(Settings::SOUNDS_ENABLE, Settings::DEFAULT_SOUNDS_ENABLE));

	m_loading = false;
	updateView();
}

void CBreaksSettingsSection::write(Settings &settings)
{
	CycleTiming t = currentTiming();

	settings.setInt(Settings::PERIOD_DURATION, t.periodMin);
	settings.setInt(Settings::BREAK_DURATION, t.breakMin);
	settings.setInt(Settings::ALERT_DURATION, t.alertMin);
	settings.setInt(Settings::DELAY_AMOUNT, std::clamp(GetInt(IDC_DELAY_LIMIT), 0, 5));
	settings.setInt(Settings::DELAY_DURATION, std::clamp(GetInt(IDC_DELAY, 1), 1, 60));
	settings.setInt(Settings::SUSPENDED_DURATION, std::clamp(GetInt(IDC_DISABLE_FOR, 1), 1, 24));
	settings.setBoolean(Settings::PERIOD_ENABLE, IsChecked(IDC_PERIODIC_ENABLE));
	settings.setBoolean(Settings::PERIOD_FROM_LAUNCH, !IsChecked(IDC_SYNC));
	settings.setBoolean(Settings::SOUNDS_ENABLE, IsChecked(IDC_BEEP));
}

CycleTiming CBreaksSettingsSection::currentTiming()
{
	return clampCycle(GetInt(IDC_PERIOD, MIN_PERIOD_MIN), GetInt(IDC_BREAK, 1), GetInt(IDC_ALERT, 1));
}

void CBreaksSettingsSection::updatePresets(const Preset* presets, size_t count, int value)
{
	for (size_t i = 0; i < count; ++i)
		SetChecked(presets[i].id, presets[i].value == value);
}

bool CBreaksSettingsSection::applyPreset(const Preset* presets, size_t count, int id, int editId)
{
	for (size_t i = 0; i < count; ++i)
		if (presets[i].id == id) {
			SetInt(editId, presets[i].value);
			return true;
		}
	return false;
}

void CBreaksSettingsSection::updateView()
{
	if (m_loading)
		return;

	CycleTiming t = currentTiming();
	int delayCount = std::clamp(GetInt(IDC_DELAY_LIMIT), 0, 5);
	int delayMin = std::clamp(GetInt(IDC_DELAY, 1), 1, 60);
	bool enabled = IsChecked(IDC_PERIODIC_ENABLE);
	bool align = IsChecked(IDC_SYNC);

	updatePresets(PERIOD_PRESETS, std::size(PERIOD_PRESETS), GetInt(IDC_PERIOD));
	updatePresets(BREAK_PRESETS, std::size(BREAK_PRESETS), GetInt(IDC_BREAK));
	updatePresets(ALERT_PRESETS, std::size(ALERT_PRESETS), GetInt(IDC_ALERT));

	CycleModel model;
	model.periodMin = t.periodMin;
	model.breakMin = t.breakMin;
	model.alertMin = t.alertMin;
	model.delayCount = delayCount;
	model.delayMin = delayMin;
	model.alignToClock = align;
	model.enabled = enabled;
	m_cycleBar.SetModel(model);

	tstring summary;
	if (!enabled)
		summary = _T("Periodic breaks are off. The warning and postponing settings below still apply to scheduled breaks.");
	else {
		summary = std::format(_T("Work {} min, then a {}-min break ({}-min cycle), with a {}-min warning first."),
			t.workMin(), t.breakMin, t.periodMin, t.alertMin);
		if (delayCount > 0)
			summary += std::format(_T(" Postponing (up to {} × {} min) eats into the next cycle."),
				delayCount, delayMin);
		if (align)
			summary += _T(" Cycles start on the hour.");
	}
	SetDlgItemText(IDC_CYCLE_SUMMARY, summary.c_str());

	for (int id : PERIODIC_CONTROLS)
		Enable(id, enabled);
}

LRESULT CBreaksSettingsSection::OnEditChange(WORD, WORD, HWND, BOOL&)
{
	if (m_loading)
		return 0;
	updateView();
	NotifyChanged();
	return 0;
}

LRESULT CBreaksSettingsSection::OnEditKillFocus(WORD, WORD id, HWND, BOOL&)
{
	if (m_loading)
		return 0;

	// Normalise the timing fields so they stay consistent with each other.
	if (id == IDC_PERIOD || id == IDC_BREAK || id == IDC_ALERT) {
		CycleTiming t = currentTiming();
		m_loading = true;
		if (GetInt(IDC_PERIOD, -1) != t.periodMin) SetInt(IDC_PERIOD, t.periodMin);
		if (GetInt(IDC_BREAK, -1) != t.breakMin) SetInt(IDC_BREAK, t.breakMin);
		if (GetInt(IDC_ALERT, -1) != t.alertMin) SetInt(IDC_ALERT, t.alertMin);
		m_loading = false;
		updateView();
	}
	return 0;
}

LRESULT CBreaksSettingsSection::OnButtonClicked(WORD, WORD id, HWND, BOOL& bHandled)
{
	if (m_loading)
		return 0;

	bool preset = applyPreset(PERIOD_PRESETS, std::size(PERIOD_PRESETS), id, IDC_PERIOD)
		|| applyPreset(BREAK_PRESETS, std::size(BREAK_PRESETS), id, IDC_BREAK)
		|| applyPreset(ALERT_PRESETS, std::size(ALERT_PRESETS), id, IDC_ALERT);

	if (preset || id == IDC_PERIODIC_ENABLE || id == IDC_SYNC || id == IDC_BEEP) {
		if (preset) {
			// keep break and warning consistent with a shorter cycle
			CycleTiming t = currentTiming();
			m_loading = true;
			SetInt(IDC_BREAK, t.breakMin);
			SetInt(IDC_ALERT, t.alertMin);
			m_loading = false;
		}
		updateView();
		NotifyChanged();
	}
	else
		bHandled = FALSE;

	return 0;
}
