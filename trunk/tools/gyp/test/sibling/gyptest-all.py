#!/usr/bin/env python

"""
"""

import sys

import TestGyp

test = TestGyp.TestGyp()

test.run_gyp('build/all.gyp', chdir='src')

test.build_all('build/all.gyp', chdir='src')

chdir = 'src/build'

if sys.platform in ('darwin',):
  chdir = 'src/prog1'
test.run_built_executable('prog1',
                          chdir=chdir,
                          stdout="Hello from prog1.c\n")

if sys.platform in ('darwin',):
  chdir = 'src/prog2'
test.run_built_executable('prog2',
                          chdir=chdir,
                          stdout="Hello from prog2.c\n")

test.pass_test()
