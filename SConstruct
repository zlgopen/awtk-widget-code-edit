import os
import sys
import platform
import shutil


def getAwtkRoot():
  awtk_root = '../awtk'
  if not os.path.exists(awtk_root):
    dirnames = ['../awtk', '../../awtk', '../../../awtk']
    for dirname in dirnames:
      if os.path.exists(dirname):
        awtk_root = dirname
        break
  return awtk_root

def isBuildShared():
  return 'WITH_AWTK_SO' in os.environ and os.environ['WITH_AWTK_SO'] == 'true' and BUILD_SHARED == 'true'


def copyAwtkDLL():
  if awtk.OS_NAME == 'Darwin':
    src = os.path.join(AWTK_ROOT, 'bin/libawtk.dylib')
    dst = os.path.join(APP_BIN_DIR, 'libawtk.dylib')
  elif awtk.OS_NAME == 'Linux':
    src = os.path.join(AWTK_ROOT, 'bin/libawtk.so')
    dst = os.path.join(APP_BIN_DIR, 'libawtk.so')
  elif awtk.OS_NAME == 'Windows':
    src = os.path.join(AWTK_ROOT, 'bin/awtk.dll')
    dst = os.path.join(APP_BIN_DIR, 'awtk.dll')
  else:
    print('not support ' + awtk.OS_NAME)
    return

  if not os.path.exists(src):
    print('Can\'t find ' + src + '. Please build AWTK before!')
  else:
    if not os.path.exists(APP_BIN_DIR):
        os.makedirs(APP_BIN_DIR)
    shutil.copy(src, dst)


def genIdlAndDef(awtk_root):
  idl_gen_tools = os.path.join(awtk_root, 'tools/idl_gen/index.js')
  dll_def_gen_tools = os.path.join(awtk_root, 'tools/dll_def_gen/index.js')

  cmd = 'node ' + idl_gen_tools + ' idl/idl.json ' + 'src'
  if os.system(cmd) != 0:
    print('exe cmd: ' + cmd + ' failed.')

  cmd = 'node ' + dll_def_gen_tools + ' idl/idl.json ' + 'src/code_edit.def'
  if os.system(cmd) != 0 :
    print('exe cmd: ' + cmd + ' failed.')


AWTK_ROOT = getAwtkRoot()
sys.path.insert(0, AWTK_ROOT)
import awtk_config as awtk


BUILD_SHARED = 'true'
GEN_IDL_DEF = 'true'
LCD_WIDTH = '1024'
LCD_HEIGHT = '800'
APP_DEFAULT_FONT = 'default'
APP_THEME = 'default'
APP_RES_ROOT = '../res'
APP_DEFAULT_LANGUAGE = 'zh'
APP_DEFAULT_COUNTRY = 'CN'

LCD = ARGUMENTS.get('LCD', '')
if len(LCD) > 0:
  wh = LCD.split('_')
  if len(wh) >= 1:
    LCD_WIDTH = wh[0]
  if len(wh) >= 2:
    LCD_HEIGHT = wh[1]

FONT = ARGUMENTS.get('FONT', '')
if len(FONT) > 0:
  APP_DEFAULT_FONT = FONT

THEME = ARGUMENTS.get('THEME', '')
if len(THEME) > 0:
  APP_THEME = THEME

LANGUAGE = ARGUMENTS.get('LANGUAGE', '')
if len(LANGUAGE) > 0:
  lan = LANGUAGE.split('_')
  if len(lan) >= 1:
    APP_DEFAULT_LANGUAGE = lan[0]
  if len(lan) >= 2:
    APP_DEFAULT_COUNTRY = lan[1]

SHARED = ARGUMENTS.get('SHARED', '')
if len(SHARED) > 0 and SHARED.lower().startswith('f'):
  BUILD_SHARED = 'false'

IDL_DEF = ARGUMENTS.get('IDL_DEF', '')
if len(IDL_DEF) > 0 and IDL_DEF.lower().startswith('f'):
  GEN_IDL_DEF = 'false'

APP_CPPPATH = []
APP_CCFLAGS = ' -DLCD_WIDTH=' + LCD_WIDTH + ' -DLCD_HEIGHT=' + LCD_HEIGHT + ' ' 
APP_CXXFLAGS = '-DSCI_LEXER -DAWTK=1 '
APP_CCFLAGS = APP_CCFLAGS + ' -DAPP_DEFAULT_FONT=\\\"' + APP_DEFAULT_FONT + '\\\" '
APP_CCFLAGS = APP_CCFLAGS + ' -DAPP_THEME=\\\"' + APP_THEME + '\\\" ' 
APP_CCFLAGS = APP_CCFLAGS + ' -DAPP_RES_ROOT=\\\"' + APP_RES_ROOT + '\\\" ' 
APP_CCFLAGS = APP_CCFLAGS + ' -DAPP_DEFAULT_LANGUAGE=\\\"' + APP_DEFAULT_LANGUAGE + '\\\" ' 
APP_CCFLAGS = APP_CCFLAGS + ' -DAPP_DEFAULT_COUNTRY=\\\"' + APP_DEFAULT_COUNTRY + '\\\" ' 

if awtk.OS_NAME == 'Windows':
  APP_CXXFLAGS += ' /std:c++17 '
else:
  APP_CXXFLAGS += ' -std=c++17 '

APP_ROOT    = os.path.normpath(os.getcwd())
APP_BIN_DIR = os.path.join(APP_ROOT, 'bin')
APP_LIB_DIR = os.path.join(APP_ROOT, 'lib')
APP_SRC     = os.path.join(APP_ROOT, 'src')

os.environ['APP_ROOT'] = APP_ROOT;
os.environ['BIN_DIR'] = APP_BIN_DIR;
os.environ['LIB_DIR'] = APP_LIB_DIR;
os.environ['APP_SRC'] = APP_SRC;
os.environ['BUILD_SHARED'] = str(isBuildShared())

print('BUILD_SHARED: ' + str(isBuildShared()))

APP_LINKFLAGS=''
CUSTOM_WIDGET_LIBS = []
APP_LIBS = CUSTOM_WIDGET_LIBS + ['code_edit']
APP_CPPPATH = [
  os.path.join(APP_ROOT, "src"),
  os.path.join(APP_ROOT, "src/scintilla/src"),
  os.path.join(APP_ROOT, "src/scintilla/include"),
  os.path.join(APP_ROOT, "src/scintilla/lexers"),
  os.path.join(APP_ROOT, "src/scintilla/lexlib"),
  os.path.join(APP_ROOT, "src/scintilla/awtk"),
]
APP_TOOLS = None
if hasattr(awtk, 'TOOLS_NAME') and awtk.TOOLS_NAME != '' :
  APP_TOOLS = [awtk.TOOLS_NAME]

AWTK_CCFLAGS = awtk.CCFLAGS
if not isBuildShared() and hasattr(awtk, 'AWTK_CCFLAGS'):
  AWTK_CCFLAGS = awtk.AWTK_CCFLAGS

APP_LIBPATH = [APP_LIB_DIR]
AWTK_LIBS = awtk.LIBS
if isBuildShared():
  APP_LIBPATH = [APP_BIN_DIR, APP_LIB_DIR]
  AWTK_LIBS = awtk.SHARED_LIBS
  copyAwtkDLL();

  if awtk.OS_NAME == 'Linux':
    APP_LINKFLAGS += ' -Wl,-rpath=' + APP_BIN_DIR + ' '

if GEN_IDL_DEF == 'true':
  genIdlAndDef(AWTK_ROOT)

if hasattr(awtk, 'CC'):
  DefaultEnvironment(
    CC=awtk.CC,
    CXX=awtk.CXX,
    LD=awtk.LD,
    AR=awtk.AR,
    STRIP=awtk.STRIP,
    TOOLS     = APP_TOOLS,
    CPPPATH   = awtk.CPPPATH + APP_CPPPATH,
    LINKFLAGS = awtk.LINKFLAGS + APP_LINKFLAGS,
    LIBS      = APP_LIBS + AWTK_LIBS,
    LIBPATH   = APP_LIBPATH + awtk.LIBPATH,
    CCFLAGS   = APP_CCFLAGS + AWTK_CCFLAGS,
    CXXFLAGS   = APP_CXXFLAGS,
    TARGET_ARCH = awtk.TARGET_ARCH,
    OS_SUBSYSTEM_CONSOLE=awtk.OS_SUBSYSTEM_CONSOLE,
    OS_SUBSYSTEM_WINDOWS=awtk.OS_SUBSYSTEM_WINDOWS)
else:
  DefaultEnvironment(
    TOOLS     = APP_TOOLS,
    CPPPATH   = awtk.CPPPATH + APP_CPPPATH,
    LINKFLAGS = awtk.LINKFLAGS + APP_LINKFLAGS,
    LIBS      = APP_LIBS + AWTK_LIBS,
    LIBPATH   = APP_LIBPATH + awtk.LIBPATH,
    CCFLAGS   = APP_CCFLAGS + AWTK_CCFLAGS,
    CXXFLAGS   = APP_CXXFLAGS,
    TARGET_ARCH = awtk.TARGET_ARCH,
    OS_SUBSYSTEM_CONSOLE=awtk.OS_SUBSYSTEM_CONSOLE,
    OS_SUBSYSTEM_WINDOWS=awtk.OS_SUBSYSTEM_WINDOWS)

CustomWidgetSConscriptFiles = []
if awtk.OS_NAME == 'Windows':
  SConscriptFiles = CustomWidgetSConscriptFiles + ['src/SConscript', 'demos/SConscript']
else:
  SConscriptFiles = CustomWidgetSConscriptFiles + ['src/SConscript', 'demos/SConscript', 'tests/SConscript']
SConscript(SConscriptFiles)
