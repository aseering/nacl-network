#!/usr/bin/env python

"""
Verifies file copies using an explicit build target of 'all'.
"""

import os
import sys

import TestGyp

test = TestGyp.TestGyp()

test.run_gyp('copies.gyp', chdir='src')

test.subdir('relocate')
os.rename('src', 'relocate/src')

test.build_all('copies.gyp', chdir='relocate/src')

test.must_match(['relocate', 'src', 'copies-out', 'file1'], "file1 contents\n")

if sys.platform in ('darwin',):
  file2 = ['relocate', 'src', 'build', 'Default', 'copies-out', 'file2']
else:
  file2 = ['relocate', 'src', 'Default', 'copies-out', 'file2']
test.must_match(file2, "file2 contents\n")

test.pass_test()
