#include "code_edit/code_edit.h"
#include "gtest/gtest.h"

TEST(code_edit, basic) {
  widget_t* w = code_edit_create(NULL, 10, 20, 30, 40);
  ASSERT_EQ(widget_set_text_utf8(w, "int a = 0"), RET_OK);
  ASSERT_STREQ(widget_get_prop_str(w, WIDGET_PROP_TEXT, NULL), "int a = 0");
  widget_destroy(w);
}
