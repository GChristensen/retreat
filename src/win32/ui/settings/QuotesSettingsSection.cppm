// The "Quotes" page of the settings: a picture and a quote shown on the break
// screen, their look and the list of quotes.
module;

#include "wtl.h"
#include "resource.h"

#include <objidl.h>
#include <gdiplus.h>

#include "ColorButton.h"

#include <algorithm>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <vector>
#include "tstring.h"

export module QuotesSettingsSection;

import Settings;
import SectionDlg;
import SettingsControls;
import SettingsModel;
import BreakScreenSettingsSection;
import QuoteCard;

// In QUOTE_SIZES order.
static const int SIZE_IDS[] = { IDC_QUOTE_SIZE_S, IDC_QUOTE_SIZE_M, IDC_QUOTE_SIZE_L, IDC_QUOTE_SIZE_XL };

// In QuoteImage order.
static const int IMAGE_IDS[] = { IDC_QUOTE_IMAGE_THINKER, IDC_QUOTE_IMAGE_CUSTOM, IDC_QUOTE_IMAGE_NONE };

// Everything but the "Enabled" box and the "Try it" button.
static const int OPTION_CONTROLS[] = {
	IDC_QUOTE_IMAGE_THINKER, IDC_QUOTE_IMAGE_CUSTOM, IDC_QUOTE_IMAGE_NONE, IDC_QUOTE_IMAGE_FILE,
	IDC_QUOTE_LABEL_SHOW, IDC_QUOTE_BACKDROP_PICTURE, IDC_QUOTE_BACKDROP_PLAIN,
	IDC_QUOTE_LABEL_FONT, IDC_QUOTE_FONT, IDC_QUOTE_ITALIC,
	IDC_QUOTE_LABEL_COLOR, IDC_QUOTE_COLOR,
	IDC_QUOTE_LABEL_SIZE, IDC_QUOTE_SIZE_S, IDC_QUOTE_SIZE_M, IDC_QUOTE_SIZE_L, IDC_QUOTE_SIZE_XL,
	IDC_QUOTE_GROUP, IDC_QUOTE_LABEL_LIST, IDC_QUOTE_COUNT, IDC_QUOTE_ORDER_RANDOM, IDC_QUOTE_ORDER_SEQUENCE,
	IDC_QUOTE_TEXT };

export class CQuotesSettingsSection : public CSectionDlg
{
public:
	CQuotesSettingsSection();

	virtual void read(Settings &settings) override;
	virtual void write(Settings &settings) override;

	BEGIN_MSG_MAP(CQuotesSettingsSection)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(IDC_TRY_BREAK, OnTryBreak)
		COMMAND_HANDLER(IDC_QUOTE_TEXT, EN_CHANGE, OnTextChange)
		COMMAND_HANDLER(IDC_QUOTE_FONT, CBN_SELCHANGE, OnFontChange)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
		NOTIFY_CODE_HANDLER(CPN_SELCHANGE, OnColorChange)
		CHAIN_MSG_MAP(CSectionDlg)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

private:
	CQuotePreview m_preview;
	CColorButton m_color;
	CComboBox m_font;

	tstring m_imageFile;
	std::shared_ptr<Gdiplus::Bitmap> m_thinkerImage;
	std::shared_ptr<Gdiplus::Bitmap> m_customImage;
	// the picture chosen before the last click, to go back to when the
	// file dialog is cancelled
	QuoteImage m_image = QuoteImage::Thinker;
	int m_size = Settings::DEFAULT_QUOTES_FONT_SIZE;  // kept when not one of the presets

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnTryBreak(WORD, WORD, HWND, BOOL&);
	LRESULT OnTextChange(WORD, WORD, HWND, BOOL&);
	LRESULT OnFontChange(WORD, WORD, HWND, BOOL&);
	LRESULT OnButtonClicked(WORD, WORD, HWND, BOOL&);
	LRESULT OnColorChange(int, LPNMHDR, BOOL&);

	tstring getText(int id);
	tstring fontFace();
	void selectFont(const tstring& face);
	int quoteSize();
	QuoteImage selectedImage();
	bool chooseImage();
	void loadCustomImage();
	void updateView();
};

module :private;

static int CALLBACK addFontFamily(const LOGFONT* lf, const TEXTMETRIC*, DWORD type, LPARAM lParam)
{
	// vertical variants start with '@'; GDI+ draws only TrueType and OpenType-TT fonts
	if (lf->lfFaceName[0] == _T('@') || !(type & TRUETYPE_FONTTYPE))
		return 1;

	CComboBox combo((HWND)lParam);
	if (combo.FindStringExact(-1, lf->lfFaceName) == CB_ERR)
		combo.AddString(lf->lfFaceName);

	return 1;
}

CQuotesSettingsSection::CQuotesSettingsSection() :
	CSectionDlg(IDD_QUOTES)
{
	SetOptionsCategoryName(CString(MAKEINTRESOURCE(IDS_QUOTES_SECTION_NAME)));
}

LRESULT CQuotesSettingsSection::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	InitSectionHeader();
	MarkMuted(IDC_QUOTE_IMAGE_FILE);
	MarkMuted(IDC_QUOTE_COUNT);

	m_preview.SubclassWindow(GetDlgItem(IDC_QUOTE_PREVIEW));
	m_thinkerImage = loadThinkerImage();

	m_color.SubclassWindow(GetDlgItem(IDC_QUOTE_COLOR));
	m_color.SetDefaultText(CString(MAKEINTRESOURCE(IDS_DEFAULT_COLOR_TEXT)));
	m_color.SetCustomText(CString(MAKEINTRESOURCE(IDS_CUSTOM_COLOR_TEXT)));
	m_color.SetDefaultColor(Settings::DEFAULT_QUOTES_TEXT_COLOR);

	m_font.Attach(GetDlgItem(IDC_QUOTE_FONT));

	LOGFONT lf{};
	lf.lfCharSet = DEFAULT_CHARSET;
	CClientDC dc(m_hWnd);
	::EnumFontFamiliesEx(dc, &lf, (FONTENUMPROC)addFontFamily, (LPARAM)m_font.m_hWnd, 0);

	CEdit text(GetDlgItem(IDC_QUOTE_TEXT));
	text.SetLimitText(0);

	return TRUE;
}

void CQuotesSettingsSection::read(Settings &settings)
{
	m_loading = true;

	SetChecked(IDC_QUOTES_ENABLE, settings.getBoolean(Settings::QUOTES_ENABLE, Settings::DEFAULT_QUOTES_ENABLE));

	bool plain = settings.getBoolean(Settings::QUOTES_PLAIN_BACKDROP, Settings::DEFAULT_QUOTES_PLAIN_BACKDROP);
	CheckRadioButton(IDC_QUOTE_BACKDROP_PICTURE, IDC_QUOTE_BACKDROP_PLAIN,
		plain ? IDC_QUOTE_BACKDROP_PLAIN : IDC_QUOTE_BACKDROP_PICTURE);

	selectFont(settings.getString(Settings::QUOTES_FONT_FACE, DEFAULT_QUOTES_FONT_FACE));
	SetChecked(IDC_QUOTE_ITALIC, settings.getBoolean(Settings::QUOTES_FONT_ITALIC, Settings::DEFAULT_QUOTES_FONT_ITALIC));
	m_color.SetColor(settings.getInt(Settings::QUOTES_TEXT_COLOR, Settings::DEFAULT_QUOTES_TEXT_COLOR));

	m_size = std::clamp(settings.getInt(Settings::QUOTES_FONT_SIZE, Settings::DEFAULT_QUOTES_FONT_SIZE),
		MIN_QUOTE_SIZE, MAX_QUOTE_SIZE);
	for (int i = 0; i < (int)std::size(SIZE_IDS); ++i)
		SetChecked(SIZE_IDS[i], QUOTE_SIZES[i] == m_size);

	m_imageFile = settings.getString(Settings::QUOTES_IMAGE_FILE, _T(""));
	m_image = toQuoteImage(settings.getInt(Settings::QUOTES_IMAGE, Settings::DEFAULT_QUOTES_IMAGE));
	if (m_image == QuoteImage::Custom && m_imageFile.empty())
		m_image = QuoteImage::Thinker;
	CheckRadioButton(IDC_QUOTE_IMAGE_THINKER, IDC_QUOTE_IMAGE_NONE, IMAGE_IDS[(int)m_image]);

	bool sequential = settings.getBoolean(Settings::QUOTES_SEQUENTIAL, Settings::DEFAULT_QUOTES_SEQUENTIAL);
	CheckRadioButton(IDC_QUOTE_ORDER_RANDOM, IDC_QUOTE_ORDER_SEQUENCE,
		sequential ? IDC_QUOTE_ORDER_SEQUENCE : IDC_QUOTE_ORDER_RANDOM);

	SetDlgItemText(IDC_QUOTE_TEXT, joinQuoteLines(loadQuoteLines(settings)).c_str());

	m_loading = false;

	loadCustomImage();
	updateView();
}

void CQuotesSettingsSection::write(Settings &settings)
{
	settings.setBoolean(Settings::QUOTES_ENABLE, IsChecked(IDC_QUOTES_ENABLE));
	settings.setBoolean(Settings::QUOTES_PLAIN_BACKDROP, IsChecked(IDC_QUOTE_BACKDROP_PLAIN));
	settings.setString(Settings::QUOTES_FONT_FACE, fontFace());
	settings.setBoolean(Settings::QUOTES_FONT_ITALIC, IsChecked(IDC_QUOTE_ITALIC));
	settings.setInt(Settings::QUOTES_TEXT_COLOR, m_color.GetColor());
	settings.setInt(Settings::QUOTES_FONT_SIZE, quoteSize());
	settings.setInt(Settings::QUOTES_IMAGE, (int)selectedImage());
	settings.setString(Settings::QUOTES_IMAGE_FILE, m_imageFile);
	settings.setBoolean(Settings::QUOTES_SEQUENTIAL, IsChecked(IDC_QUOTE_ORDER_SEQUENCE));

	std::vector<tstring> lines = splitQuoteLines(getText(IDC_QUOTE_TEXT));

	// untouched built-in quotes stay built-in, so that they follow updates
	bool builtIn = settings.getInt(Settings::QUOTES_COUNT, -1) < 0 && lines == defaultQuoteLines();
	if (!builtIn)
		saveQuoteLines(settings, lines);
}

tstring CQuotesSettingsSection::getText(int id)
{
	CString text;
	GetDlgItemText(id, text);
	return (LPCTSTR)text;
}

tstring CQuotesSettingsSection::fontFace()
{
	int index = m_font.GetCurSel();
	if (index == CB_ERR)
		return DEFAULT_QUOTES_FONT_FACE;

	CString face;
	m_font.GetLBText(index, face);
	return (LPCTSTR)face;
}

void CQuotesSettingsSection::selectFont(const tstring& face)
{
	int index = m_font.FindStringExact(-1, face.c_str());

	// not installed (any more): keep it, GDI+ falls back to a serif font
	if (index == CB_ERR)
		index = m_font.AddString(face.c_str());

	m_font.SetCurSel(index);
}

int CQuotesSettingsSection::quoteSize()
{
	for (int i = 0; i < (int)std::size(SIZE_IDS); ++i)
		if (IsChecked(SIZE_IDS[i]))
			return QUOTE_SIZES[i];
	return m_size;
}

QuoteImage CQuotesSettingsSection::selectedImage()
{
	for (int i = 0; i < (int)std::size(IMAGE_IDS); ++i)
		if (IsChecked(IMAGE_IDS[i]))
			return (QuoteImage)i;
	return QuoteImage::Thinker;
}

bool CQuotesSettingsSection::chooseImage()
{
	tstring filter = _T("Images (*.jpg, *.png, *.gif, *.bmp)|*.jpg;*.jpeg;*.png;*.gif;*.bmp|All files|*.*|");
	std::replace(filter.begin(), filter.end(), _T('|'), (TCHAR)0);

	CFileDialog dlg(TRUE, NULL, m_imageFile.empty() ? NULL : m_imageFile.c_str(),
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, filter.c_str(), GetParent());

	if (dlg.DoModal(GetParent()) != IDOK)
		return false;

	m_imageFile = dlg.m_szFileName;
	loadCustomImage();
	return true;
}

void CQuotesSettingsSection::loadCustomImage()
{
	m_customImage.reset();

	if (m_imageFile.empty())
		return;

	std::unique_ptr<Gdiplus::Bitmap> full(Gdiplus::Bitmap::FromFile(m_imageFile.c_str()));
	if (!full || full->GetLastStatus() != Gdiplus::Ok || !full->GetWidth() || !full->GetHeight())
		return;

	// keep a small copy: the preview is only a few hundred pixels wide
	const double maxSide = 480;
	double scale = (std::min)(1.0, maxSide / (std::max)(full->GetWidth(), full->GetHeight()));
	int w = (std::max)(1, (int)(full->GetWidth() * scale));
	int h = (std::max)(1, (int)(full->GetHeight() * scale));

	auto thumb = std::make_shared<Gdiplus::Bitmap>(w, h, PixelFormat32bppARGB);
	{
		Gdiplus::Graphics g(thumb.get());
		g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		g.DrawImage(full.get(), 0, 0, w, h);
	}

	m_customImage = thumb;
}

void CQuotesSettingsSection::updateView()
{
	if (m_loading)
		return;

	bool enabled = IsChecked(IDC_QUOTES_ENABLE);
	for (int id : OPTION_CONTROLS)
		Enable(id, enabled);

	QuoteImage image = selectedImage();

	tstring caption;
	switch (image) {
	case QuoteImage::Thinker:
		caption = _T("Auguste Rodin, The Thinker");
		break;
	case QuoteImage::Custom:
		caption = m_customImage ? std::filesystem::path(m_imageFile).filename().native()
		                        : tstring(_T("The image could not be read"));
		break;
	case QuoteImage::None:
		caption = _T("Only the quote");
		break;
	}
	SetDlgItemText(IDC_QUOTE_IMAGE_FILE, caption.c_str());

	std::vector<tstring> lines = splitQuoteLines(getText(IDC_QUOTE_TEXT));

	tstring count = lines.empty() ? tstring(_T("none: only the picture"))
		: std::format(_T("{} quote{}"), lines.size(), lines.size() == 1 ? _T("") : _T("s"));
	SetDlgItemText(IDC_QUOTE_COUNT, count.c_str());

	QuotePreviewModel model;
	model.enabled = enabled;
	model.plainBackdrop = IsChecked(IDC_QUOTE_BACKDROP_PLAIN);
	if (!lines.empty())
		model.quote = parseQuote(lines.front());
	model.face = fontFace();
	model.italic = IsChecked(IDC_QUOTE_ITALIC);
	model.color = m_color.GetColor();
	model.sizePt = quoteSize();
	model.image = image;
	model.thinkerImage = m_thinkerImage;
	model.customImage = m_customImage;
	m_preview.SetModel(model);
}

LRESULT CQuotesSettingsSection::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	m_customImage.reset();
	m_thinkerImage.reset();
	bHandled = FALSE;
	return 0;
}

LRESULT CQuotesSettingsSection::OnTryBreak(WORD, WORD, HWND, BOOL&)
{
	::PostMessage(GetParent(), WM_TRY_BREAK_SCREEN, 0, 0);
	return 0;
}

LRESULT CQuotesSettingsSection::OnTextChange(WORD, WORD, HWND, BOOL&)
{
	if (m_loading)
		return 0;
	updateView();
	NotifyChanged();
	return 0;
}

LRESULT CQuotesSettingsSection::OnFontChange(WORD, WORD, HWND, BOOL&)
{
	updateView();
	NotifyChanged();
	return 0;
}

LRESULT CQuotesSettingsSection::OnButtonClicked(WORD, WORD id, HWND, BOOL& bHandled)
{
	switch (id) {
	case IDC_QUOTE_IMAGE_CUSTOM: {
		// without a file yet, or clicked again: pick one
		if (m_imageFile.empty() || m_image == QuoteImage::Custom) {
			if (!chooseImage() && m_imageFile.empty()) {
				CheckRadioButton(IDC_QUOTE_IMAGE_THINKER, IDC_QUOTE_IMAGE_NONE, IMAGE_IDS[(int)m_image]);
				updateView();
				return 0;
			}
		}
		m_image = QuoteImage::Custom;
		updateView();
		NotifyChanged();
		return 0;
	}

	case IDC_QUOTE_IMAGE_THINKER: case IDC_QUOTE_IMAGE_NONE:
		m_image = selectedImage();
		updateView();
		NotifyChanged();
		return 0;

	case IDC_QUOTES_ENABLE:
	case IDC_QUOTE_BACKDROP_PICTURE: case IDC_QUOTE_BACKDROP_PLAIN:
	case IDC_QUOTE_ITALIC:
	case IDC_QUOTE_SIZE_S: case IDC_QUOTE_SIZE_M: case IDC_QUOTE_SIZE_L: case IDC_QUOTE_SIZE_XL:
	case IDC_QUOTE_ORDER_RANDOM: case IDC_QUOTE_ORDER_SEQUENCE:
		updateView();
		NotifyChanged();
		return 0;
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CQuotesSettingsSection::OnColorChange(int, LPNMHDR, BOOL&)
{
	updateView();
	NotifyChanged();
	return 0;
}
