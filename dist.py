#!/usr/bin/env python

from __future__ import with_statement
import re
from os import system
from sys import exit

def run(cmd):
  if system(cmd):
    print("dist.py: exiting...")
    exit(-1)

run('scons release=1')

arch = 'lin64'

pattern = re.compile('VERSION = ([\'"])([^\'"\\s]+)\\1')

with open('SConstruct') as f:
  for line in f:
    match = pattern.search(line)
    if match:
      version = match.group(2)
      break

basename = 'gonstruct-%s-%s' % (version, arch)
filename = basename + '.tar.gz'
# if someone figures out how to do this without risking shell expansion,
# let me know
run('rm -f %s %s' % (filename, basename))
run('ln -s dist %s' % basename)
run('tar cfz %s %s' % (filename, ' '.join(['%s/%s' % (basename, file) for file in ['gonstruct', 'readme.html', 'changelog.txt']])))
run('rm -f %s' % basename)
