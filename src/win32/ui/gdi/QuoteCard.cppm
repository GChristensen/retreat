// The quote card of the break screen: a picture (Rodin's The Thinker by
// default) above a quote and its author, on a dark rounded panel. Drawn with
// GDI+ both by the quote window and by the preview in the settings.
module;

#include "wtl.h"
#include "resource.h"

#include <objidl.h>
#include <gdiplus.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cwctype>
#include <memory>
#include <string>
#include "tstring.h"

export module QuoteCard;

import SettingsModel;

using namespace Gdiplus;

export struct QuoteLook {
	tstring face;                       // font of the quote
	bool italic = true;
	COLORREF color = RGB(0xEF, 0xE6, 0xD6);
	REAL fontPx = 34;                   // height of the quote font, pixels
	REAL unit = 1;                      // pixels per 1/96 inch: padding, gaps, corners
	REAL maxTextWidth = 800;            // the quote wraps at this width, pixels
	REAL imageHeight = 300;             // height of the picture, pixels
	QuoteImage image = QuoteImage::Thinker;
	Bitmap* thinkerImage = nullptr;     // for QuoteImage::Thinker; a drawing of it if null
	Bitmap* customImage = nullptr;      // for QuoteImage::Custom; none if null
};

// The picture of The Thinker from the resources, or null. GDI+ must be
// running for as long as the bitmap lives, so the caller owns it.
export std::unique_ptr<Bitmap> loadThinkerImage();

// Size of the whole card; empty when there is nothing to show.
export SizeF measureQuoteCard(const Quote& quote, const QuoteLook& look);

export void drawQuoteCard(Graphics& g, PointF origin, const Quote& quote, const QuoteLook& look);

// The Thinker, as large as fits into the box, standing on its bottom edge.
export void drawThinker(Graphics& g, const RectF& box);

module :private;

namespace {

const Color PANEL_COLOR(232, 0x10, 0x13, 0x18);
const Color PANEL_BORDER(36, 0xFF, 0xFF, 0xFF);
const Color BRONZE(0xFF, 0xA8, 0x86, 0x5A);
const Color ROCK(0xFF, 0x3D, 0x38, 0x32);

// the drawing is made in a 200 x 260 box
const REAL THINKER_W = 200, THINKER_H = 260;

// The figure and the rock of the drawing take this share of its height; the
// picture, which has no margins, is drawn at the same size.
const REAL THINKER_FIGURE_SHARE = 171.0f / 260;

struct CardFonts {
	std::unique_ptr<FontFamily> quoteFamily;
	std::unique_ptr<Font> quote;
	std::unique_ptr<FontFamily> authorFamily;
	std::unique_ptr<Font> author;
};

std::unique_ptr<FontFamily> family(const tstring& face, const FontFamily* fallback)
{
	auto result = std::make_unique<FontFamily>(face.c_str());
	if (result->GetLastStatus() != Ok || !result->IsAvailable())
		result.reset(fallback->Clone());
	return result;
}

CardFonts makeFonts(const QuoteLook& look)
{
	CardFonts fonts;

	fonts.quoteFamily = family(look.face, FontFamily::GenericSerif());
	int style = look.italic && fonts.quoteFamily->IsStyleAvailable(FontStyleItalic)
		? FontStyleItalic : FontStyleRegular;
	if (!fonts.quoteFamily->IsStyleAvailable(style))
		style = FontStyleBold;
	fonts.quote = std::make_unique<Font>(fonts.quoteFamily.get(), (std::max)(1.0f, look.fontPx), style, UnitPixel);

	fonts.authorFamily = family(_T("Segoe UI"), FontFamily::GenericSansSerif());
	REAL authorPx = (std::max)(look.fontPx * 0.45f, 11 * look.unit);
	fonts.author = std::make_unique<Font>(fonts.authorFamily.get(), (std::max)(1.0f, authorPx), FontStyleRegular, UnitPixel);

	return fonts;
}

tstring quotedText(const tstring& text)
{
	if (text.empty())
		return text;

	TCHAR first = text.front();
	if (first == _T('"') || first == _T('“') || first == _T('«') || first == _T('„'))
		return text;

	return _T("“") + text + _T("”");
}

tstring upper(const tstring& s)
{
	tstring result = s;
	for (TCHAR& c : result)
		c = (TCHAR)std::towupper((wint_t)c);
	return result;
}

void centeredFormat(StringFormat& format)
{
	format.SetAlignment(StringAlignmentCenter);
	format.SetLineAlignment(StringAlignmentNear);
}

struct Layout {
	SizeF card;
	REAL pad = 0;
	SizeF image;
	SizeF quote;
	SizeF author;
	REAL imageGap = 0;
	REAL authorGap = 0;
	tstring quoteText;
	tstring authorText;
};

SizeF imageSize(const QuoteLook& look)
{
	REAL h = (std::max)(0.0f, look.imageHeight);
	if (h < 1)
		return SizeF(0, 0);

	switch (look.image) {
	case QuoteImage::Thinker: {
		Bitmap* picture = look.thinkerImage;
		if (!picture || !picture->GetWidth() || !picture->GetHeight())
			return SizeF(h * THINKER_W / THINKER_H, h);
		REAL ph = h * THINKER_FIGURE_SHARE;
		return SizeF(ph * picture->GetWidth() / picture->GetHeight(), ph);
	}

	case QuoteImage::Custom: {
		if (!look.customImage || !look.customImage->GetWidth() || !look.customImage->GetHeight())
			return SizeF(0, 0);
		REAL iw = (REAL)look.customImage->GetWidth(), ih = (REAL)look.customImage->GetHeight();
		REAL scale = (std::min)(h / ih, (std::max)(1.0f, look.maxTextWidth) / iw);
		return SizeF(iw * scale, ih * scale);
	}

	default:
		return SizeF(0, 0);
	}
}

Layout layout(const Quote& quote, const QuoteLook& look, const CardFonts& fonts)
{
	Layout l;
	l.quoteText = quotedText(quote.text);
	l.authorText = upper(quote.author);
	l.image = imageSize(look);

	if (l.quoteText.empty() && l.image.Width < 1)
		return l;

	Bitmap probe(1, 1, PixelFormat32bppPARGB);
	Graphics measure(&probe);
	measure.SetTextRenderingHint(TextRenderingHintAntiAlias);

	StringFormat format;
	centeredFormat(format);

	REAL wrap = (std::max)(look.fontPx * 4, look.maxTextWidth);

	if (!l.quoteText.empty())
		measure.MeasureString(l.quoteText.c_str(), (INT)l.quoteText.size(), fonts.quote.get(),
			SizeF(wrap, 100000), &format, &l.quote);

	if (!l.quoteText.empty() && !l.authorText.empty())
		measure.MeasureString(l.authorText.c_str(), (INT)l.authorText.size(), fonts.author.get(),
			SizeF(wrap, 100000), &format, &l.author);

	l.pad = 36 * look.unit;
	l.imageGap = l.image.Height > 0 && l.quote.Height > 0 ? 24 * look.unit : 0;
	l.authorGap = l.author.Height > 0 ? 12 * look.unit : 0;

	REAL contentW = (std::max)({ l.image.Width, l.quote.Width, l.author.Width });
	REAL contentH = l.image.Height + l.imageGap + l.quote.Height + l.authorGap + l.author.Height;

	l.card = SizeF(std::ceil(contentW + 2 * l.pad), std::ceil(contentH + 2 * l.pad));
	return l;
}

void roundRect(GraphicsPath& path, const RectF& r, REAL radius)
{
	REAL d = (std::min)(radius * 2, (std::min)(r.Width, r.Height));
	if (d < 1) {
		path.AddRectangle(r);
		return;
	}
	path.AddArc(r.X, r.Y, d, d, 180, 90);
	path.AddArc(r.X + r.Width - d, r.Y, d, d, 270, 90);
	path.AddArc(r.X + r.Width - d, r.Y + r.Height - d, d, d, 0, 90);
	path.AddArc(r.X, r.Y + r.Height - d, d, d, 90, 90);
	path.CloseFigure();
}

} // namespace

void drawThinker(Graphics& g, const RectF& box)
{
	REAL scale = (std::min)(box.Width / THINKER_W, box.Height / THINKER_H);
	if (scale <= 0)
		return;

	GraphicsState state = g.Save();
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.TranslateTransform(box.X + (box.Width - THINKER_W * scale) / 2, box.Y + box.Height - THINKER_H * scale);
	g.ScaleTransform(scale, scale);

	SolidBrush shadow(Color(90, 0, 0, 0));
	g.FillEllipse(&shadow, 20.0f, 246.0f, 168.0f, 12.0f);

	// the rock: quadratic curves of the original sketch as cubic ones
	GraphicsPath rock;
	rock.AddBezier(PointF(36, 198), PointF(29.33f, 208.67f), PointF(28, 226.67f), PointF(32, 252));
	rock.AddLine(PointF(32, 252), PointF(176, 252));
	rock.AddBezier(PointF(176, 252), PointF(181.33f, 234.67f), PointF(179.33f, 218.67f), PointF(170, 204));
	rock.AddBezier(PointF(170, 204), PointF(135.33f, 192), PointF(90.67f, 190), PointF(36, 198));
	rock.CloseFigure();
	SolidBrush rockBrush(ROCK);
	g.FillPath(&rockBrush, &rock);

	// limbs and body as thick round strokes, back to front
	struct Stroke { PointF points[3]; int count; REAL width; };
	static const Stroke strokes[] = {
		{ { PointF(92, 188), PointF(146, 172) }, 2, 24 },                     // far thigh
		{ { PointF(146, 174), PointF(136, 236) }, 2, 18 },                    // far shin
		{ { PointF(134, 239), PointF(154, 241) }, 2, 10 },                    // far foot
		{ { PointF(70, 190), PointF(88, 150), PointF(114, 124) }, 3, 44 },    // hunched torso
		{ { PointF(118, 120), PointF(132, 112) }, 2, 16 },                    // neck
		{ { PointF(78, 196), PointF(158, 180) }, 2, 30 },                     // near thigh
		{ { PointF(158, 182), PointF(152, 240) }, 2, 21 },                    // near shin
		{ { PointF(150, 243), PointF(176, 245) }, 2, 12 },                    // near foot
		{ { PointF(100, 132), PointF(116, 168), PointF(152, 188) }, 3, 14 },  // arm on the knee
		{ { PointF(114, 126), PointF(154, 172), PointF(146, 126) }, 3, 17 },  // arm to the chin
	};

	for (const Stroke& stroke : strokes) {
		Pen pen(BRONZE, stroke.width);
		pen.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
		pen.SetLineJoin(LineJoinRound);
		g.DrawLines(&pen, stroke.points, stroke.count);
	}

	SolidBrush bronze(BRONZE);
	g.FillEllipse(&bronze, 142.0f - 17, 104.0f - 17, 34.0f, 34.0f); // head, bowed
	g.FillEllipse(&bronze, 146.0f - 10, 124.0f - 10, 20.0f, 20.0f); // fist under the chin

	g.Restore(state);
}

SizeF measureQuoteCard(const Quote& quote, const QuoteLook& look)
{
	CardFonts fonts = makeFonts(look);
	return layout(quote, look, fonts).card;
}

void drawQuoteCard(Graphics& g, PointF origin, const Quote& quote, const QuoteLook& look)
{
	CardFonts fonts = makeFonts(look);
	Layout l = layout(quote, look, fonts);

	if (l.card.Width < 1 || l.card.Height < 1)
		return;

	GraphicsState state = g.Save();
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetTextRenderingHint(TextRenderingHintAntiAlias);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	RectF card(origin.X, origin.Y, l.card.Width, l.card.Height);

	GraphicsPath panel;
	roundRect(panel, card, 18 * look.unit);
	SolidBrush panelBrush(PANEL_COLOR);
	g.FillPath(&panelBrush, &panel);

	GraphicsPath outline;
	roundRect(outline, RectF(card.X + 0.5f, card.Y + 0.5f, card.Width - 1, card.Height - 1), 18 * look.unit);
	Pen border(PANEL_BORDER, 1);
	g.DrawPath(&border, &outline);

	REAL y = card.Y + l.pad;
	REAL contentX = card.X + l.pad;
	REAL contentW = card.Width - 2 * l.pad;

	if (l.image.Height > 0) {
		RectF imageBox(contentX + (contentW - l.image.Width) / 2, y, l.image.Width, l.image.Height);

		if (look.image == QuoteImage::Thinker && look.thinkerImage)
			g.DrawImage(look.thinkerImage, imageBox);
		else if (look.image == QuoteImage::Thinker)
			drawThinker(g, imageBox);
		else if (look.customImage)
			g.DrawImage(look.customImage, imageBox);

		y += l.image.Height + l.imageGap;
	}

	StringFormat format;
	centeredFormat(format);

	BYTE r = GetRValue(look.color), gr = GetGValue(look.color), b = GetBValue(look.color);

	if (l.quote.Height > 0) {
		SolidBrush brush(Color(255, r, gr, b));
		RectF box(contentX, y, contentW, l.quote.Height + 1);
		g.DrawString(l.quoteText.c_str(), (INT)l.quoteText.size(), fonts.quote.get(), box, &format, &brush);
		y += l.quote.Height + l.authorGap;
	}

	if (l.author.Height > 0) {
		SolidBrush brush(Color(185, r, gr, b));
		RectF box(contentX, y, contentW, l.author.Height + 1);
		g.DrawString(l.authorText.c_str(), (INT)l.authorText.size(), fonts.author.get(), box, &format, &brush);
	}

	g.Restore(state);
}

std::unique_ptr<Bitmap> loadThinkerImage()
{
	HINSTANCE instance = _Module.GetResourceInstance();
	HRSRC resource = ::FindResource(instance, MAKEINTRESOURCE(IDR_THINKER), RT_RCDATA);
	if (!resource)
		return nullptr;

	DWORD size = ::SizeofResource(instance, resource);
	const void* data = ::LockResource(::LoadResource(instance, resource));
	if (!data || !size)
		return nullptr;

	HGLOBAL global = ::GlobalAlloc(GMEM_MOVEABLE, size);
	if (!global)
		return nullptr;

	void* copy = ::GlobalLock(global);
	std::memcpy(copy, data, size);
	::GlobalUnlock(global);

	IStream* stream = nullptr;
	if (FAILED(::CreateStreamOnHGlobal(global, TRUE, &stream))) {
		::GlobalFree(global);
		return nullptr;
	}

	// a bitmap made from a stream keeps reading it, so draw it into one of our
	// own; premultiplied alpha also makes it faster to draw
	std::unique_ptr<Bitmap> result;
	{
		std::unique_ptr<Bitmap> decoded(Bitmap::FromStream(stream));

		if (decoded && decoded->GetLastStatus() == Ok && decoded->GetWidth() && decoded->GetHeight()) {
			int w = (int)decoded->GetWidth(), h = (int)decoded->GetHeight();
			result = std::make_unique<Bitmap>(w, h, PixelFormat32bppPARGB);
			Graphics g(result.get());
			g.Clear(Color(0, 0, 0, 0));
			g.DrawImage(decoded.get(), 0, 0, w, h);
		}
	}

	stream->Release();
	return result;
}
