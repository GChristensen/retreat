module;

#include "wtl.h"
#include "resource.h"

#include <objidl.h>
#include <gdiplus.h>

#include "ColorButton.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <memory>
#include <string>
#include <vector>
#include "tstring.h"

export module BreakScreenSettingsSection;

import Settings;
import SectionDlg;
import SettingsControls;
import system;

// Posted to the main settings dialog to show the break screen briefly with
// the values currently entered in the dialog.
export constexpr UINT WM_TRY_BREAK_SCREEN = WM_APP + 0x52;

static const std::vector<tstring> IMAGE_EXTENSIONS = { _T("jp*"), _T("bmp"), _T("gif"), _T("png") };
static const std::vector<tstring> SOUND_EXTENSIONS = { _T("wav"), _T("mp3"), _T("wma"), _T("flac") };

static const UINT_PTR FOLDER_TIMER_ID = 1;

static int alphaToPercent(int alpha) { return std::clamp((int)std::lround(alpha * 100.0 / 255), 10, 100); }
static int percentToAlpha(int percent) { return std::clamp((int)std::lround(percent * 255.0 / 100), 0, 255); }

export class CBreakScreenSettingsSection : public CSectionDlg
{
public:
	CBreakScreenSettingsSection();

	virtual void read(Settings &settings) override;
	virtual void write(Settings &settings) override;

	BEGIN_MSG_MAP(CBreakScreenSettingsSection)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(IDC_BROWSE_IMAGE_FOLDER, OnBrowseImageFolder)
		COMMAND_ID_HANDLER(IDC_BROWSE_SOUND_FOLDER, OnBrowseSoundFolder)
		COMMAND_ID_HANDLER(IDC_PLAY_SOUND, OnPlaySound)
		COMMAND_ID_HANDLER(IDC_TRY_BREAK, OnTryBreak)
		COMMAND_HANDLER(IDC_IMAGE_FOLDER, EN_CHANGE, OnImageFolderChange)
		COMMAND_HANDLER(IDC_SOUND_FOLDER, EN_CHANGE, OnSimpleChange)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
		NOTIFY_CODE_HANDLER(CPN_SELCHANGE, OnColorChange)
		CHAIN_MSG_MAP(CSectionDlg)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

private:
	CBreakPreview m_preview;
	CTrackBarCtrl m_opacity;
	CColorButton m_backgroundColor;
	CColorButton m_timerColor;
	std::shared_ptr<Gdiplus::Bitmap> m_sampleImage;
	int m_imageCount = 0;

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnHScroll(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnTimer(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnBrowseImageFolder(WORD, WORD, HWND, BOOL&);
	LRESULT OnBrowseSoundFolder(WORD, WORD, HWND, BOOL&);
	LRESULT OnPlaySound(WORD, WORD, HWND, BOOL&);
	LRESULT OnTryBreak(WORD, WORD, HWND, BOOL&);
	LRESULT OnImageFolderChange(WORD, WORD, HWND, BOOL&);
	LRESULT OnSimpleChange(WORD, WORD, HWND, BOOL&);
	LRESULT OnButtonClicked(WORD, WORD, HWND, BOOL&);
	LRESULT OnColorChange(int, LPNMHDR, BOOL&);

	tstring getText(int id);
	bool browseFolder(int editId, int titleId);
	void loadSampleImage();
	void updateView();
};

module :private;

CBreakScreenSettingsSection::CBreakScreenSettingsSection() :
	CSectionDlg(IDD_BREAK_SCREEN)
{
	SetOptionsCategoryName(CString(MAKEINTRESOURCE(IDS_BREAK_SCREEN_SECTION_NAME)));
}

LRESULT CBreakScreenSettingsSection::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	InitSectionHeader();
	MarkMuted(IDC_PREVIEW_CAPTION);
	MarkMuted(IDC_IMAGE_COUNT);

	m_preview.SubclassWindow(GetDlgItem(IDC_BREAK_PREVIEW));

	m_opacity.Attach(GetDlgItem(IDC_OPACITY_SLIDER));
	m_opacity.SetRange(10, 100);
	m_opacity.SetPageSize(10);
	m_opacity.SetLineSize(5);

	m_backgroundColor.SubclassWindow(GetDlgItem(IDC_BACKGROUND_COLOR));
	m_timerColor.SubclassWindow(GetDlgItem(IDC_TIMER_TEXT_COLOR));

	CString defaultText(MAKEINTRESOURCE(IDS_DEFAULT_COLOR_TEXT));
	CString customText(MAKEINTRESOURCE(IDS_CUSTOM_COLOR_TEXT));

	m_backgroundColor.SetDefaultText(defaultText);
	m_backgroundColor.SetCustomText(customText);
	m_backgroundColor.SetDefaultColor(Settings::DEFAULT_APPEARANCE_BACKGROUND_COLOR);
	m_timerColor.SetDefaultText(defaultText);
	m_timerColor.SetCustomText(customText);
	m_timerColor.SetDefaultColor(Settings::DEFAULT_APPEARANCE_TIMER_TEXT_COLOR);

	CEdit(GetDlgItem(IDC_IMAGE_FOLDER)).SetCueBannerText(L"A folder with .jpg, .png, .gif or .bmp images");
	CEdit(GetDlgItem(IDC_SOUND_FOLDER)).SetCueBannerText(L"Empty: play the system sound");

	return TRUE;
}

void CBreakScreenSettingsSection::read(Settings &settings)
{
	m_loading = true;

	bool fullscreen = settings.getBoolean(Settings::APPEARANCE_FULLSCREEN, Settings::DEFAULT_APPEARANCE_FULLSCREEN);
	CheckRadioButton(IDC_COVER_FULL, IDC_COVER_WINDOW, fullscreen ? IDC_COVER_FULL : IDC_COVER_WINDOW);

	m_opacity.SetPos(alphaToPercent(settings.getInt(Settings::APPEARANCE_OPACITY_LEVEL,
		Settings::DEFAULT_APPEARANCE_OPACITY_LEVEL)));

	SetChecked(IDC_SHOW_TIMER, settings.getBoolean(Settings::APPEARANCE_SHOW_TIMER, Settings::DEFAULT_APPEARANCE_SHOW_TIMER));
	m_timerColor.SetColor(settings.getInt(Settings::APPEARANCE_TIMER_TEXT_COLOR, Settings::DEFAULT_APPEARANCE_TIMER_TEXT_COLOR));

	tstring imageDir = settings.getString(Settings::APPEARANCE_IMAGE_DIRECTORY, _T(""));
	bool useImages = settings.getBoolean(Settings::APPEARANCE_USE_IMAGES, !imageDir.empty());
	CheckRadioButton(IDC_PICTURE_STARS, IDC_PICTURE_IMAGES, useImages ? IDC_PICTURE_IMAGES : IDC_PICTURE_STARS);
	SetDlgItemText(IDC_IMAGE_FOLDER, imageDir.c_str());

	bool stretch = settings.getBoolean(Settings::APPEARANCE_STRETCH_IMAGES, Settings::DEFAULT_APPEARANCE_STRETCH_IMAGES);
	CheckRadioButton(IDC_FIT_FILL, IDC_FIT_ORIGINAL, stretch ? IDC_FIT_FILL : IDC_FIT_ORIGINAL);

	m_backgroundColor.SetColor(settings.getInt(Settings::APPEARANCE_BACKGROUND_COLOR, Settings::DEFAULT_APPEARANCE_BACKGROUND_COLOR));
	SetChecked(IDC_TRANSPARENT_REGIONS, settings.getBoolean(Settings::APPEARANCE_TRANSPARENT, Settings::DEFAULT_APPEARANCE_TRANSPARENT));

	SetDlgItemText(IDC_SOUND_FOLDER, settings.getString(Settings::SOUNDS_AUDIO_DIRECTORY, _T("")).c_str());

	m_loading = false;

	loadSampleImage();
	updateView();
}

void CBreakScreenSettingsSection::write(Settings &settings)
{
	settings.setBoolean(Settings::APPEARANCE_FULLSCREEN, IsChecked(IDC_COVER_FULL));
	settings.setInt(Settings::APPEARANCE_OPACITY_LEVEL, percentToAlpha(m_opacity.GetPos()));
	settings.setBoolean(Settings::APPEARANCE_SHOW_TIMER, IsChecked(IDC_SHOW_TIMER));
	settings.setInt(Settings::APPEARANCE_TIMER_TEXT_COLOR, m_timerColor.GetColor());
	settings.setBoolean(Settings::APPEARANCE_USE_IMAGES, IsChecked(IDC_PICTURE_IMAGES));
	settings.setString(Settings::APPEARANCE_IMAGE_DIRECTORY, getText(IDC_IMAGE_FOLDER));
	settings.setBoolean(Settings::APPEARANCE_STRETCH_IMAGES, IsChecked(IDC_FIT_FILL));
	settings.setInt(Settings::APPEARANCE_BACKGROUND_COLOR, m_backgroundColor.GetColor());
	settings.setBoolean(Settings::APPEARANCE_TRANSPARENT, IsChecked(IDC_TRANSPARENT_REGIONS));
	settings.setString(Settings::SOUNDS_AUDIO_DIRECTORY, getText(IDC_SOUND_FOLDER));
}

tstring CBreakScreenSettingsSection::getText(int id)
{
	CString text;
	GetDlgItemText(id, text);
	text.Trim();
	return (LPCTSTR)text;
}

void CBreakScreenSettingsSection::loadSampleImage()
{
	m_sampleImage.reset();
	m_imageCount = 0;

	tstring dir = getText(IDC_IMAGE_FOLDER);
	if (dir.empty())
		return;

	std::vector<tstring> files;
	for (const tstring& ext : IMAGE_EXTENSIONS)
		searchForFiles(dir, ext, files);

	m_imageCount = (int)files.size();
	if (files.empty())
		return;

	std::sort(files.begin(), files.end());

	// keep a small copy: the preview is only a few hundred pixels wide
	std::unique_ptr<Gdiplus::Bitmap> full(Gdiplus::Bitmap::FromFile(files.front().c_str()));
	if (!full || full->GetLastStatus() != Gdiplus::Ok || !full->GetWidth() || !full->GetHeight())
		return;

	const double maxSide = 640;
	double scale = (std::min)(1.0, maxSide / (std::max)(full->GetWidth(), full->GetHeight()));
	int w = (std::max)(1, (int)(full->GetWidth() * scale));
	int h = (std::max)(1, (int)(full->GetHeight() * scale));

	auto thumb = std::make_shared<Gdiplus::Bitmap>(w, h, PixelFormat32bppARGB);
	{
		Gdiplus::Graphics g(thumb.get());
		g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		g.DrawImage(full.get(), 0, 0, w, h);
	}

	// sample the corner color from the original, as the break screen does
	Gdiplus::Color corner;
	full->GetPixel(0, 0, &corner);
	thumb->SetPixel(0, 0, corner);

	m_sampleImage = thumb;
}

void CBreakScreenSettingsSection::updateView()
{
	if (m_loading)
		return;

	bool images = IsChecked(IDC_PICTURE_IMAGES);
	bool showTimer = IsChecked(IDC_SHOW_TIMER);
	tstring imageDir = getText(IDC_IMAGE_FOLDER);

	SetDlgItemText(IDC_OPACITY_VALUE, std::format(_T("{}%"), m_opacity.GetPos()).c_str());

	for (int id : { IDC_IMAGE_FOLDER, IDC_BROWSE_IMAGE_FOLDER, IDC_FIT_FILL, IDC_FIT_ORIGINAL,
	                IDC_BACKGROUND_LABEL, IDC_BACKGROUND_COLOR, IDC_TRANSPARENT_REGIONS, IDC_IMAGE_COUNT })
		Enable(id, images);
	Enable(IDC_TIMER_TEXT_COLOR, showTimer);

	tstring count;
	if (!images)
		count = _T("A new pattern every break");
	else if (imageDir.empty())
		count = _T("Choose a folder below");
	else if (m_imageCount == 0)
		count = _T("No images found in this folder");
	else
		count = std::format(_T("{} image{} found"), m_imageCount, m_imageCount == 1 ? _T("") : _T("s"));
	SetDlgItemText(IDC_IMAGE_COUNT, count.c_str());

	PreviewModel model;
	model.fullscreen = IsChecked(IDC_COVER_FULL);
	model.useImages = images;
	model.image = m_sampleImage;
	model.stretch = IsChecked(IDC_FIT_FILL);
	model.backgroundColor = m_backgroundColor.GetColor();
	model.alpha = percentToAlpha(m_opacity.GetPos());
	model.transparentRegions = IsChecked(IDC_TRANSPARENT_REGIONS);
	model.showTimer = showTimer;
	model.timerColor = m_timerColor.GetColor();
	m_preview.SetModel(model);

	tstring caption;
	if (m_preview.IsStarFallback())
		caption = _T("Preview: without an image, \"A window\" shows full-screen stars");
	else
		caption = std::format(_T("Preview: {}, {}"), model.fullscreen ? _T("whole screen") : _T("a window"),
			images && m_sampleImage ? _T("image") : _T("random stars"));
	SetDlgItemText(IDC_PREVIEW_CAPTION, caption.c_str());
}

bool CBreakScreenSettingsSection::browseFolder(int editId, int titleId)
{
	CString title(MAKEINTRESOURCE(titleId));
	CFolderDialog dlg(NULL, title);

	tstring current = getText(editId);
	if (!current.empty())
		dlg.SetInitialFolder(current.c_str());

	if (dlg.DoModal(GetParent()) == IDOK) {
		SetDlgItemText(editId, dlg.GetFolderPath());
		return true;
	}
	return false;
}

LRESULT CBreakScreenSettingsSection::OnHScroll(UINT, WPARAM, LPARAM lParam, BOOL& bHandled)
{
	if ((HWND)lParam != m_opacity.m_hWnd) {
		bHandled = FALSE;
		return 0;
	}
	updateView();
	NotifyChanged();
	return 0;
}

LRESULT CBreakScreenSettingsSection::OnTimer(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
{
	if (wParam != FOLDER_TIMER_ID) {
		bHandled = FALSE;
		return 0;
	}
	KillTimer(FOLDER_TIMER_ID);
	loadSampleImage();
	updateView();
	return 0;
}

LRESULT CBreakScreenSettingsSection::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	KillTimer(FOLDER_TIMER_ID);
	m_sampleImage.reset();
	bHandled = FALSE;
	return 0;
}

LRESULT CBreakScreenSettingsSection::OnBrowseImageFolder(WORD, WORD, HWND, BOOL&)
{
	if (browseFolder(IDC_IMAGE_FOLDER, IDS_BROWSE_IMAGE_FOLDER_TITLE)) {
		CheckRadioButton(IDC_PICTURE_STARS, IDC_PICTURE_IMAGES, IDC_PICTURE_IMAGES);
		KillTimer(FOLDER_TIMER_ID);
		loadSampleImage();
		updateView();
	}
	return 0;
}

LRESULT CBreakScreenSettingsSection::OnBrowseSoundFolder(WORD, WORD, HWND, BOOL&)
{
	browseFolder(IDC_SOUND_FOLDER, IDS_BROWSE_SOUND_FOLDER_TITLE);
	return 0;
}

LRESULT CBreakScreenSettingsSection::OnPlaySound(WORD, WORD, HWND, BOOL&)
{
	tstring dir = getText(IDC_SOUND_FOLDER);
	tstring file = dir.empty() ? tstring() : getRandomFile(dir, SOUND_EXTENSIONS);

	if (!file.empty())
		playAudioFile(file);
	else
		playSystemSound();
	return 0;
}

LRESULT CBreakScreenSettingsSection::OnTryBreak(WORD, WORD, HWND, BOOL&)
{
	::PostMessage(GetParent(), WM_TRY_BREAK_SCREEN, 0, 0);
	return 0;
}

LRESULT CBreakScreenSettingsSection::OnImageFolderChange(WORD, WORD, HWND, BOOL&)
{
	if (m_loading)
		return 0;
	SetTimer(FOLDER_TIMER_ID, 400);  // look for images once typing pauses
	NotifyChanged();
	return 0;
}

LRESULT CBreakScreenSettingsSection::OnSimpleChange(WORD, WORD, HWND, BOOL&)
{
	if (!m_loading)
		NotifyChanged();
	return 0;
}

LRESULT CBreakScreenSettingsSection::OnButtonClicked(WORD, WORD id, HWND, BOOL& bHandled)
{
	switch (id) {
	case IDC_COVER_FULL: case IDC_COVER_WINDOW:
	case IDC_PICTURE_STARS: case IDC_PICTURE_IMAGES:
	case IDC_FIT_FILL: case IDC_FIT_ORIGINAL:
	case IDC_SHOW_TIMER: case IDC_TRANSPARENT_REGIONS:
		updateView();
		NotifyChanged();
		return 0;
	}
	bHandled = FALSE;
	return 0;
}

LRESULT CBreakScreenSettingsSection::OnColorChange(int, LPNMHDR, BOOL&)
{
	updateView();
	NotifyChanged();
	return 0;
}
