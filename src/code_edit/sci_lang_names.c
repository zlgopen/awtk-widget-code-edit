#include "tkc/utils.h"
#include "tkc/int_str.h"
#include "SciLexer.h"

static const int_str_t s_lang_values[] = {{SCLEX_CONTAINER, "CONTAINER"},
                                          {SCLEX_NULL, "NULL"},
                                          {SCLEX_PYTHON, "PYTHON"},
                                          {SCLEX_CPP, "C"},
                                          {SCLEX_CPP, "CPP"},
                                          {SCLEX_HTML, "HTML"},
                                          {SCLEX_XML, "XML"},
                                          {SCLEX_PERL, "PERL"},
                                          {SCLEX_SQL, "SQL"},
                                          {SCLEX_VB, "VB"},
                                          {SCLEX_PROPERTIES, "PROPERTIES"},
                                          {SCLEX_ERRORLIST, "ERRORLIST"},
                                          {SCLEX_MAKEFILE, "MAKEFILE"},
                                          {SCLEX_BATCH, "BATCH"},
                                          {SCLEX_XCODE, "XCODE"},
                                          {SCLEX_LATEX, "LATEX"},
                                          {SCLEX_LUA, "LUA"},
                                          {SCLEX_DIFF, "DIFF"},
                                          {SCLEX_CONF, "CONF"},
                                          {SCLEX_PASCAL, "PASCAL"},
                                          {SCLEX_AVE, "AVE"},
                                          {SCLEX_ADA, "ADA"},
                                          {SCLEX_LISP, "LISP"},
                                          {SCLEX_RUBY, "RUBY"},
                                          {SCLEX_EIFFEL, "EIFFEL"},
                                          {SCLEX_EIFFELKW, "EIFFELKW"},
                                          {SCLEX_TCL, "TCL"},
                                          {SCLEX_NNCRONTAB, "NNCRONTAB"},
                                          {SCLEX_BULLANT, "BULLANT"},
                                          {SCLEX_VBSCRIPT, "VBSCRIPT"},
                                          {SCLEX_BAAN, "BAAN"},
                                          {SCLEX_MATLAB, "MATLAB"},
                                          {SCLEX_SCRIPTOL, "SCRIPTOL"},
                                          {SCLEX_ASM, "ASM"},
                                          {SCLEX_CPPNOCASE, "CPPNOCASE"},
                                          {SCLEX_FORTRAN, "FORTRAN"},
                                          {SCLEX_F77, "F77"},
                                          {SCLEX_CSS, "CSS"},
                                          {SCLEX_POV, "POV"},
                                          {SCLEX_LOUT, "LOUT"},
                                          {SCLEX_ESCRIPT, "ESCRIPT"},
                                          {SCLEX_PS, "PS"},
                                          {SCLEX_NSIS, "NSIS"},
                                          {SCLEX_MMIXAL, "MMIXAL"},
                                          {SCLEX_CLW, "CLW"},
                                          {SCLEX_CLWNOCASE, "CLWNOCASE"},
                                          {SCLEX_LOT, "LOT"},
                                          {SCLEX_YAML, "YAML"},
                                          {SCLEX_TEX, "TEX"},
                                          {SCLEX_METAPOST, "METAPOST"},
                                          {SCLEX_POWERBASIC, "POWERBASIC"}};

int sci_lang_value(const char* lang) {
  char ulang[TK_NAME_LEN + 1];
  return_value_if_fail(lang != NULL, -1);

  memset(ulang, 0x00, sizeof(ulang));
  tk_strncpy(ulang, lang, TK_NAME_LEN);

  return int_str_name(s_lang_values, tk_str_toupper(ulang), -1);
}
