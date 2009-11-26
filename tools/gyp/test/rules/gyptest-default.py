#!/usr/bin/env python

"""
Verifies simple rules when using an explicit build target of 'all'.
"""

import os
import sys

import TestGyp

test = TestGyp.TestGyp()

test.run_gyp('actions.gyp', chdir='src')

test.subdir('relocate')
os.rename('src', 'relocate/src')

test.build_default('actions.gyp', chdir='relocate/src')

expect = """\
Hello from program.c
Hello from function1.in
Hello from function2.in
"""

if sys.platform in ('darwin',):
  chdir = 'relocate/src/subdir1'
else:
  chdir = 'relocate/src'
test.run_built_executable('program', chdir=chdir, stdout=expect)

test.must_match('relocate/src/subdir2/file1.out', "Hello from file1.in\n")
test.must_match('relocate/src/subdir2/file2.out', "Hello from file2.in\n")

test.pass_test()
