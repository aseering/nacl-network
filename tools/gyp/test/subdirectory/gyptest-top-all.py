#!/usr/bin/env python

"""
Verifies building a target and a subsidiary dependent target from a
.gyp file in a subdirectory, without specifying an explicit output build
directory, and using the generated solution or project file at the top
of the tree as the entry point.

There is a difference here in the default behavior of the underlying
build tools.  Specifically, when building the entire "solution", Xcode
puts the output of each project relative to the .xcodeproj directory,
while Visual Studio (and our implementations of SCons and Make) put it
in a build directory relative to the "solution"--that is, the entry-point
from which you built the entire tree.
"""

import os

import TestGyp

test = TestGyp.TestGyp()

test.run_gyp('prog1.gyp', chdir='src')

test.subdir('relocate')
os.rename('src', 'relocate/src')

test.build_all('prog1.gyp', chdir='relocate/src')

test.run_built_executable('prog1',
                          stdout="Hello from prog1.c\n",
                          chdir='relocate/src')

import sys
if sys.platform in ('darwin',):
  chdir = 'relocate/src/subdir'
else:
  chdir = 'relocate/src'
test.run_built_executable('prog2',
                          chdir=chdir,
                          stdout="Hello from prog2.c\n")

test.pass_test()
