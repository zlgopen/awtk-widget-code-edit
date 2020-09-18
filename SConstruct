import os
import platform
import scripts.awtk_locator as locator

def getAppHelper(ARGUMENTS):
    locator.init(ARGUMENTS)

    from app_helper_base import AppHelperBase
    return AppHelperBase(ARGUMENTS)

helper = getAppHelper(ARGUMENTS)

APP_ROOT = helper.APP_ROOT

APP_CPPPATH = [
  os.path.join(APP_ROOT, "src"),
  os.path.join(APP_ROOT, "src/scintilla/src"),
  os.path.join(APP_ROOT, "src/scintilla/include"),
  os.path.join(APP_ROOT, "src/scintilla/lexers"),
  os.path.join(APP_ROOT, "src/scintilla/lexlib"),
  os.path.join(APP_ROOT, "src/scintilla/awtk"),
]

APP_CXXFLAGS = '-DSCI_LEXER -DAWTK=1 '

if platform.system() == 'Windows':
  APP_CXXFLAGS += ' /std:c++17 '
else:
  APP_CXXFLAGS += ' -std=c++17 '

helper.set_dll_def('src/code_edit.def').set_libs(['code_edit'])
helper.add_cxxflags(APP_CXXFLAGS).add_cpppath(APP_CPPPATH).call(DefaultEnvironment)

SConscriptFiles = ['src/SConscript', 'demos/SConscript', 'tests/SConscript']
SConscript(SConscriptFiles)
