import installer
import os

svg_builder = Builder(action =
                      'inkscape --without-gui --export-png=$TARGET $SOURCE',
                      suffix     = '.png',
                      src_suffix = '.svg')

env = Environment(
  ENV = os.environ,
  BUILDERS = {'Svg' : svg_builder},
  tools = ['g++', 'mingw'])

env.Append(VERSION = "0.1.5")
opts = Options(ARGUMENTS)

installer.AddOptions(opts)
install = installer.Installer(env, ARGUMENTS)

env.Help(opts.GenerateHelpText(env))
env.Help("\nType: 'scons debug=1' to build the debug version.\n")

# no -Wold-style-cast, -Wredundant-decls or -pedantic because of the gtkmm headers
env.Append(CXXFLAGS = '-ansi -Wall -Wextra '
                      '-Wfloat-equal -Wwrite-strings -Wpointer-arith '
                      '-Wcast-qual -Wctor-dtor-privacy -Woverloaded-virtual '
                      '-Wno-unused-parameter -Wtrigraphs -Wsign-promo')

if ARGUMENTS.get('debug', '0') == '0':
  env.Append(CXXFLAGS = '-O3')
  if env['PLATFORM'] == 'win32':
    env.Append(LINKFLAGS = '-mwindows')
else:
  env.Append(CXXFLAGS = '-g -O0')

env.Alias('gonstruct', 'src/level_editor')
SConscript(['src/SConscript'], exports = ['env', 'install'])
