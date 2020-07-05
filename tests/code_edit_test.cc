#include "code_edit/code_edit.h"
#include "gtest/gtest.h"

TEST(code_edit, basic) {
  widget_t* w = code_edit_create(NULL, 10, 20, 30, 40);
  code_edit_t* code_edit = CODE_EDIT(w);
  ASSERT_EQ(widget_set_text_utf8(w, "int a = 0"), RET_OK);
  ASSERT_STREQ(widget_get_prop_str(w, WIDGET_PROP_TEXT, NULL), "int a = 0");


  ASSERT_EQ(code_edit->readonly, FALSE);
  ASSERT_EQ(widget_set_prop_bool(w, WIDGET_PROP_READONLY, TRUE), RET_OK);
  ASSERT_EQ(widget_get_prop_bool(w, WIDGET_PROP_READONLY, FALSE), TRUE);
  ASSERT_EQ(code_edit->readonly, TRUE);

  widget_destroy(w);
}
