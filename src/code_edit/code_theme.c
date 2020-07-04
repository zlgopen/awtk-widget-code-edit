/**
 * File:   code_theme.h
 * Author: AWTK Develop Team
 * Brief:  code_theme
 *
 * Copyright (c) 2020 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
#include "tkc/str.h"
#include "tkc/utils.h"
#include "base/enums.h"
#include "base/layout.h"
#include "tkc/color_parser.h"
#include "xml/xml_parser.h"
#include "code_edit/code_theme.h"

typedef struct _xml_builder_t {
  XmlBuilder builder;

  code_theme_t* theme;
  str_t str;
  char lang[TK_NAME_LEN + 1];
} xml_builder_t;

static void xml_loader_on_prop_end(XmlBuilder* thiz) {
  xml_builder_t* b = (xml_builder_t*)thiz;
}

static void xml_loader_on_start_lexer_type(XmlBuilder* thiz, const char* tag, const char** attrs) {
  uint32_t i = 0;
  xml_builder_t* b = (xml_builder_t*)thiz;
  while (attrs[i] != NULL) {
    const char* name = attrs[i];
    const char* value = attrs[i + 1];
    if (tk_str_eq(name, "name")) {
      tk_strncpy(b->lang, value, TK_NAME_LEN);
      return;
    }

    i += 2;
  }

  return;
}

static void xml_loader_on_start_on_style(XmlBuilder* thiz, const char* tag, const char** attrs,
                                         bool_t word) {
  uint32_t i = 0;
  uint32_t c = 0;
  code_style_t style;
  xml_builder_t* b = (xml_builder_t*)thiz;

  memset(&style, 0x00, sizeof(style));
  while (attrs[i] != NULL) {
    const char* name = attrs[i];
    const char* value = attrs[i + 1];
    if (value == NULL || *value == '\0') {
      i += 2;
      continue;
    }

    if (tk_str_eq(name, "name")) {
    } else if (tk_str_eq(name, "styleID")) {
      style.id = tk_atoi(value);
    } else if (tk_str_eq(name, "fgColor")) {
      if (sscanf(value, "%06X", &(c)) == 1) {
        style.fg = c;
        style.has_fg = TRUE;
      }
    } else if (tk_str_eq(name, "bgColor")) {
      if (sscanf(value, "%06X", &(c)) == 1) {
        style.bg = c;
        style.has_bg = TRUE;
      }
    } else if (tk_str_eq(name, "fontName")) {
      style.font_name = value;
      style.has_font_name = TRUE;
    } else if (tk_str_eq(name, "fontSize")) {
      style.font_size = tk_atoi(value);
      style.has_font_size = TRUE;
    } else if (tk_str_eq(name, "fontStyle")) {
      uint32_t font_style = tk_atoi(value);
      if (font_style & 0x01) {
        style.bold = TRUE;
        style.has_bold = TRUE;
      }
      /*FIXME*/
    }

    i += 2;
  }

  if (word) {
    b->theme->on_word_style(b->theme->on_style_ctx, &style);
  } else {
    b->theme->on_widget_style(b->theme->on_style_ctx, &style);
  }

  return;
}

static void xml_loader_on_start(XmlBuilder* thiz, const char* tag, const char** attrs) {
  xml_builder_t* b = (xml_builder_t*)thiz;
  if (tk_str_eq(tag, "LexerType")) {
    xml_loader_on_start_lexer_type(thiz, tag, attrs);
  } else if (tk_str_eq(tag, "WordsStyle")) {
    if (tk_str_eq(b->lang, b->theme->lang)) {
      if (b->theme->on_word_style != NULL) {
        xml_loader_on_start_on_style(thiz, tag, attrs, TRUE);
      }
    }
  } else if (tk_str_eq(tag, "WidgetStyle")) {
    if (b->theme->on_widget_style != NULL) {
      xml_loader_on_start_on_style(thiz, tag, attrs, FALSE);
    }
  }

  return;
}

static void xml_loader_on_end(XmlBuilder* thiz, const char* tag) {
  xml_builder_t* b = (xml_builder_t*)thiz;
  return;
}

static void xml_loader_on_text(XmlBuilder* thiz, const char* text, size_t length) {
  return;
}

static void xml_loader_on_comment(XmlBuilder* thiz, const char* text, size_t length) {
  return;
}

static void xml_loader_on_pi(XmlBuilder* thiz, const char* tag, const char** attrs) {
  return;
}

static void xml_loader_on_error(XmlBuilder* thiz, int line, int row, const char* message) {
  (void)thiz;
  log_debug("parse error: %d:%d %s\n", line, row, message);
  return;
}

static void xml_loader_destroy(XmlBuilder* thiz) {
  (void)thiz;
  return;
}

static XmlBuilder* builder_init(xml_builder_t* b, code_theme_t* theme) {
  memset(b, 0x00, sizeof(xml_builder_t));

  b->builder.on_start = xml_loader_on_start;
  b->builder.on_end = xml_loader_on_end;
  b->builder.on_text = xml_loader_on_text;
  b->builder.on_error = xml_loader_on_error;
  b->builder.on_comment = xml_loader_on_comment;
  b->builder.on_pi = xml_loader_on_pi;
  b->builder.destroy = xml_loader_destroy;
  b->theme = theme;
  str_init(&(b->str), 100);

  return &(b->builder);
}

ret_t code_theme_init(code_theme_t* theme, code_theme_on_word_style_t on_word_style,
                      code_theme_on_widget_style_t on_widget_style, void* ctx, const char* lang) {
  return_value_if_fail(theme != NULL, RET_BAD_PARAMS);
  return_value_if_fail(ctx != NULL && lang != NULL, RET_BAD_PARAMS);
  return_value_if_fail(on_word_style != NULL || on_widget_style != NULL, RET_BAD_PARAMS);

  theme->lang = lang;
  theme->on_style_ctx = ctx;
  theme->on_word_style = on_word_style;
  theme->on_widget_style = on_widget_style;

  return RET_OK;
}

ret_t code_theme_load(code_theme_t* theme, const char* data, uint32_t size) {
  xml_builder_t b;
  XmlParser* parser = NULL;
  return_value_if_fail(theme != NULL, RET_BAD_PARAMS);
  return_value_if_fail(data != NULL && size > 0, RET_BAD_PARAMS);

  parser = xml_parser_create();
  return_value_if_fail(parser != NULL, RET_OOM);
  xml_parser_set_builder(parser, builder_init(&b, theme));
  xml_parser_parse(parser, (const char*)data, size);
  xml_parser_destroy(parser);
  str_reset(&(b.str));

  return RET_OK;
}

ret_t code_theme_deinit(code_theme_t* theme) {
  return RET_OK;
}
