""" installer

This module defines a minimal installer for scons build scripts.  It is aimed
at *nix like systems.

Based on http://www.scons.org/wiki/Installer
"""

import fnmatch, os, os.path
import SCons.Defaults

PREFIX  = "prefix"
EPREFIX = "eprefix"
BINDIR  = "bindir"
DATADIR = "datadir"

def AddOptions(opts):
        opts.Add(PREFIX, "Directory of architecture independant files.",
                  "/usr" )
        opts.Add(EPREFIX, "Directory of architecture dependant files.",
                  "${%s}" % PREFIX )
        opts.Add(BINDIR, "Directory of executables.",
                  "${%s}/bin" % EPREFIX )
        opts.Add(DATADIR, "Directory of data files.",
                  "${%s}/share/gonstruct" % PREFIX )


class Installer:
    def __init__(self, env, args):
        self._prefix  = args.get(PREFIX,  "/usr/local")
        self._eprefix = args.get(EPREFIX, self._prefix)
        self._bindir  = args.get(BINDIR,  os.path.join(self._eprefix, "bin"))
        self._datadir = args.get(DATADIR, os.path.join(self._prefix,
                                                       "share", "gonstruct"))
        print(self._prefix)
        print(self._eprefix)
        print(self._bindir)
        print(self._datadir)
        self._env = env

    def GetConfig(self):
        return {
          PREFIX  : self._prefix,
          EPREFIX : self._eprefix,
          BINDIR  : self._bindir,
          DATADIR : self._datadir,
        }

    def Add(self, destdir, name, basedir="", perm=0644):
        destination = os.path.join(destdir, basedir)
        obj = self._env.Install(destination, name)
        self._env.Alias("install", destination)
        for i in obj:
            self._env.AddPostAction(i, SCons.Defaults.Chmod(str(i), perm))

    def AddProgram( self, program ):
        self.Add(self._bindir, program, perm=0755)

    def AddData( self, datafile ):
        self.Add(self._datadir, datafile)
