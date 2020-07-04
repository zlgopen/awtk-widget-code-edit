#include "code_edit/code_edit.h"
#include "gtest/gtest.h"

TEST(code_edit, basic) {
  widget_t* w = code_edit_create(NULL, 10, 20, 30, 40);

  widget_destroy(w);
}
