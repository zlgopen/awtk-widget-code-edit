// Scintilla source code edit control
// PlatAWTK.cxx - implementation of platform facilities on AWTK+/Linux
// Copyright 1998-2004 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <sstream>

#include "awtk.h"
#include "Platform.h"

#include "Scintilla.h"
#include "ScintillaWidget.h"
#include "StringCopy.h"
#include "IntegerRectangle.h"
#include "XPM.h"
#include "UniConversion.h"

#include "Converter.h"

#ifdef _MSC_VER
// Ignore unreferenced local functions in AWTK+ headers
#pragma warning(disable : 4505)
#endif

using namespace Scintilla;

static color_t color_from_sci(ColourDesired from) {
  return color_init(from.GetRed(), from.GetGreen(), from.GetBlue(), 0xff);
}

namespace {

widget_t* WindowFromWidget(widget_t* w) noexcept {
  return widget_get_window(w);
}

widget_t* PWidget(WindowID wid) noexcept {
  return static_cast<widget_t*>(wid);
}

enum encodingType { singleByte, UTF8, dbcs };

// Holds a PangoFontDescription*.
class FontHandle {
 public:
  char* name;
  uint32_t size;
  uint32_t weight;
  bool_t italic;

 public:
  FontHandle() noexcept {
    this->name = NULL;
    this->weight = 400;
    this->italic = false;
    this->size = TK_DEFAULT_FONT_SIZE;
  }
  FontHandle(const FontParameters& fp) noexcept {
    this->name = NULL;
    this->weight = fp.weight;
    this->italic = fp.italic;
    this->size = TK_DEFAULT_FONT_SIZE;
  }
  ~FontHandle() {
    TKMEM_FREE(this->name);
  }
  static FontHandle* CreateNewFont(const FontParameters& fp);
};

FontHandle* FontHandle::CreateNewFont(const FontParameters& fp) {
  return new FontHandle(fp);
}

// X has a 16 bit coordinate space, so stop drawing here to avoid wrapping
const int maxCoordinate = 32000;

FontHandle* PFont(const Font& f) noexcept {
  return static_cast<FontHandle*>(f.GetID());
}

}  // namespace

Font::Font() noexcept : fid(nullptr) {
}

Font::~Font() {
}

void Font::Create(const FontParameters& fp) {
  Release();
  fid = FontHandle::CreateNewFont(fp);
}

void Font::Release() {
  if (fid) delete static_cast<FontHandle*>(fid);
  fid = nullptr;
}

// Required on OS X
namespace Scintilla {

// SurfaceID is a cairo_t*
class SurfaceImpl : public Surface {
  encodingType et;
  int x;
  int y;
  bool inited;
  bool createdGC;
  float ascent;
  float descent;
  float line_height;
  Converter conv;
  int characterSet;
  canvas_t* canvas;
  vgcanvas_t* vg;
  widget_t* widget;
  color_t pen_color;
  str_t str;
  wstr_t wstr;
  void SetConverter(int characterSet_);

  void SetFont(Font& font);
  vgcanvas_t* GetVgCanvas();

 public:
  SurfaceImpl() noexcept;
  ~SurfaceImpl() override;

  void Init(WindowID wid) override;
  void Init(SurfaceID sid, WindowID wid) override;
  void InitPixMap(int width, int height, Surface* surface_, WindowID wid) override;

  void Clear() noexcept;
  void Release() override;
  bool Initialised() override;
  void PenColour(ColourDesired fore) override;
  int LogPixelsY() override;
  int DeviceHeightFont(int points) override;
  void MoveTo(int x_, int y_) override;
  void LineTo(int x_, int y_) override;
  void Polygon(Point* pts, size_t npts, ColourDesired fore, ColourDesired back) override;
  void RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) override;
  void FillRectangle(PRectangle rc, ColourDesired back) override;
  void FillRectangle(PRectangle rc, Surface& surfacePattern) override;
  void RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) override;
  void AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
                      ColourDesired outline, int alphaOutline, int flags) override;
  void GradientRectangle(PRectangle rc, const std::vector<ColourStop>& stops,
                         GradientOptions options) override;
  void DrawRGBAImage(PRectangle rc, int width, int height,
                     const unsigned char* pixelsImage) override;
  void Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) override;
  void Copy(PRectangle rc, Point from, Surface& surfaceSource) override;

  std::unique_ptr<IScreenLineLayout> Layout(const IScreenLine* screenLine) override;

  void DrawTextBase(PRectangle rc, Font& font_, XYPOSITION ybase, std::string_view text,
                    ColourDesired fore);
  void DrawTextNoClip(PRectangle rc, Font& font_, XYPOSITION ybase, std::string_view text,
                      ColourDesired fore, ColourDesired back) override;
  void DrawTextClipped(PRectangle rc, Font& font_, XYPOSITION ybase, std::string_view text,
                       ColourDesired fore, ColourDesired back) override;
  void DrawTextTransparent(PRectangle rc, Font& font_, XYPOSITION ybase, std::string_view text,
                           ColourDesired fore) override;
  void MeasureWidths(Font& font_, std::string_view text, XYPOSITION* positions) override;
  XYPOSITION WidthText(Font& font_, std::string_view text) override;
  XYPOSITION Ascent(Font& font_) override;
  XYPOSITION Descent(Font& font_) override;
  XYPOSITION InternalLeading(Font& font_) override;
  XYPOSITION Height(Font& font_) override;
  XYPOSITION AverageCharWidth(Font& font_) override;

  void SetClip(PRectangle rc) override;
  void FlushCachedState() override;

  void SetUnicodeMode(bool unicodeMode_) override;
  void SetDBCSMode(int codePage) override;
  void SetBidiR2L(bool bidiR2L_) override;
};
}  // namespace Scintilla

const char* CharacterSetID(int characterSet) noexcept {
  switch (characterSet) {
    case SC_CHARSET_ANSI:
      return "";
    case SC_CHARSET_DEFAULT:
      return "ISO-8859-1";
    case SC_CHARSET_BALTIC:
      return "ISO-8859-13";
    case SC_CHARSET_CHINESEBIG5:
      return "BIG-5";
    case SC_CHARSET_EASTEUROPE:
      return "ISO-8859-2";
    case SC_CHARSET_GB2312:
      return "CP936";
    case SC_CHARSET_GREEK:
      return "ISO-8859-7";
    case SC_CHARSET_HANGUL:
      return "CP949";
    case SC_CHARSET_MAC:
      return "MACINTOSH";
    case SC_CHARSET_OEM:
      return "ASCII";
    case SC_CHARSET_RUSSIAN:
      return "KOI8-R";
    case SC_CHARSET_OEM866:
      return "CP866";
    case SC_CHARSET_CYRILLIC:
      return "CP1251";
    case SC_CHARSET_SHIFTJIS:
      return "SHIFT-JIS";
    case SC_CHARSET_SYMBOL:
      return "";
    case SC_CHARSET_TURKISH:
      return "ISO-8859-9";
    case SC_CHARSET_JOHAB:
      return "CP1361";
    case SC_CHARSET_HEBREW:
      return "ISO-8859-8";
    case SC_CHARSET_ARABIC:
      return "ISO-8859-6";
    case SC_CHARSET_VIETNAMESE:
      return "";
    case SC_CHARSET_THAI:
      return "ISO-8859-11";
    case SC_CHARSET_8859_15:
      return "ISO-8859-15";
    default:
      return "";
  }
}

void SurfaceImpl::SetConverter(int characterSet_) {
  assert(!"");
  if (characterSet != characterSet_) {
    characterSet = characterSet_;
    conv.Open("UTF-8", CharacterSetID(characterSet), false);
  }
}

SurfaceImpl::SurfaceImpl() noexcept
    : et(singleByte), x(0), y(0), inited(false), createdGC(false), characterSet(-1) {
  this->vg = NULL;
  this->canvas = NULL;
  this->widget = NULL;
  this->ascent = 15;
  this->descent = 5;
  this->line_height = 20;
  this->vg = NULL;
  this->canvas = NULL;
  this->pen_color = color_init(0, 0, 0, 0);

  str_init(&(this->str), 256);
  wstr_init(&(this->wstr), 256);
}

SurfaceImpl::~SurfaceImpl() {
  Clear();
  str_reset(&(this->str));
  wstr_reset(&(this->wstr));
}

void SurfaceImpl::Clear() noexcept {
  conv.Close();
  x = 0;
  y = 0;
  inited = false;
  createdGC = false;
  characterSet = -1;
}

void SurfaceImpl::Release() {
  Clear();
}

bool SurfaceImpl::Initialised() {
  return true;
}

void SurfaceImpl::Init(WindowID wid) {
  Release();
  PLATFORM_ASSERT(wid);

  this->inited = true;
  this->widget = WIDGET(wid);
  this->canvas = widget_get_canvas(this->widget);
}

void SurfaceImpl::Init(SurfaceID sid, WindowID wid) {
  Release();
  PLATFORM_ASSERT(sid);
  PLATFORM_ASSERT(wid);

  this->inited = true;
  this->widget = WIDGET(wid);
  this->canvas = (canvas_t*)sid;
}

vgcanvas_t* SurfaceImpl::GetVgCanvas() {
  vgcanvas_t* vg = canvas_get_vgcanvas(this->canvas);

  return vg;
}

void SurfaceImpl::InitPixMap(int width, int height, Surface* surface_, WindowID wid) {
  PLATFORM_ASSERT(surface_);
  PLATFORM_ASSERT(wid);
  Release();

  SurfaceImpl* surfImpl = static_cast<SurfaceImpl*>(surface_);
  et = surfImpl->et;
}

void SurfaceImpl::PenColour(ColourDesired fore) {
  this->pen_color = color_init(fore.GetRed(), fore.GetGreen(), fore.GetBlue(), 0xff);
}

int SurfaceImpl::LogPixelsY() {
  return 72;
}

int SurfaceImpl::DeviceHeightFont(int points) {
  const int logPix = LogPixelsY();
  return (points * logPix + logPix / 2) / 72;
}

void SurfaceImpl::MoveTo(int x, int y) {
  this->x = x;
  this->y = y;
  vgcanvas_t* vg = this->GetVgCanvas();
  return_if_fail(vg != NULL);
  int ox = this->canvas->ox;
  int oy = this->canvas->oy;

  vgcanvas_move_to(vg, x + ox, y + oy);
}

static int Delta(int difference) noexcept {
  if (difference < 0)
    return -1;
  else if (difference > 0)
    return 1;
  else
    return 0;
}

void SurfaceImpl::LineTo(int x, int y) {
  this->x = x;
  this->y = y;
  vgcanvas_t* vg = this->GetVgCanvas();
  return_if_fail(vg != NULL);

  int ox = this->canvas->ox;
  int oy = this->canvas->oy;
  vgcanvas_move_to(vg, x + ox, y + oy);
}

void SurfaceImpl::Polygon(Point* pts, size_t npts, ColourDesired fore, ColourDesired back) {
  vgcanvas_t* vg = this->GetVgCanvas();
  return_if_fail(vg != NULL);

  vgcanvas_begin_path(vg);
  vgcanvas_move_to(vg, pts[0].x + 0.5, pts[0].y + 0.5);
  for (size_t i = 1; i < npts; i++) {
    vgcanvas_line_to(vg, pts[i].x + 0.5, pts[i].y + 0.5);
  }
  vgcanvas_close_path(vg);

  vgcanvas_set_fill_color(vg, color_from_sci(back));
  vgcanvas_fill(vg);
  vgcanvas_set_stroke_color(vg, color_from_sci(fore));
  vgcanvas_stroke(vg);
}

void SurfaceImpl::RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) {
  vgcanvas_t* vg = this->GetVgCanvas();
  return_if_fail(vg != NULL);
  if (rc.left >= rc.right || rc.top >= rc.bottom) {
    return;
  }

  vgcanvas_begin_path(vg);
  int ox = this->canvas->ox;
  int oy = this->canvas->oy;
  vgcanvas_translate(vg, ox, oy);
  vgcanvas_rect(vg, rc.left + 0.5, rc.top + 0.5, rc.right - rc.left - 1, rc.bottom - rc.top - 1);
  vgcanvas_translate(vg, -ox, -oy);

  vgcanvas_set_fill_color(vg, color_from_sci(back));
  vgcanvas_fill(vg);
  vgcanvas_set_stroke_color(vg, color_from_sci(fore));
  vgcanvas_stroke(vg);
}

void SurfaceImpl::FillRectangle(PRectangle rc, ColourDesired back) {
  int w = rc.right - rc.left;
  int h = rc.bottom - rc.top; 
  vgcanvas_t* vg = this->GetVgCanvas();
  return_if_fail(vg != NULL);

  if (rc.left >= rc.right || rc.top >= rc.bottom) {
    return;
  }

  if(w > 200 && h > 200) {
  log_debug("bg\n");
  }
  rc.left = std::round(rc.left);
  rc.right = std::round(rc.right);

  int ox = this->canvas->ox;
  int oy = this->canvas->oy;
  vgcanvas_translate(vg, ox, oy);
  vgcanvas_rect(vg, rc.left, rc.top, w, h);
  vgcanvas_translate(vg, -ox, -oy);

  vgcanvas_set_fill_color(vg, color_from_sci(back));
  vgcanvas_fill(vg);
}

void SurfaceImpl::FillRectangle(PRectangle rc, Surface& surfacePattern) {
  FillRectangle(rc, ColourDesired(0));
}

void SurfaceImpl::RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) {
  vgcanvas_t* vg = this->GetVgCanvas();
  return_if_fail(vg != NULL);

  vgcanvas_begin_path(vg);
  int ox = this->canvas->ox;
  int oy = this->canvas->oy;
  vgcanvas_translate(vg, ox, oy);
  vgcanvas_rounded_rect(vg, rc.left + 0.5, rc.top + 0.5, rc.right - rc.left - 1,
                        rc.bottom - rc.top - 1, 5);
  vgcanvas_translate(vg, -ox, -oy);

  vgcanvas_set_fill_color(vg, color_from_sci(back));
  vgcanvas_fill(vg);
  vgcanvas_set_stroke_color(vg, color_from_sci(fore));
  vgcanvas_stroke(vg);
}

void SurfaceImpl::AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
                                 ColourDesired outline, int alphaOutline, int /*flags*/) {
  FillRectangle(rc, fill);
}

void SurfaceImpl::GradientRectangle(PRectangle rc, const std::vector<ColourStop>& stops,
                                    GradientOptions options) {
  assert(!"");
}

void SurfaceImpl::DrawRGBAImage(PRectangle rc, int width, int height,
                                const unsigned char* pixelsImage) {
  assert(!"");
}

void SurfaceImpl::Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) {
  vgcanvas_t* vg = this->GetVgCanvas();
  float_t cx = rc.right / 2 + rc.left;
  float_t cy = rc.bottom / 2 + rc.top;
  float_t rx = (rc.right - rc.left) / 2;
  float_t ry = (rc.bottom - rc.top) / 2;

  return_if_fail(vg != NULL);

  vgcanvas_begin_path(vg);
  int ox = this->canvas->ox;
  int oy = this->canvas->oy;
  vgcanvas_translate(vg, ox, oy);
  vgcanvas_ellipse(vg, cx, cy, rx, ry);
  vgcanvas_translate(vg, -ox, -oy);

  vgcanvas_set_fill_color(vg, color_from_sci(back));
  vgcanvas_fill(vg);
  vgcanvas_set_stroke_color(vg, color_from_sci(fore));
  vgcanvas_stroke(vg);
}

void SurfaceImpl::Copy(PRectangle rc, Point from, Surface& surfaceSource) {
}

std::unique_ptr<IScreenLineLayout> SurfaceImpl::Layout(const IScreenLine*) {
  return {};
}

std::string UTF8FromLatin1(std::string_view text) {
  std::string utfForm(text.length() * 2 + 1, '\0');
  size_t lenU = 0;
  for (char ch : text) {
    const unsigned char uch = ch;
    if (uch < 0x80) {
      utfForm[lenU++] = uch;
    } else {
      utfForm[lenU++] = static_cast<char>(0xC0 | (uch >> 6));
      utfForm[lenU++] = static_cast<char>(0x80 | (uch & 0x3f));
    }
  }
  utfForm.resize(lenU);
  return utfForm;
}

static std::string UTF8FromIconv(const Converter& conv, std::string_view text) {
  if (conv) {
    std::string utfForm(text.length() * 3 + 1, '\0');
    char* pin = const_cast<char*>(text.data());
    uint32_t inLeft = text.length();
    char* putf = &utfForm[0];
    char* pout = putf;
    uint32_t outLeft = text.length() * 3 + 1;
    const uint32_t conversions = conv.Convert(&pin, &inLeft, &pout, &outLeft);
    if (conversions != sizeFailure) {
      *pout = '\0';
      utfForm.resize(pout - putf);
      return utfForm;
    }
  }
  return std::string();
}

// Work out how many bytes are in a character by trying to convert using iconv,
// returning the first length that succeeds.
static size_t MultiByteLenFromIconv(const Converter& conv, const char* s, size_t len) {
  for (size_t lenMB = 1; (lenMB < 4) && (lenMB <= len); lenMB++) {
    char wcForm[2];
    char* pin = const_cast<char*>(s);
    uint32_t inLeft = lenMB;
    char* pout = wcForm;
    uint32_t outLeft = 2;
    const uint32_t conversions = conv.Convert(&pin, &inLeft, &pout, &outLeft);
    if (conversions != sizeFailure) {
      return lenMB;
    }
  }
  return 1;
}

void SurfaceImpl::SetFont(Font& font) {
  vgcanvas_t* vg = this->GetVgCanvas();

  if (vg != NULL) {
    float_t a = 0;
    float_t d = 0;
    float_t lh = 0;
    vgcanvas_set_font(vg, NULL);
    vgcanvas_set_font_size(vg, 20);
    canvas_set_font(this->canvas, NULL, 20);

    canvas_get_text_metrics(this->canvas, &a, &d, &lh);
    this->ascent = tk_abs(a);
    this->descent = tk_abs(d);
    this->line_height = lh;
  }

  return;
}

void SurfaceImpl::DrawTextBase(PRectangle rc, Font& font_, XYPOSITION ybase, std::string_view text,
                               ColourDesired fore) {
  vgcanvas_t* vg = this->GetVgCanvas();
  str_t* str = &(this->str);
  return_if_fail(vg != NULL);

  this->SetFont(font_);
  vgcanvas_set_fill_color(vg, color_from_sci(fore));
  str_set_with_len(str, text.data(), text.length());

  int ox = this->canvas->ox;
  int oy = this->canvas->oy;
  vgcanvas_translate(vg, ox, oy);
  vgcanvas_fill_text(vg, str->str, rc.left, rc.top, 1000);
  vgcanvas_translate(vg, -ox, -oy);
}

void SurfaceImpl::DrawTextNoClip(PRectangle rc, Font& font_, XYPOSITION ybase,
                                 std::string_view text, ColourDesired fore, ColourDesired back) {
  FillRectangle(rc, back);
  DrawTextBase(rc, font_, ybase, text, fore);
}

// On AWTK+, exactly same as DrawTextNoClip
void SurfaceImpl::DrawTextClipped(PRectangle rc, Font& font_, XYPOSITION ybase,
                                  std::string_view text, ColourDesired fore, ColourDesired back) {
  FillRectangle(rc, back);
  DrawTextBase(rc, font_, ybase, text, fore);
}

void SurfaceImpl::DrawTextTransparent(PRectangle rc, Font& font_, XYPOSITION ybase,
                                      std::string_view text, ColourDesired fore) {
  // Avoid drawing spaces in transparent mode
  for (size_t i = 0; i < text.length(); i++) {
    if (text[i] != ' ') {
      DrawTextBase(rc, font_, ybase, text, fore);
      return;
    }
  }
}

void SurfaceImpl::MeasureWidths(Font& font_, std::string_view text, XYPOSITION* positions) {
  uint32_t i = 0;
  float_t x = 0;
  uint32_t k = 0;
  uint32_t j = 0;
  str_t* str = &(this->str);
  wstr_t* wstr = &(this->wstr);
  vgcanvas_t* vg = this->GetVgCanvas();
  return_if_fail(vg != NULL);

  this->SetFont(font_);
  str_set_with_len(str, text.data(), text.length());
  wstr_set_utf8(wstr, str->str);

  for (i = 0; i < wstr->size; i++) {
    wchar_t c = wstr->str[i];
    str_from_wstr_with_len(str, &c, 1);
    x += vgcanvas_measure_text(vg, str->str);
    for(j = 0; j < str->size; j++) {
      positions[k++] = tk_roundi(x);
      assert(k <= text.length());
    }
  }
}

XYPOSITION SurfaceImpl::WidthText(Font& font_, std::string_view text) {
  str_t* str = &(this->str);
  vgcanvas_t* vg = this->GetVgCanvas();
  return_value_if_fail(vg != NULL, 0);

  this->SetFont(font_);
  str_set_with_len(str, text.data(), text.length());

  return vgcanvas_measure_text(vg, str->str);
}

// Ascent and descent determined by Pango font metrics.

XYPOSITION SurfaceImpl::Ascent(Font& font_) {
  int a = this->ascent ? this->ascent : 15;
  log_debug("a=%d\n", a);

  return a;
}

XYPOSITION SurfaceImpl::Descent(Font& font_) {
  int d = this->descent ? this->descent : 5;
  log_debug("d=%d\n", d);
  return d;
}

XYPOSITION SurfaceImpl::InternalLeading(Font&) {
  return 20;
}

XYPOSITION SurfaceImpl::Height(Font& font_) {
  int h = this->line_height ? this->line_height : 20;

  log_debug("h=%d\n", h);

  return h;
}

XYPOSITION SurfaceImpl::AverageCharWidth(Font& font_) {
  return WidthText(font_, "n");
}

void SurfaceImpl::SetClip(PRectangle rc) {
  vgcanvas_t* vg = this->GetVgCanvas();
  return_if_fail(vg != NULL);

  int x = this->canvas->ox + rc.left;
  int y = this->canvas->oy + rc.top;
  int w = rc.right - rc.left;
  int h = rc.bottom - rc.top;

  vgcanvas_clip_rect(vg, x, y, w, h);
}

void SurfaceImpl::FlushCachedState() {
}

void SurfaceImpl::SetUnicodeMode(bool unicodeMode_) {
}

void SurfaceImpl::SetDBCSMode(int codePage) {
}

void SurfaceImpl::SetBidiR2L(bool) {
}

Surface* Surface::Allocate(int) {
  return new SurfaceImpl();
}

Window::~Window() {
}

void Window::Destroy() {
}

PRectangle Window::GetPosition() const {
  PRectangle r;
  widget_t* win = widget_get_window(WIDGET(this->wid));

  r.left = win->x;
  r.top = win->y;
  r.right = win->x + win->w;
  r.bottom = win->y + win->h;

  return r;
}

void Window::SetPosition(PRectangle rc) {
  assert(!"");
}

void Window::SetPositionRelative(PRectangle rc, const Window* relativeTo) {
  assert(!"");
}

PRectangle Window::GetClientPosition() const {
  // On AWTK+, the client position is the window position
  return GetPosition();
}

void Window::Show(bool show) {
  widget_t* win = widget_get_window(WIDGET(this->wid));
  widget_invalidate_force(win, NULL);
}

void Window::InvalidateAll() {
  widget_t* win = widget_get_window(WIDGET(this->wid));
  widget_invalidate_force(win, NULL);
}

void Window::InvalidateRectangle(PRectangle rc) {
  widget_t* win = widget_get_window(WIDGET(this->wid));
  rect_t r = rect_init(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);

  widget_invalidate_force(win, &r);
}

void Window::SetFont(Font&) {
  assert(!"");
  // Can not be done generically but only needed for ListBox
}

void Window::SetCursor(Cursor curs) {
  widget_t* widget = WIDGET(this->wid);

  const char* cursors[] = {
      WIDGET_CURSOR_DEFAULT, /*cursorInvalid*/
      WIDGET_CURSOR_EDIT,    /*cursorText*/
      WIDGET_CURSOR_DEFAULT, /*cursorArrow*/
      WIDGET_CURSOR_DEFAULT, /*cursorUp*/
      WIDGET_CURSOR_WAIT,    /*cursorWait*/
      WIDGET_CURSOR_DEFAULT, /*cursorHoriz*/
      WIDGET_CURSOR_DEFAULT, /*cursorVert*/
      WIDGET_CURSOR_DEFAULT, /*cursorReverseArrow*/
      WIDGET_CURSOR_HAND,    /*cursorHand*/
      WIDGET_CURSOR_EDIT,
  };

  widget_set_cursor(widget, cursors[curs]);
  widget_update_pointer_cursor(widget);
  log_debug("SetCursor:%s\n", cursors[curs]);
}

/* Returns rectangle of monitor pt is on, both rect and pt are in Window's
   gdk window coordinates */
PRectangle Window::GetMonitorRect(Point pt) {
  widget_t* widget = WIDGET(wid);
  PRectangle r;
  r.left = widget->x;
  r.right = widget->x + widget->w;
  r.top = widget->y;
  r.bottom = widget->y + widget->h;

  return r;
}

ListBox::ListBox() noexcept {
}

ListBox::~ListBox() {
}

enum { PIXBUF_COLUMN, TEXT_COLUMN, N_COLUMNS };

class ListBoxX : public ListBox {
 public:
  IListBoxDelegate* delegate;

  ListBoxX() noexcept {
  }
  ~ListBoxX() override {
  }
  void SetFont(Font& font) override;
  void Create(Window& parent, int ctrlID, Point location_, int lineHeight_, bool unicodeMode_,
              int technology_) override;
  void SetAverageCharWidth(int width) override;
  void SetVisibleRows(int rows) override;
  int GetVisibleRows() const override;
  int GetRowHeight();
  PRectangle GetDesiredRect() override;
  int CaretFromEdge() override;
  void Clear() override;
  void Append(char* s, int type = -1) override;
  int Length() override;
  void Select(int n) override;
  int GetSelection() override;
  int Find(const char* prefix) override;
  void GetValue(int n, char* value, int len) override;
  void RegisterRGBA(int type, RGBAImage* image);
  void RegisterImage(int type, const char* xpm_data) override;
  void RegisterRGBAImage(int type, int width, int height,
                         const unsigned char* pixelsImage) override;
  void ClearRegisteredImages() override;
  void SetDelegate(IListBoxDelegate* lbDelegate) override;
  void SetList(const char* listText, char separator, char typesep) override;
};

ListBox* ListBox::Allocate() {
  ListBoxX* lb = new ListBoxX();
  return lb;
}

void ListBoxX::Create(Window& parent, int, Point, int, bool, int) {
}

void ListBoxX::SetFont(Font& font) {
}

void ListBoxX::SetAverageCharWidth(int width) {
}

void ListBoxX::SetVisibleRows(int rows) {
}

int ListBoxX::GetVisibleRows() const {
  return 0;
}

int ListBoxX::GetRowHeight() {
  return 0;
}

PRectangle ListBoxX::GetDesiredRect() {
  PRectangle r;

  return r;
}

int ListBoxX::CaretFromEdge() {
  return 0;
}

void ListBoxX::Clear() {
}

void ListBoxX::Append(char* s, int type) {
}

int ListBoxX::Length() {
  return 0;
}

void ListBoxX::Select(int n) {
}

int ListBoxX::GetSelection() {
  return 0;
}

int ListBoxX::Find(const char* prefix) {
  return 0;
}

void ListBoxX::GetValue(int n, char* value, int len) {
}

// g_return_if_fail causes unnecessary compiler warning in release compile.
#ifdef _MSC_VER
#pragma warning(disable : 4127)
#endif

void ListBoxX::RegisterRGBA(int type, RGBAImage* image) {
}

void ListBoxX::RegisterImage(int type, const char* xpm_data) {
}

void ListBoxX::RegisterRGBAImage(int type, int width, int height,
                                 const unsigned char* pixelsImage) {
}

void ListBoxX::ClearRegisteredImages() {
}

void ListBoxX::SetDelegate(IListBoxDelegate* lbDelegate) {
}

void ListBoxX::SetList(const char* listText, char separator, char typesep) {
}

Menu::Menu() noexcept : mid(nullptr) {
}

void Menu::CreatePopUp() {
}

void Menu::Destroy() {
}

void Menu::Show(Point pt, Window& w) {
}

class DynamicLibraryImpl : public DynamicLibrary {
 protected:
 public:
  explicit DynamicLibraryImpl(const char* modulePath) noexcept {
  }

  ~DynamicLibraryImpl() override {
  }

  // Use g_module_symbol to get a pointer to the relevant function.
  Function FindFunction(const char* name) override {
    return NULL;
  }

  bool IsValid() override {
    return true;
  }
};

DynamicLibrary* DynamicLibrary::Load(const char* modulePath) {
  return static_cast<DynamicLibrary*>(new DynamicLibraryImpl(modulePath));
}

ColourDesired Platform::Chrome() {
  return ColourDesired(0xe0, 0xe0, 0xe0);
}

ColourDesired Platform::ChromeHighlight() {
  return ColourDesired(0xff, 0xff, 0xff);
}

const char* Platform::DefaultFont() {
  return NULL;
}

int Platform::DefaultFontSize() {
  return 20;
}

unsigned int Platform::DoubleClickTime() {
  return 500;  // Half a second
}

void Platform::DebugDisplay(const char* s) {
  fprintf(stderr, "%s", s);
}

//#define TRACE

void Platform::DebugPrintf(const char*, ...) {
}

// Not supported for AWTK+
static bool assertionPopUps = true;

bool Platform::ShowAssertionPopUps(bool assertionPopUps_) {
  const bool ret = assertionPopUps;
  assertionPopUps = assertionPopUps_;
  return ret;
}

void Platform::Assert(const char* c, const char* file, int line) {
  abort();
}

void Platform_Initialise() {
}

void Platform_Finalise() {
}
