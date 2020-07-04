/**
 * File:   code_edit.h
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

#ifndef TK_CODE_EDIT_H
#define TK_CODE_EDIT_H

#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class code_edit_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * 代码编辑器控件。
 */
typedef struct _code_edit_t {
  widget_t widget;

  /**
   * @property {char*} lang
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 当前的代码的语言。
   */
  char* lang;

  /**
   * @property {char*} code_theme
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 当前的代码的主题名称(在资源的data目录必须有相应的xml文件存在，格式与nodepad++的一致)。
   */
  char* code_theme;

  /**
   * @property {char*} filename
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 文件名。
   */
  char* filename;

  /**
   * @property {bool_t} show_line_number
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 是否显示行号。
   */
  bool_t show_line_number;

  /*private*/
  void* impl;
} code_edit_t;

/**
 * @method code_edit_create
 * @annotation ["constructor", "scriptable"]
 * 创建code_edit对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} code_edit对象。
 */
widget_t* code_edit_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method code_edit_cast
 * 转换为code_edit对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget code_edit对象。
 *
 * @return {widget_t*} code_edit对象。
 */
widget_t* code_edit_cast(widget_t* widget);

/**
 * @method code_edit_set_lang
 * 设置 当前的代码的语言。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {const char*} lang 当前的代码的语言。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t code_edit_set_lang(widget_t* widget, const char* lang);

/**
 * @method code_edit_set_code_theme
 * 设置 当前的代码的主题。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {const char*} code_theme 当前的代码的语言。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t code_edit_set_code_theme(widget_t* widget, const char* code_theme);

/**
 * @method code_edit_set_filename
 * 设置 文件名。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {const char*} filename 文件名。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t code_edit_set_filename(widget_t* widget, const char* filename);

/**
 * @method code_edit_set_show_line_number
 * 设置 是否显示行号。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {bool_t} show_line_number 是否显示行号。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t code_edit_set_show_line_number(widget_t* widget, bool_t show_line_number);

/**
 * @method code_edit_redo
 * 重做。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t code_edit_redo(widget_t* widget);

/**
 * @method code_edit_undo
 * 撤销。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t code_edit_undo(widget_t* widget);

/**
 * @method code_edit_copy
 * 拷贝选中的文本。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t code_edit_copy(widget_t* widget);

/**
 * @method code_edit_cut
 * 剪切。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t code_edit_cut(widget_t* widget);

/**
 * @method code_edit_paste
 * 粘贴。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t code_edit_paste(widget_t* widget);

/**
 * @method code_edit_clear
 * 清除内容。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t code_edit_clear(widget_t* widget);

/**
 * @method code_edit_select_none
 * 取消选择。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t code_edit_select_none(widget_t* widget);

/**
 * @method code_edit_select_all
 * 全选。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t code_edit_select_all(widget_t* widget);

/**
 * @method code_edit_can_redo
 * 检查是否可以重做。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {bool_t} 返回TRUE表示能，否则表示不能。
 */
bool_t code_edit_can_redo(widget_t* widget);

/**
 * @method code_edit_can_undo
 * 检查是否可以撤销。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {bool_t} 返回TRUE表示能，否则表示不能。
 */
bool_t code_edit_can_undo(widget_t* widget);

/**
 * @method code_edit_can_copy
 * 检查是否可以拷贝。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {bool_t} 返回TRUE表示能，否则表示不能。
 */
bool_t code_edit_can_copy(widget_t* widget);

/**
 * @method code_edit_can_cut
 * 检查是否可以剪切。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {bool_t} 返回TRUE表示能，否则表示不能。
 */
bool_t code_edit_can_cut(widget_t* widget);

/**
 * @method code_edit_can_paste
 * 检查是否可以粘贴。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {bool_t} 返回TRUE表示能，否则表示不能。
 */
bool_t code_edit_can_paste(widget_t* widget);

#define CODE_EDIT_PROP_LANG "lang"
#define CODE_EDIT_PROP_CODE_THEME "theme"
#define CODE_EDIT_PROP_FILENAME "filename"
#define CODE_EDIT_PROP_SHOW_LINE_NUMBER "show_line_number"

#define WIDGET_TYPE_CODE_EDIT "code_edit"

#define CODE_EDIT(widget) ((code_edit_t*)(code_edit_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(code_edit);

END_C_DECLS

#endif /*TK_CODE_EDIT_H*/
