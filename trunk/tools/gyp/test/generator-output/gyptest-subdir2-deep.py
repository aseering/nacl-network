#!/usr/bin/env python

"""
Verifies building a target from a .gyp file a few subdirectories
deep when the --generator-output= option is used to put the build
configuration files in a separate directory tree.
"""

import sys

import TestGyp

test = TestGyp.TestGyp()

test.writable(test.workpath('src'), False)

test.writable(test.workpath('src/subdir2/deeper/build'), True)

test.run_gyp('deeper.gyp',
             '-Dset_symroot=1',
             '--generator-output=' + test.workpath('gypfiles'),
             chdir='src/subdir2/deeper')

test.build_all('deeper.gyp', chdir='gypfiles')

chdir = 'gypfiles'

if sys.platform in ('darwin',):
  chdir = 'src/subdir2/deeper'
test.run_built_executable('deeper',
                          chdir=chdir,
                          stdout="Hello from deeper.c\n")

test.pass_test()
