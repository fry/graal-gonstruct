import installer

svg_builder = Builder(action =
                      'inkscape --without-gui --export-png=$TARGET $SOURCE',
                      suffix     = '.png',
                      src_suffix = '.svg')

env = Environment(BUILDERS = {'Svg' : svg_builder})
env.Append(VERSION = "0.1.5")
opts = Options(ARGUMENTS)
installer.AddOptions(opts)
env.Help(opts.GenerateHelpText(env))
install = installer.Installer(env, ARGUMENTS)

env.ParseConfig('pkg-config --cflags --libs gtkmm-2.4 gtksourceview-2.0')
env.Append(CPPPATH = ['#src'],
           LIBS = ['boost_system-gcc43-mt', 'boost_filesystem-gcc43-mt'])

# no -Wold-style-cast or -Wredundant-decls because of the gtkmm headers
env.Append(CXXFLAGS = '-pedantic -ansi -Wall -Wextra '
                      '-Wfloat-equal -Wwrite-strings -Wpointer-arith '
                      '-Wcast-qual -Wctor-dtor-privacy -Woverloaded-virtual '
                      '-Wno-unused-parameter -Wtrigraphs -Wsign-promo')

if ARGUMENTS.get('release', '0') == '0':
  env.Append(CXXFLAGS = '-g -O0')
else:
  env.Append(CXXFLAGS = '-O3')

SConscript(['src/SConscript'], exports = ['env', 'install'])
