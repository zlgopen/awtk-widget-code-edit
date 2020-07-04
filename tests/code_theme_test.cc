#include "code_edit/code_theme.h"
#include "gtest/gtest.h"
#include "tkc/utils.h"
#include <string>
using std::string;

static void check_style(code_style_t* a, code_style_t* b) {
  ASSERT_EQ(memcmp(a, b, sizeof(*a)), 0);
}

static ret_t code_theme_on_style(void* ctx, code_style_t* style) {
  check_style((code_style_t*)ctx, style);
  return RET_OK;
}

TEST(code_theme, basic) {
  code_style_t style;
  code_theme_t theme;
  const char* str = "\
<NotepadPlus>\
    <LexerStyles>\
        <LexerType name=\"c\" desc=\"C\" ext=\"\">\
            <WordsStyle name=\"PREPROCESSOR\" styleID=\"9\" fgColor=\"8996A8\" bgColor=\"141414\" fontName=\"\" fontStyle=\"0\" fontSize=\"10\" />\
        </LexerType>\
    <LexerStyles>\
</NotepadPlus>";

  memset(&style, 0x00, sizeof(style));
  style.id = 9;
  style.fg = 9017000;
  style.bg = 1315860;
  style.font_size = 10;

  style.has_bg = 1;
  style.has_fg = 1;
  style.has_font_size = 1;

  code_theme_init(&theme, code_theme_on_style, code_theme_on_style, &style, "c");
  code_theme_load(&theme, str, strlen(str));
  code_theme_deinit(&theme);
}

