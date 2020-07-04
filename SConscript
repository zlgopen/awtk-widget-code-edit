import os
import sys
import platform

OS_NAME = platform.system();
LIB_DIR=os.environ['LIB_DIR'];
BIN_DIR=os.environ['BIN_DIR'];

env=DefaultEnvironment().Clone()

SOURCES  = Glob('code_edit/*.cpp')+Glob('*.c')
SOURCES += Glob('scintilla/lexlib/*.cxx') + \
  Glob('scintilla/lexers/*.cxx') + Glob('scintilla/src/*.cxx') + \
  Glob('scintilla/awtk/*.cxx');

EXPORT_DEF=''
if OS_NAME == 'Windows':
  EXPORT_DEF = ' /DEF:"src/code_edit.def" '

if os.environ['BUILD_SHARED'] == 'True':
  LIBS=['awtk'];
  LINKFLAGS=env['LINKFLAGS'] + EXPORT_DEF 
  env.SharedLibrary(os.path.join(BIN_DIR, 'code_edit'), SOURCES, LIBS=LIBS, LINKFLAGS=LINKFLAGS);
else:
  env.Library(os.path.join(LIB_DIR, 'code_edit'), SOURCES);

