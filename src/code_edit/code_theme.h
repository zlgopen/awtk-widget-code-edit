/**
 * File:   code_theme.h
 * Author: AWTK Develop Team
 * Brief:  load code theme
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

#ifndef TK_CODE_THEME_H
#define TK_CODE_THEME_H

#include "tkc/color.h"
#include "tkc/types_def.h"

typedef struct _code_style_t {
  int id;
  uint32_t fg;
  uint32_t bg;
  bool_t bold;
  bool_t italic;
  uint32_t font_size;
  const char* font_name;
  const char* name;
  
  uint32_t has_bg : 1;
  uint32_t has_fg : 1;
  uint32_t has_bold : 1;
  uint32_t has_italic : 1;
  uint32_t has_font_size : 1;
  uint32_t has_font_name : 1;
} code_style_t;

typedef ret_t (*code_theme_on_word_style_t)(void* ctx, code_style_t* style);
typedef ret_t (*code_theme_on_widget_style_t)(void* ctx, code_style_t* style);

BEGIN_C_DECLS

typedef struct _code_theme_t {
  code_theme_on_word_style_t on_word_style;
  code_theme_on_widget_style_t on_widget_style;
  void* on_style_ctx;
  const char* lang;
  void* parser;
} code_theme_t;

ret_t code_theme_init(code_theme_t* theme, code_theme_on_word_style_t on_word_style,
                      code_theme_on_widget_style_t on_widget_style, void* ctx, const char* lang);
ret_t code_theme_load(code_theme_t* theme, const char* data, uint32_t size);
ret_t code_theme_deinit(code_theme_t* theme);

END_C_DECLS

#endif /*TK_CODE_THEME_H*/
