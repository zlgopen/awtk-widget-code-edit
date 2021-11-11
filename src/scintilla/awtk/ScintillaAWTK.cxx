#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cmath>

#include <stdexcept>
#include <new>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>

#include "awtk.h"
#include "Platform.h"
#include "ILoader.h"
#include "ILexer.h"
#include "Scintilla.h"
#include "ScintillaWidget.h"
#include "StringCopy.h"
#include "CharacterCategory.h"
#include "LexerModule.h"
#include "Position.h"
#include "UniqueString.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"
#include "CaseConvert.h"
#include "UniConversion.h"
#include "Selection.h"
#include "PositionCache.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "Editor.h"
#include "AutoComplete.h"
#include "ScintillaBase.h"

#include "ExternalLexer.h"

#include "Converter.h"
#include "ScintillaAWTK.h"

namespace Scintilla {

#define SSM(m, w, l) this->DefWndProc(m, w, l)
ScintillaAWTK::ScintillaAWTK(WindowID wid) {
  this->wMain = wid;
  this->lastKeyDownConsumed = false;
  this->widget = WIDGET(wid);
  this->bar_to_edit = FALSE;
  this->idle_id = TK_INVALID_ID;
  this->lastKeyDownConsumed = TRUE;

  memset(timers, 0x00, sizeof(timers));

  Scintilla_LinkLexers();

  SSM(SCI_SETTABWIDTH, 4, 0);
  SSM(SCI_STYLECLEARALL, 0, 0);
  SSM(SCI_SETCODEPAGE, SC_CP_UTF8, 0);
  SSM(SCI_SETCARETPERIOD, 500, 0);
  SSM(SCI_SETFOCUS, 1, 0);
  SSM(SCI_SETZOOM, 0, 0);
#if 0
  SSM(SCI_SETLEXER, SCLEX_CPP, 0);
  SSM(SCI_STYLESETFORE, SCE_C_COMMENT, 0x008000);
  SSM(SCI_STYLESETFORE, SCE_C_COMMENTLINE, 0x008000);
  SSM(SCI_STYLESETFORE, SCE_C_NUMBER, 0x808000);
  SSM(SCI_STYLESETFORE, SCE_C_WORD, 0x800000);
  SSM(SCI_STYLESETFORE, SCE_C_STRING, 0x800080);
  SSM(SCI_STYLESETBOLD, SCE_C_OPERATOR, 1);
#endif

  SSM(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
  SSM(SCI_SETMARGINWIDTHN, 0, 40);

  SSM(SCI_STYLESETSIZE, 0, 10);
  SSM(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
  SSM(SCI_STYLESETFONT, 0, (sptr_t)("default"));
  SSM(SCI_STYLESETFONT, STYLE_DEFAULT, (sptr_t)("default"));

  return;
}

ScintillaAWTK::~ScintillaAWTK() {
  for (TickReason tr = tickCaret; tr <= tickDwell; tr = static_cast<TickReason>(tr + 1)) {
    FineTickerCancel(tr);
  }
  SetIdle(false);
}

void ScintillaAWTK::OnScrollBarChange(const char* name, int32_t value) {
  if (tk_str_eq(VBAR_NAME, name)) {
    widget_t* bar = widget_lookup(this->widget, VBAR_NAME, TRUE);
    int line_height = this->vs.lineHeight;
    int virtual_h = SCROLL_BAR(bar)->virtual_size;
    int total_lines = virtual_h / line_height + 1;
    int visible_lines = bar->h / line_height + 1;
    float percent = (float)value / (float)(virtual_h);
    int line = (total_lines - visible_lines) * percent;

    line = tk_max(0, line);
    this->bar_to_edit = TRUE;
    this->ScrollTo(line);
    this->bar_to_edit = FALSE;
    log_debug("scroll to: value=%d %f %d\n", value, percent, line);
  }

  return;
}

void ScintillaAWTK::SetVerticalScrollPos() {
  int line_height = this->vs.lineHeight;
  int value = this->topLine * line_height;
  widget_t* bar = widget_lookup(this->widget, VBAR_NAME, TRUE);

  if (bar != NULL && !this->bar_to_edit) {
    int virtual_h = SCROLL_BAR(bar)->virtual_size;
    if (virtual_h == bar->h) {
      value = 0;
    } else {
      value = value * virtual_h / (virtual_h - bar->h);
    }
    scroll_bar_set_value(bar, value);
  }
}

bool ScintillaAWTK::ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) {
  int line_height = this->vs.lineHeight;
  int virtual_height = nMax * line_height;
  widget_t* bar = widget_lookup(this->widget, VBAR_NAME, TRUE);

  if (bar != NULL) {
    scroll_bar_set_params(bar, virtual_height, line_height);
  }

  return true;
}

bool ScintillaAWTK::CanPaste() {
  if (!Editor::CanPaste()) return false;

  return clip_board_get_text() != NULL;
}

void ScintillaAWTK::Paste() {
  const char* text = clip_board_get_text();
  if (text != NULL) {
    this->InsertString(text);
  }
}

void ScintillaAWTK::Copy() {
  SelectionText selectedText;
  CopySelectionRange(&selectedText);
  CopyToClipboard(selectedText);
}

void ScintillaAWTK::CopyToClipboard(const SelectionText& selectedText) {
  const char* textData = selectedText.Data();

  if (textData != NULL) {
    clip_board_set_text(textData);
  }
}

void ScintillaAWTK::SetMouseCapture(bool on) {
  return_if_fail(this->widget != NULL);

  if (on) {
    widget_grab(this->widget->parent, this->widget);
  } else {
    widget_ungrab(this->widget->parent, this->widget);
  }
}

sptr_t ScintillaAWTK::DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
  return ScintillaBase::WndProc(iMessage, wParam, lParam);
}

void ScintillaAWTK::OnPaint(widget_t* widget, canvas_t* c) {
  std::unique_ptr<Surface> surfaceWindow(Surface::Allocate(SC_TECHNOLOGY_DEFAULT));

  surfaceWindow->Init(c, widget);
  rcPaint = this->GetClientRectangle();
  this->Paint(surfaceWindow.get(), rcPaint);

  surfaceWindow->Release();
}

void ScintillaAWTK::Invalidate(void) {
  if (this != NULL) {
    this->InvalidateStyleRedraw();
  }
}

bool ScintillaAWTK::FineTickerRunning(TickReason reason) {
  return timers[reason].timer != TK_INVALID_ID;
}

ret_t ScintillaAWTK::OnTimeout(const timer_info_t* info) {
  TimeThunk* tt = static_cast<TimeThunk*>(info->ctx);

  tt->scintilla->TickFor(tt->reason);

  return RET_REPEAT;
}

void ScintillaAWTK::FineTickerStart(TickReason reason, int millis, int tolerance) {
  this->FineTickerCancel(reason);
  this->timers[reason].scintilla = this;
  this->timers[reason].timer = timer_add(OnTimeout, &timers[reason], millis);
}

void ScintillaAWTK::FineTickerCancel(TickReason reason) {
  if (timers[reason].timer) {
    timer_remove(timers[reason].timer);
    this->timers[reason].timer = TK_INVALID_ID;
  }
}

ret_t ScintillaAWTK::OnIdle(const idle_info_t* info) {
  ScintillaAWTK* sciThis = (ScintillaAWTK*)(info->ctx);

  sciThis->Idle();
  sciThis->idle_id = TK_INVALID_ID;

  return RET_REMOVE;
}

bool ScintillaAWTK::SetIdle(bool on) {
  if (on) {
    this->idle_id = idle_add(OnIdle, this);
  } else {
    if (this->idle_id != TK_INVALID_ID) {
      idle_remove(this->idle_id);
    }
    this->idle_id = TK_INVALID_ID;
  }
  return true;
}

void ScintillaAWTK::AddText(const char* str) {
  this->AddCharUTF(str, strlen(str));
}

void ScintillaAWTK::ScrollAdd(int32_t delta, bool moveThumb) {
  this->ScrollTo(this->topLine + delta, moveThumb);
}

void ScintillaAWTK::SetClient(int32_t x, int32_t y, int32_t w, int32_t h) {
  this->client.left = x;
  this->client.top = y;
  this->client.right = x + w;
  this->client.bottom = y + h;
}

PRectangle ScintillaAWTK::GetClientRectangle() const {
  PRectangle r;

  r.left = 0;
  r.top = 0;
  r.right = this->client.right - this->client.left;
  r.bottom = this->client.bottom - this->client.top;

  widget_t* bar = widget_lookup(this->widget, VBAR_NAME, TRUE);

  if (bar != NULL && bar->visible) {
    r.right -= bar->w;
  }

  return r;
}

ret_t ScintillaAWTK::OnPointerDown(pointer_event_t* e) {
  Point p = Point::FromInts(e->x - this->client.left, e->y - this->client.top);

  this->last_point = p;
  this->ButtonDownWithModifiers(p, e->e.time, 0);

  return RET_OK;
}

ret_t ScintillaAWTK::OnPointerMove(pointer_event_t* e) {
  Point p = Point::FromInts(e->x - this->client.left, e->y - this->client.top);

  if (this->last_point != p && e->pressed) {
    this->ButtonMoveWithModifiers(p, e->e.time, 0);
    this->last_point = p;
  }

  return RET_OK;
}

ret_t ScintillaAWTK::OnPointerUp(pointer_event_t* e) {
  Point p = Point::FromInts(e->x - this->client.left, e->y - this->client.top);

  this->last_point = p;
  this->ButtonUpWithModifiers(p, e->e.time, 0);

  return RET_OK;
}

static int KeyTranslate(int keyIn) noexcept {
  //PLATFORM_ASSERT(!keyIn);
  switch (keyIn) {
    case TK_KEY_DOWN:
      return SCK_DOWN;
    case TK_KEY_UP:
      return SCK_UP;
    case TK_KEY_LEFT:
      return SCK_LEFT;
    case TK_KEY_RIGHT:
      return SCK_RIGHT;
    case TK_KEY_HOME:
      return SCK_HOME;
    case TK_KEY_END:
      return SCK_END;
    case TK_KEY_DELETE:
      return SCK_DELETE;
    case TK_KEY_INSERT:
      return SCK_INSERT;
    case TK_KEY_ESCAPE:
      return SCK_ESCAPE;
    case TK_KEY_BACK:
      return SCK_BACK;
    case TK_KEY_TAB:
      return SCK_TAB;
    case TK_KEY_RETURN:
      return SCK_RETURN;
    case TK_KEY_PLUS:
      return SCK_ADD;
    case TK_KEY_MINUS:
      return SCK_SUBTRACT;
    case TK_KEY_PERCENT:
      return SCK_DIVIDE;
    case TK_KEY_LCOMMAND:
      return SCK_WIN;
    case TK_KEY_RCOMMAND:
      return SCK_RWIN;
    case TK_KEY_MENU:
      return SCK_MENU;
    case TK_KEY_SLASH:
      return '/';
    case TK_KEY_BACKQUOTE:
      return '`';
    case TK_KEY_LEFTBRACKET:
      return '[';
    case TK_KEY_BACKSLASH:
      return '\\';
    case TK_KEY_RIGHTBRACKET:
      return ']';
    default:
      return keyIn;
  }
}

ret_t ScintillaAWTK::OnKeyDown(key_event_t* e) {
  lastKeyDownConsumed = false;
  bool_t ctrl = e->ctrl || e->cmd;
  if (ctrl) {
    if (e->key == TK_KEY_c) {
      SSM(SCI_COPY, 0, 0);
    } else if (e->key == TK_KEY_v) {
      SSM(SCI_PASTE, 0, 0);
    } else if (e->key == TK_KEY_x) {
      SSM(SCI_CUT, 0, 0);
    } else if (e->key == TK_KEY_z) {
      SSM(SCI_UNDO, 0, 0);
    } else if (e->key == TK_KEY_y) {
      SSM(SCI_REDO, 0, 0);
    } else if (e->key == TK_KEY_a) {
      SSM(SCI_SELECTALL, 0, 0);
    } else if (e->key == TK_KEY_PLUS) {
      SSM(SCI_ZOOMIN, 0, 0);
    } else if (e->key == TK_KEY_MINUS) {
      SSM(SCI_ZOOMOUT, 0, 0);
    } else if (e->key == TK_KEY_0) {
      SSM(SCI_SETZOOM, 0, 0);
    }

    return RET_STOP;
  }

  this->KeyDownWithModifiers(KeyTranslate(e->key), ModifierFlags(e->shift, e->ctrl, e->menu),
                             &lastKeyDownConsumed);

  return RET_STOP;
}

ret_t ScintillaAWTK::OnKeyUp(key_event_t* e) {
  return RET_STOP;
}

ret_t ScintillaAWTK::InsertString(const char* str) {
  this->ClearSelection();
  int lengthInserted = pdoc->InsertString(CurrentPosition(), str, strlen(str));

  if (lengthInserted > 0) {
    this->MovePositionTo(CurrentPosition() + lengthInserted);
  }

  return RET_STOP;
}

void ScintillaAWTK::Initialise() {
}
void ScintillaAWTK::CreateCallTipWindow(PRectangle rc) {
}

void ScintillaAWTK::AddToPopUp(const char* label, int cmd, bool enabled) {
}

void ScintillaAWTK::SetHorizontalScrollPos() {
}

void ScintillaAWTK::ClaimSelection() {
}

void ScintillaAWTK::NotifyChange() {
  widget_dispatch_simple_event(this->widget, EVT_VALUE_CHANGED);
}

void ScintillaAWTK::NotifyParent(SCNotification scn) {
}

bool ScintillaAWTK::HaveMouseCapture() {
  return true;
}

}  // namespace Scintilla
