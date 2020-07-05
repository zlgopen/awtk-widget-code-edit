#include "code_edit/code_edit.h"
#include "gtest/gtest.h"

TEST(code_edit, basic) {
  widget_t* w = code_edit_create(NULL, 10, 20, 30, 40);
  code_edit_t* code_edit = CODE_EDIT(w);

  ASSERT_EQ(widget_set_text_utf8(w, "int a = 0"), RET_OK);
  ASSERT_STREQ(widget_get_prop_str(w, WIDGET_PROP_TEXT, NULL), "int a = 0");
  
  ASSERT_EQ(widget_set_prop_str(w, WIDGET_PROP_TEXT, "int a"), RET_OK);
  ASSERT_STREQ(widget_get_prop_str(w, WIDGET_PROP_TEXT, NULL), "int a");

  ASSERT_EQ(code_edit->readonly, FALSE);
  ASSERT_EQ(widget_set_prop_bool(w, WIDGET_PROP_READONLY, TRUE), RET_OK);
  ASSERT_EQ(widget_get_prop_bool(w, WIDGET_PROP_READONLY, FALSE), TRUE);
  ASSERT_EQ(code_edit->readonly, TRUE);
  
  ASSERT_EQ(code_edit->show_line_number, FALSE);
  ASSERT_EQ(widget_set_prop_bool(w, CODE_EDIT_PROP_SHOW_LINE_NUMBER, TRUE), RET_OK);
  ASSERT_EQ(widget_get_prop_bool(w, CODE_EDIT_PROP_SHOW_LINE_NUMBER, FALSE), TRUE);
  ASSERT_EQ(code_edit->show_line_number, TRUE);
  
  ASSERT_EQ(widget_set_prop_str(w, CODE_EDIT_PROP_LANG, "java"), RET_OK);
  ASSERT_STREQ(widget_get_prop_str(w, CODE_EDIT_PROP_LANG, NULL), "java");
  ASSERT_STREQ(code_edit->lang, "java");
  
  ASSERT_EQ(widget_set_prop_str(w, CODE_EDIT_PROP_CODE_THEME, "khaki"), RET_OK);
  ASSERT_STREQ(widget_get_prop_str(w, CODE_EDIT_PROP_CODE_THEME, NULL), "khaki");
  ASSERT_STREQ(code_edit->code_theme, "khaki");
  
  ASSERT_EQ(widget_set_prop_str(w, CODE_EDIT_PROP_FILENAME, "test.c"), RET_OK);
  ASSERT_STREQ(widget_get_prop_str(w, CODE_EDIT_PROP_FILENAME, NULL), "test.c");
  ASSERT_STREQ(code_edit->filename, "test.c");
  
  ASSERT_EQ(code_edit->tab_width, 4);
  ASSERT_EQ(widget_set_prop_int(w, CODE_EDIT_PROP_TAB_WIDTH, 2), RET_OK);
  ASSERT_EQ(widget_get_prop_int(w, CODE_EDIT_PROP_TAB_WIDTH, 0), 2);
  ASSERT_EQ(code_edit->tab_width, 2);

  widget_destroy(w);
}
