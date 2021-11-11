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

#include "tkc/fs.h"
#include "tkc/mem.h"
#include "tkc/utils.h"
#include "base/widget_vtable.h"
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

#include "sci_lang_names.h"
#include "base/input_method.h"
#include "code_edit/code_theme.h"
#include "scintilla/awtk/ScintillaAWTK.h"

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

static ret_t code_edit_apply_lang_theme(widget_t* widget) {
  ScintillaAWTK* impl = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  if (code_edit->lang == NULL) {
    code_edit->lang = tk_str_copy(code_edit->lang, "NULL");
    return_value_if_fail(code_edit->lang != NULL, RET_OOM);
  }

  if (code_edit->code_theme != NULL) {
    const asset_info_t* info = NULL;
    int lexer = sci_lang_value(code_edit->lang);
    assets_manager_t* am = widget_get_assets_manager(widget);

    info = assets_manager_ref(am, ASSET_TYPE_XML, code_edit->code_theme);
    if (info != NULL) {
      code_theme_t theme;
      code_theme_init(&theme, code_edit_on_word_style, code_edit_on_widget_style, widget,
                      code_edit->lang);

      SSM(SCI_STYLECLEARALL, 0, 0);
      code_theme_load(&theme, (const char*)(info->data), info->size);
      code_theme_deinit(&theme);
      assets_manager_unref(am, info);
      SSM(SCI_SETZOOM, code_edit->zoom, 0);
    }

    return_value_if_fail(lexer >= 0, RET_BAD_PARAMS);
    SSM(SCI_SETLEXER, lexer, 0);
  }

  return RET_OK;
}

ret_t code_edit_set_lang(widget_t* widget, const char* lang) {
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);

  code_edit->lang = tk_str_copy(code_edit->lang, lang);

  return code_edit_apply_lang_theme(widget);
}

ret_t code_edit_set_code_theme(widget_t* widget, const char* code_theme) {
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);

  code_edit->code_theme = tk_str_copy(code_edit->code_theme, code_theme);

  return code_edit_apply_lang_theme(widget);
}

ret_t code_edit_set_filename(widget_t* widget, const char* filename) {
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);

  if (!tk_str_eq(code_edit->filename, filename)) {
    code_edit->filename = tk_str_copy(code_edit->filename, filename);
    code_edit_load(widget, NULL);
  }

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
  SSM(SCI_SETMARGINWIDTHN, 0, show_line_number ? 40 : 0);

  return RET_OK;
}

ret_t code_edit_set_readonly(widget_t* widget, bool_t readonly) {
  ScintillaAWTK* impl = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  code_edit->readonly = readonly;

  SSM(SCI_SETREADONLY, readonly, 0);

  return RET_OK;
}

ret_t code_edit_set_tab_width(widget_t* widget, uint32_t tab_width) {
  ScintillaAWTK* impl = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  code_edit->tab_width = tab_width;

  SSM(SCI_SETTABWIDTH, tab_width, 0);

  return RET_OK;
}

ret_t code_edit_set_zoom(widget_t* widget, int32_t zoom) {
  ScintillaAWTK* impl = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  code_edit->zoom = zoom;

  SSM(SCI_SETZOOM, zoom, 0);

  return RET_OK;
}

ret_t code_edit_set_wrap_word(widget_t* widget, bool_t wrap_word) {
  ScintillaAWTK* impl = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  code_edit->wrap_word = wrap_word;

  SSM(SCI_SETWRAPMODE, wrap_word, 0);

  return RET_OK;
}

ret_t code_edit_set_scroll_line(widget_t* widget, int32_t scroll_line) {
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  code_edit->scroll_line = scroll_line;
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
  } else if (tk_str_eq(CODE_EDIT_PROP_ZOOM, name)) {
    value_set_int32(v, code_edit->zoom);
    return RET_OK;
  } else if (tk_str_eq(CODE_EDIT_PROP_WRAP_WORD, name)) {
    value_set_bool(v, code_edit->wrap_word);
    return RET_OK;
  } else if (tk_str_eq(CODE_EDIT_PROP_SCROLL_LINE, name)) {
    value_set_int32(v, code_edit->scroll_line);
    return RET_OK;
  } else if (tk_str_eq(CODE_EDIT_PROP_TAB_WIDTH, name)) {
    value_set_uint32(v, code_edit->tab_width);
    return RET_OK;
  } else if (tk_str_eq(WIDGET_PROP_READONLY, name)) {
    value_set_bool(v, code_edit->readonly);
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
  } else if (tk_str_eq(CODE_EDIT_PROP_ZOOM, name)) {
    code_edit_set_zoom(widget, value_int32(v));
    return RET_OK;
  } else if (tk_str_eq(CODE_EDIT_PROP_WRAP_WORD, name)) {
    code_edit_set_wrap_word(widget, value_bool(v));
    return RET_OK;
  } else if (tk_str_eq(CODE_EDIT_PROP_SCROLL_LINE, name)) {
    code_edit_set_scroll_line(widget, value_int32(v));
    return RET_OK;
  } else if (tk_str_eq(CODE_EDIT_PROP_TAB_WIDTH, name)) {
    code_edit_set_tab_width(widget, value_uint32(v));
    return RET_OK;
  } else if (tk_str_eq(WIDGET_PROP_READONLY, name)) {
    code_edit_set_readonly(widget, value_bool(v));
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

static ret_t code_edit_on_paint_self_impl(widget_t* widget, canvas_t* c) {
  code_edit_t* code_edit = CODE_EDIT(widget);
  ScintillaAWTK* impl = static_cast<ScintillaAWTK*>(code_edit->impl);
  if (impl != NULL) {
    impl->OnPaint(widget, c);
  }
  return RET_OK;
}

ret_t code_edit_on_paint_self(widget_t* widget, canvas_t* c) {
  return widget_paint_with_clip(widget, NULL, c, code_edit_on_paint_self_impl);
}

ret_t code_edit_on_event(widget_t* widget, event_t* e) {
  ret_t ret = RET_OK;
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
      value_change_event_t evt;

      input_method_request(input_method(), NULL);
      value_set_wstr(&(evt.old_value), NULL);
      value_set_wstr(&(evt.new_value), widget->text.str);
      widget_dispatch(widget, (event_t*)&evt);
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
    case EVT_WHEEL: {
      wheel_event_t* evt = (wheel_event_t*)e;
      int32_t delta = evt->dy;
      if (delta > 0) {
        impl->ScrollAdd(-code_edit->scroll_line, TRUE);
      } else if (delta < 0) {
        impl->ScrollAdd(code_edit->scroll_line, TRUE);
      }
    }
    case EVT_WIDGET_LOAD:
    case EVT_MOVE_RESIZE: {
      point_t p = {0, 0};
      widget_to_screen(widget, &p);
      impl->SetClient(p.x, p.y, widget->w, widget->h);
      impl->Invalidate();
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
  code_edit_set_tab_width(widget, 4);

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
  SSM(SCI_GETTEXT, len + 1, (sptr_t)(str->str));
  value_set_str(v, str->str);

  return RET_OK;
}

static ret_t code_edit_set_text(widget_t* widget, const value_t* v) {
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

static const uint8_t s_utf8_bom[3] = {0xEF, 0xBB, 0xBF};

ret_t code_edit_save(widget_t* widget, const char* filename, bool_t with_utf8_bom) {
  value_t v;
  int32_t size = 0;
  fs_file_t* fp = NULL;
  const char* str = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  filename = filename != NULL ? filename : code_edit->filename;
  return_value_if_fail(filename != NULL, RET_BAD_PARAMS);
  return_value_if_fail(code_edit_get_text(widget, &v) == RET_OK, RET_BAD_PARAMS);

  str = value_str(&v);
  return_value_if_fail(str != NULL, RET_BAD_PARAMS);

  fp = fs_open_file(os_fs(), filename, "w+");
  return_value_if_fail(fp != NULL, RET_FAIL);

  if (with_utf8_bom) {
    ENSURE(fs_file_write(fp, s_utf8_bom, sizeof(s_utf8_bom)) == sizeof(s_utf8_bom));
  }
  size = strlen(str);
  ENSURE(fs_file_write(fp, str, size) == size);
  fs_file_close(fp);

  return RET_OK;
}

ret_t code_edit_load(widget_t* widget, const char* filename) {
  value_t v;
  uint32_t size = 0;
  const char* str = NULL;
  const char* data = NULL;
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, RET_BAD_PARAMS);
  filename = filename != NULL ? filename : code_edit->filename;
  return_value_if_fail(filename != NULL, RET_BAD_PARAMS);

  data = (const char*)file_read(filename, &size);
  return_value_if_fail(data != NULL, RET_BAD_PARAMS);

  if (memcmp(data, s_utf8_bom, sizeof(s_utf8_bom)) == 0) {
    str = data + 3;
  } else {
    str = data;
  }

  value_set_str(&v, str);
  if (code_edit_set_text(widget, &v) == RET_OK) {
    const char* lang = strrchr(filename, '.');
    if (lang != NULL) {
      lang++;
      code_edit_set_lang(widget, lang);
    }
  }

  TKMEM_FREE(data);

  return RET_OK;
}

bool_t code_edit_is_modified(widget_t* widget) {
  return code_edit_cmd_bool_void(widget, SCI_GETMODIFY);
}
