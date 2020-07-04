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
#include "code_edit_register.h"
#include "base/widget_factory.h"
#include "code_edit/code_edit.h"

ret_t code_edit_register(void) {
  return widget_factory_register(widget_factory(), WIDGET_TYPE_CODE_EDIT, code_edit_create);
}
