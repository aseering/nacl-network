#!/usr/bin/env python

"""
Verifies build of an executable in three different configurations.
"""

import TestGyp

test = TestGyp.TestGyp()

test.run_gyp('configurations.gyp')

test.set_configuration('Release')
test.run_build('configurations.gyp')
test.run_built_executable('configurations', stdout="Release configuration\n")

test.set_configuration('Debug')
test.run_build('configurations.gyp')
test.run_built_executable('configurations', stdout="Debug configuration\n")

test.set_configuration('Foo')
test.run_build('configurations.gyp')
test.run_built_executable('configurations', stdout="Foo configuration\n")

test.pass_test()
