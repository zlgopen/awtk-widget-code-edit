/**
 * File:   code_edit.c
 * Author: AWTK Develop Team
 * Brief:  代码编辑器控件。
 *
 * Copyright (c) 2020 - 2020 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2020-07-04 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "code_edit.h"

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
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include "Platform.h"

#include "ILoader.h"
#include "ILexer.h"
#include "Scintilla.h"
#include "ScintillaWidget.h"
#include "SciLexer.h"
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

#include "Converter.h"
#include "ExternalLexer.h"

#include "base/input_method.h"
#include "scintilla/awtk/ScintillaAWTK.h"
#include "sci_lang_names.h"
#include "code_edit/code_theme.h"

using Scintilla::ScintillaAWTK;
using Scintilla::Surface;

#define SSM(m, w, l) impl->DefWndProc(m, w, l)
static ret_t code_edit_get_text(widget_t* widget, value_t* v);
static ret_t code_edit_set_text(widget_t* widget, const value_t* v);

static ret_t code_edit_on_word_style(void* ctx, code_style_t* style) {
  ScintillaAWTK* impl = NULL;
  code_edit_t* code_edit = CODE_EDIT(ctx);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  if (style->has_font_name) {
    SSM(SCI_STYLESETFONT, style->id, (sptr_t)(style->font_name));
  }

  if (style->has_font_size) {
    SSM(SCI_STYLESETSIZE, style->id, style->font_size);
  }

  if (style->has_bg) {
    SSM(SCI_STYLESETBACK, style->id, style->bg);
  }

  if (style->has_fg) {
    SSM(SCI_STYLESETFORE, style->id, style->fg);
  }

  if (style->has_bold) {
    SSM(SCI_STYLESETBOLD, style->id, style->bold);
  }

  if (style->has_italic) {
    SSM(SCI_STYLESETITALIC, style->id, style->italic);
  }

  return RET_OK;
}

static ret_t code_edit_on_widget_style(void* ctx, code_style_t* style) {
  if (style->id == STYLE_DEFAULT) {
    code_edit_on_word_style(ctx, style);
    style->id = 0;
    code_edit_on_word_style(ctx, style);
  } else if (style->id != 0) {
    code_edit_on_word_style(ctx, style);
  }
  return RET_OK;
}

static ret_t code_edit_apply_lang(widget_t* widget) {
  ScintillaAWTK* impl = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  if (code_edit->lang != NULL && code_edit->code_theme != NULL) {
    const asset_info_t* info = NULL;
    assets_manager_t* am = widget_get_assets_manager(widget);

    int v = sci_lang_value(code_edit->lang);
    return_value_if_fail(v >= 0, RET_BAD_PARAMS);
    SSM(SCI_SETLEXER, v, 0);

    info = assets_manager_ref(am, ASSET_TYPE_XML, code_edit->code_theme);
    if (info != NULL) {
      code_theme_t theme;
      code_theme_init(&theme, code_edit_on_word_style, code_edit_on_widget_style, widget,
                      code_edit->lang);
      code_theme_load(&theme, (const char*)(info->data), info->size);
      code_theme_deinit(&theme);
      assets_manager_unref(am, info);
    }
  }

  return RET_OK;
}

ret_t code_edit_set_lang(widget_t* widget, const char* lang) {
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);

  code_edit->lang = tk_str_copy(code_edit->lang, lang);

  return code_edit_apply_lang(widget);
}

ret_t code_edit_set_code_theme(widget_t* widget, const char* code_theme) {
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);

  code_edit->code_theme = tk_str_copy(code_edit->code_theme, code_theme);

  return code_edit_apply_lang(widget);
}

ret_t code_edit_set_filename(widget_t* widget, const char* filename) {
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);

  code_edit->filename = tk_str_copy(code_edit->filename, filename);

  return RET_OK;
}

ret_t code_edit_set_show_line_number(widget_t* widget, bool_t show_line_number) {
  ScintillaAWTK* impl = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  code_edit->show_line_number = show_line_number;

  SSM(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
  SSM(SCI_SETMARGINWIDTHN, 0, show_line_number ? 40 : 10);

  return RET_OK;
}

ret_t code_edit_get_prop(widget_t* widget, const char* name, value_t* v) {
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(CODE_EDIT_PROP_LANG, name)) {
    value_set_str(v, code_edit->lang);
    return RET_OK;
  } else if (tk_str_eq(WIDGET_PROP_TEXT, name) || tk_str_eq(WIDGET_PROP_VALUE, name)) {
    code_edit_get_text(widget, v);
    return RET_OK;
  } else if (tk_str_eq(CODE_EDIT_PROP_CODE_THEME, name)) {
    value_set_str(v, code_edit->code_theme);
    return RET_OK;
  } else if (tk_str_eq(CODE_EDIT_PROP_FILENAME, name)) {
    value_set_str(v, code_edit->filename);
    return RET_OK;
  } else if (tk_str_eq(CODE_EDIT_PROP_SHOW_LINE_NUMBER, name)) {
    value_set_bool(v, code_edit->show_line_number);
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

ret_t code_edit_set_prop(widget_t* widget, const char* name, const value_t* v) {
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(CODE_EDIT_PROP_LANG, name)) {
    code_edit_set_lang(widget, value_str(v));
    return RET_OK;
  } else if (tk_str_eq(WIDGET_PROP_TEXT, name) || tk_str_eq(WIDGET_PROP_VALUE, name)) {
    code_edit_set_text(widget, v);
    return RET_OK;
  } else if (tk_str_eq(CODE_EDIT_PROP_CODE_THEME, name)) {
    code_edit_set_code_theme(widget, value_str(v));
    return RET_OK;
  } else if (tk_str_eq(CODE_EDIT_PROP_FILENAME, name)) {
    code_edit_set_filename(widget, value_str(v));
    return RET_OK;
  } else if (tk_str_eq(CODE_EDIT_PROP_SHOW_LINE_NUMBER, name)) {
    code_edit_set_show_line_number(widget, value_bool(v));
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

ret_t code_edit_on_destroy(widget_t* widget) {
  code_edit_t* code_edit = CODE_EDIT(widget);
  ScintillaAWTK* impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(widget != NULL && code_edit != NULL, RET_BAD_PARAMS);

  TKMEM_FREE(code_edit->lang);
  TKMEM_FREE(code_edit->code_theme);
  TKMEM_FREE(code_edit->filename);
  str_reset(&(code_edit->text));

  delete impl;

  code_edit->impl = NULL;

  return RET_OK;
}

ret_t code_edit_on_paint_self(widget_t* widget, canvas_t* c) {
  point_t p = {0, 0};
  rect_t save_r = {0, 0, 0, 0};
  rect_t clip_r = {0, 0, 0, 0};
  rect_t client_r = {0, 0, 0, 0};
  code_edit_t* code_edit = CODE_EDIT(widget);
  ScintillaAWTK* impl = static_cast<ScintillaAWTK*>(code_edit->impl);

  if (impl != NULL) {
    canvas_get_clip_rect(c, &save_r);
    widget_to_screen(widget, &p);

    client_r = rect_init(p.x, p.y, widget->w, widget->h);
    clip_r = rect_intersect(&save_r, &client_r);
    canvas_set_clip_rect(c, &clip_r);

    impl->SetClient(c->ox, c->oy, widget->w, widget->h);
    impl->OnPaint(widget, c);

    canvas_set_clip_rect(c, &save_r);
  }

  return RET_OK;
}

ret_t code_edit_on_event(widget_t* widget, event_t* e) {
  ret_t ret = RET_STOP;
  code_edit_t* code_edit = CODE_EDIT(widget);
  ScintillaAWTK* impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  if (widget->target != NULL) {
    return RET_OK;
  }

  switch (e->type) {
    case EVT_POINTER_DOWN: {
      ret = impl->OnPointerDown((pointer_event_t*)e);
      widget_invalidate(widget, NULL);
      break;
    }
    case EVT_POINTER_MOVE: {
      ret = impl->OnPointerMove((pointer_event_t*)e);
      widget_invalidate(widget, NULL);
      break;
    }
    case EVT_POINTER_UP: {
      ret = impl->OnPointerUp((pointer_event_t*)e);
      widget_invalidate(widget, NULL);
      break;
    }
    case EVT_KEY_DOWN: {
      ret = impl->OnKeyDown((key_event_t*)e);
      widget_invalidate(widget, NULL);
      break;
    }
    case EVT_KEY_UP: {
      ret = impl->OnKeyUp((key_event_t*)e);
      widget_invalidate(widget, NULL);
      break;
    }
    case EVT_BLUR: {
      input_method_request(input_method(), NULL);
      widget_dispatch_simple_event(widget, EVT_VALUE_CHANGED);
      break;
    }
    case EVT_FOCUS: {
      input_method_request(input_method(), widget);
      break;
    }
    case EVT_IM_COMMIT: {
      im_commit_event_t* evt = (im_commit_event_t*)e;
      ret = impl->InsertString(evt->text);
      widget_invalidate(widget, NULL);
      break;
    }
    default:
      break;
  }

  return ret;
}

static ret_t code_edit_on_scroll_bar_changed(void* ctx, event_t* e) {
  widget_t* widget = WIDGET(ctx);
  widget_t* target = WIDGET(e->target);
  code_edit_t* code_edit = CODE_EDIT(widget);
  ScintillaAWTK* impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  impl->OnScrollBarChange(target->name, widget_get_value(target));

  return RET_OK;
}

ret_t code_edit_on_add_child(widget_t* widget, widget_t* child) {
  const char* type = widget_get_type(child);

  if (tk_str_eq(type, WIDGET_TYPE_SCROLL_BAR_MOBILE) ||
      tk_str_eq(type, WIDGET_TYPE_SCROLL_BAR_DESKTOP)) {
    widget_on(child, EVT_VALUE_CHANGING, code_edit_on_scroll_bar_changed, widget);
    widget_on(child, EVT_VALUE_CHANGED, code_edit_on_scroll_bar_changed, widget);
  }

  return RET_FAIL;
}

extern "C" widget_t* code_edit_create_internal(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h);

widget_t* code_edit_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h) {
  widget_t* widget = code_edit_create_internal(parent, x, y, w, h);
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, NULL);
  code_edit->impl = new (std::nothrow) ScintillaAWTK(widget);

  return widget;
}

static ret_t code_edit_cmd_void_void(widget_t* widget, int cmd) {
  ScintillaAWTK* impl = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  SSM(cmd, 0, 0);

  return RET_OK;
}

static bool_t code_edit_cmd_bool_void(widget_t* widget, int cmd) {
  ScintillaAWTK* impl = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  return SSM(cmd, 0, 0);
}

static ret_t code_edit_get_text(widget_t* widget, value_t* v) {
  uint32_t len = 0;
  str_t* str = NULL;
  ScintillaAWTK* impl = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  value_set_str(v, NULL);
  len = SSM(SCI_GETTEXTLENGTH, 0, 0);
  return_value_if_fail(len >= 0, RET_FAIL);
  str = &(code_edit->text);
  return_value_if_fail(str_extend(str, len + 1) == RET_OK, RET_FAIL);

  str_set(str, "");
  SSM(SCI_GETTEXT, len, (sptr_t)(str->str));

  return RET_OK;
}

static ret_t code_edit_set_text(widget_t* widget, const value_t* v) {
  uint32_t len = 0;
  str_t* str = NULL;
  ScintillaAWTK* impl = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  str = &(code_edit->text);
  return_value_if_fail(str_from_value(str, v) == RET_OK, RET_FAIL);

  SSM(SCI_SETTEXT, 0, (sptr_t)(str->str));

  return RET_OK;
}


ret_t code_edit_redo(widget_t* widget) {
  return code_edit_cmd_void_void(widget, SCI_REDO);
}

ret_t code_edit_undo(widget_t* widget) {
  return code_edit_cmd_void_void(widget, SCI_UNDO);
}

ret_t code_edit_copy(widget_t* widget) {
  return code_edit_cmd_void_void(widget, SCI_COPY);
}

ret_t code_edit_cut(widget_t* widget) {
  return code_edit_cmd_void_void(widget, SCI_CUT);
}

ret_t code_edit_paste(widget_t* widget) {
  return code_edit_cmd_void_void(widget, SCI_PASTE);
}

ret_t code_edit_clear(widget_t* widget) {
  return code_edit_cmd_void_void(widget, SCI_CLEAR);
}

ret_t code_edit_select_none(widget_t* widget) {
  return code_edit_cmd_void_void(widget, SCI_SELECTNONE);
}

ret_t code_edit_select_all(widget_t* widget) {
  return code_edit_cmd_void_void(widget, SCI_SELECTALL);
}

bool_t code_edit_can_redo(widget_t* widget) {
  return code_edit_cmd_bool_void(widget, SCI_CANREDO);
}

bool_t code_edit_can_undo(widget_t* widget) {
  return code_edit_cmd_bool_void(widget, SCI_CANUNDO);
}

bool_t code_edit_can_copy(widget_t* widget) {
  return code_edit_cmd_bool_void(widget, SCI_CANCOPY);
}

bool_t code_edit_can_cut(widget_t* widget) {
  return code_edit_cmd_bool_void(widget, SCI_CANCUT);
}

bool_t code_edit_can_paste(widget_t* widget) {
  return code_edit_cmd_bool_void(widget, SCI_CANPASTE);
}
