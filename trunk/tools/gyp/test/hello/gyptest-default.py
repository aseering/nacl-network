#!/usr/bin/env python

"""
Verifies simplest-possible build of a "Hello, world!" program
using the default build target.
"""

import TestGyp

test = TestGyp.TestGyp()

test.run_gyp('hello.gyp')

test.build_default('hello.gyp')

test.run_built_executable('hello', stdout="Hello, world!\n")

test.pass_test()
