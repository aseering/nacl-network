#!/usr/bin/python

"""
TestGyp.py:  a testing framework for GYP integration tests.
"""

import os
import shutil
import stat
import sys

import TestCommon
from TestCommon import __all__

__all__.extend([
  'TestGyp',
])


class TestGypBase(TestCommon.TestCommon):
  """
  Class for controlling end-to-end tests of gyp generators.

  Instantiating this class will create a temporary directory and
  arrange for its destruction (via the TestCmd superclass) and
  copy all of the non-gyptest files in the directory hierarchy of the
  executing script.

  The default behavior is to test the 'gyp' or 'gyp.bat' file in the
  current directory.  An alternative may be specified explicitly on
  instantiation, or by setting the TESTGYP_GYP environment variable.

  This class should be subclassed for each supported gyp generator
  (format).  Various abstract methods below define calling signatures
  used by the test scripts to invoke builds on the generated build
  configuration and to run executables generated by those builds.
  """

  build_tool = None
  build_tool_list = []

  _exe = TestCommon.exe_suffix
  _obj = TestCommon.obj_suffix
  shobj_ = TestCommon.shobj_prefix
  _shobj = TestCommon.shobj_suffix
  lib_ = TestCommon.lib_prefix
  _lib = TestCommon.lib_suffix
  dll_ = TestCommon.dll_prefix
  _dll = TestCommon.dll_suffix

  def __init__(self, gyp=None, *args, **kw):
    self.origin_cwd = os.path.abspath(os.path.dirname(sys.argv[0]))

    if not gyp:
      gyp = os.environ.get('TESTGYP_GYP')
      if not gyp:
        if sys.platform == 'win32':
          gyp = 'gyp.bat'
        else:
          gyp = 'gyp'
    self.gyp = os.path.abspath(gyp)

    self.initialize_build_tool()

    if not kw.has_key('match'):
      kw['match'] = TestCommon.match_exact

    if not kw.has_key('workdir'):
      # Default behavior:  the null string causes TestCmd to create
      # a temporary directory for us.
      kw['workdir'] = ''

    formats = kw.get('formats')
    if kw.has_key('formats'):
      del kw['formats']

    super(TestGypBase, self).__init__(*args, **kw)

    if formats and self.format not in formats:
      msg = 'Invalid test for %r format; skipping test.\n'
      self.skip_test(msg % self.format)

    self.copy_test_configuration(self.origin_cwd, self.workdir)
    self.set_configuration(None)

  def copy_test_configuration(self, source_dir, dest_dir):
    """
    Copies the test configuration from the specified source_dir
    (the directory in which the test script lives) to the
    specified dest_dir (a temporary working directory).

    This ignores all files and directories that begin with
    the string 'gyptest', and all '.svn' subdirectories.
    """
    for root, dirs, files in os.walk(source_dir):
      if '.svn' in dirs:
        dirs.remove('.svn')
      dirs = [ d for d in dirs if not d.startswith('gyptest') ]
      files = [ f for f in files if not f.startswith('gyptest') ]
      for dirname in dirs:
        source = os.path.join(root, dirname)
        destination = source.replace(source_dir, dest_dir)
        os.mkdir(destination)
        if sys.platform != 'win32':
          shutil.copystat(source, destination)
      for filename in files:
        source = os.path.join(root, filename)
        destination = source.replace(source_dir, dest_dir)
        shutil.copy2(source, destination)

  def initialize_build_tool(self):
    """
    Initializes the .build_tool attribute.

    Searches the .build_tool_list for an executable name on the user's
    $PATH.  The first tool on the list is used as-is if nothing is found
    on the current $PATH.
    """
    for build_tool in self.build_tool_list:
      if not build_tool:
        continue
      if os.path.isabs(build_tool):
        self.build_tool = build_tool
        return
      build_tool = self.where_is(build_tool)
      if build_tool:
        self.build_tool = build_tool
        return

    if self.build_tool_list:
      self.build_tool = self.build_tool_list[0]

  def run_gyp(self, gyp_file, *args, **kw):
    """
    Runs gyp against the specified gyp_file with the specified args.
    """
    # TODO:  --depth=. works around Chromium-specific tree climbing.
    args = ('--depth=.', '--format='+self.format, gyp_file) + args
    return self.run(program=self.gyp, arguments=args, **kw)

  def run(self, *args, **kw):
    """
    Executes a program by calling the superclass .run() method.

    This exists to provide a common place to filter out keyword
    arguments implemented in this layer, without having to update
    the tool-specific subclasses or clutter the tests themselves
    with platform-specific code.
    """
    if kw.has_key('SYMROOT'):
      del kw['SYMROOT']
    super(TestGypBase, self).run(*args, **kw)

  def set_configuration(self, configuration):
    """
    Sets the configuration, to be used for invoking the build
    tool and testing potential built output.
    """
    self.configuration = configuration

  #
  # Abstract methods to be defined by format-specific subclasses.
  #

  def build_all(self, gyp_file):
    """
    Runs an "all" build of the configuration generated from the
    specified gyp_file.
    """
    raise NotImplementeError

  def build_default(self, gyp_file):
    """
    Runs the default build of the configuration generated from the
    specified gyp_file.
    """
    raise NotImplementeError

  def build_target(self, gyp_file, target):
    """
    Runs a build of the specified target against the configuration
    generated from the specified gyp_file.
    """
    raise NotImplementeError

  def run_built_executable(self, name, *args, **kw):
    """
    Runs an executable program built from a gyp-generated configuration.

    The specified name should be independent of any particular generator.
    Subclasses should find the output executable in the appropriate
    output build directory, tack on any necessary executable suffix, etc.
    """
    raise NotImplementeError


class TestGypGypd(TestGypBase):
  """
  Subclass for testing the GYP 'gypd' generator (spit out the
  internal data structure as pretty-printed Python).
  """
  format = 'gypd'


class TestGypMake(TestGypBase):
  """
  Subclass for testing the GYP Make generator.
  """
  format = 'make'
  build_tool_list = ['make']
  def build_all(self, gyp_file, **kw):
    """
    Builds the Make 'all' target to build all targets for the Makefiles
    generated from the specified gyp_file.
    """
    self.run_build(gyp_file, 'all', **kw)
  def build_default(self, gyp_file, **kw):
    """
    Runs Make with no additional command-line arguments to get the
    default build for the Makefiles generated from the specified gyp_file.
    """
    self.run_build(gyp_file, **kw)
  def build_target(self, gyp_file, target, **kw):
    """
    Runs a Make build with the specified target on the command line
    to build just that target using the Makefile generated from the
    specified gyp_file.
    """
    self.run_build(gyp_file, target, **kw)
  def run_build(self, gyp_file, *args, **kw):
    """
    Runs a Make build using the Makefiles generated from the specified
    gyp_file.
    """
    if self.configuration:
      args += ('BUILDTYPE=' + self.configuration,)
    return self.run(program=self.build_tool, arguments=args, **kw)
  def run_built_executable(self, name, *args, **kw):
    """
    Runs an executable built by Make.
    """
    configuration = self.configuration or 'Default'
    os.environ['LD_LIBRARY_PATH'] = self.workpath(configuration, 'lib')
    # Enclosing the name in a list avoids prepending the original dir.
    program = [os.path.join('out', configuration, name)]
    return self.run(program=program, *args, **kw)


class TestGypMSVS(TestGypBase):
  """
  Subclass for testing the GYP Visual Studio generator.
  """
  format = 'msvs'

  # Initial None element will indicate to our .initialize_build_tool()
  # method below that 'devenv' was not found on %PATH%.
  build_tool_list = [None, 'devenv']

  def build_all(self, gyp_file, **kw):
    """
    Runs devenv.exe with no target-specific options to get the "all"
    build for the Visual Studio configuration generated from the
    specified gyp_file.

    (NOTE:  This is the same as the default, our generated Visual Studio
    configuration doesn't create an explicit "all" target.)
    """
    return self.run_build(gyp_file, **kw)
  def build_default(self, gyp_file, **kw):
    """
    Runs devenv.exe with no target-specific options to get the default
    build for the Visual Studio configuration generated from the
    specified gyp_file.
    """
    return self.run_build(gyp_file, **kw)
  def build_target(self, gyp_file, target, **kw):
    """
    Uses the devenv.exe /Project option to build the specified target with
    the Visual Studio configuration generated from the specified gyp_file.
    """
    return self.run_build(gyp_file, '/Project', target, **kw)
  def initialize_build_tool(self):
    """
    Initializes the Visual Studio .build_tool parameter, searching %PATH%
    and %PATHEXT% for a devenv.{exe,bat,...} executable, and falling
    back to a hard-coded default (on the current drive) if necessary.
    """
    super(TestGypMSVS, self).initialize_build_tool()
    if not self.build_tool:
      # We didn't find 'devenv' on the path.  Just hard-code a default,
      # and revisit this if it becomes important.
      self.build_tool = os.path.join('\\Program Files',
                                     'Microsoft Visual Studio 8',
                                     'Common7',
                                     'IDE',
                                     'devenv.exe')
  def run_build(self, gyp_file, *args, **kw):
    """
    Runs a Visual Studio build using the configuration generated
    from the specified gyp_file.
    """
    configuration = self.configuration or 'Default'
    args = (gyp_file.replace('.gyp', '.sln'), '/Build', configuration) + args
    if self.configuration:
      args += ('/ProjectConfig', self.configuration,)
    return self.run(program=self.build_tool, arguments=args, **kw)
  def run_built_executable(self, name, *args, **kw):
    """
    Runs an executable built by Visual Studio.
    """
    configuration = self.configuration or 'Default'
    # Enclosing the name in a list avoids prepending the original dir.
    program = [os.path.join(configuration, '%s.exe' % name)]
    return self.run(program=program, *args, **kw)


class TestGypSCons(TestGypBase):
  """
  Subclass for testing the GYP SCons generator.
  """
  format = 'scons'
  build_tool_list = ['scons', 'scons.py']
  def build_all(self, gyp_file, **kw):
    """
    Builds the scons 'all' target to build all targets for the
    SCons configuration generated from the specified gyp_file.
    """
    self.run_build(gyp_file, 'all', **kw)
  def build_default(self, gyp_file, **kw):
    """
    Runs scons with no additional command-line arguments to get the
    default build for the SCons configuration generated from the
    specified gyp_file.
    """
    self.run_build(gyp_file, **kw)
  def build_target(self, gyp_file, target, **kw):
    """
    Runs a scons build with the specified target on the command line to
    build just that target using the SCons configuration generated from
    the specified gyp_file.
    """
    self.run_build(gyp_file, target, **kw)
  def run_build(self, gyp_file, *args, **kw):
    """
    Runs a scons build using the SCons configuration generated from the
    specified gyp_file.
    """
    dirname = os.path.dirname(gyp_file)
    if dirname:
      args += ('-C', dirname)
    if self.configuration:
      args += ('--mode=' + self.configuration,)
    return self.run(program=self.build_tool, arguments=args, **kw)
  def run_built_executable(self, name, *args, **kw):
    """
    Runs an executable built by scons.
    """
    configuration = self.configuration or 'Default'
    os.environ['LD_LIBRARY_PATH'] = self.workpath(configuration, 'lib')
    # Enclosing the name in a list avoids prepending the original dir.
    program = [os.path.join(configuration, name)]
    return self.run(program=program, *args, **kw)


class TestGypXcode(TestGypBase):
  """
  Subclass for testing the GYP Xcode generator.
  """
  format = 'xcode'
  build_tool_list = ['xcodebuild']
  def build_all(self, gyp_file, **kw):
    """
    Uses the xcodebuild -alltargets option to build all targets for the
    .xcodeproj generated from the specified gyp_file.
    """
    return self.run_build(gyp_file, '-alltargets', **kw)
  def build_default(self, gyp_file, **kw):
    """
    Runs xcodebuild with no target-specific options to get the default
    build for the .xcodeproj generated from the specified gyp_file.
    """
    return self.run_build(gyp_file, **kw)
  def build_target(self, gyp_file, target, **kw):
    """
    Uses the xcodebuild -target option to build the specified target
    with the .xcodeproj generated from the specified gyp_file.
    """
    return self.run_build(gyp_file, '-target', target, **kw)
  def run_build(self, gyp_file, *args, **kw):
    """
    Runs an xcodebuild using the .xcodeproj generated from the specified
    gyp_file.
    """
    args = ('-project', gyp_file.replace('.gyp', '.xcodeproj')) + args
    if self.configuration:
      args += ('-configuration', self.configuration)
    symroot = kw.get('SYMROOT', '$SRCROOT/build')
    if symroot:
      args += ('SYMROOT='+symroot,) + args
    return self.run(program=self.build_tool, arguments=args, **kw)
  def run_built_executable(self, name, *args, **kw):
    """
    Runs an executable built by xcodebuild.
    """
    configuration = self.configuration or 'Default'
    os.environ['DYLD_LIBRARY_PATH'] = self.workpath('build', configuration)
    # Enclosing the name in a list avoids prepending the original dir.
    program = [os.path.join('build', configuration, name)]
    return self.run(program=program, *args, **kw)


format_class_list = [
  TestGypGypd,
  TestGypMake,
  TestGypMSVS,
  TestGypSCons,
  TestGypXcode,
]

def TestGyp(*args, **kw):
  """
  Returns an appropriate TestGyp* instance for a specified GYP format.
  """
  format = kw.get('format')
  if format:
    del kw['format']
  else:
    format = os.environ.get('TESTGYP_FORMAT')
  for format_class in format_class_list:
    if format == format_class.format:
      return format_class(*args, **kw)
  raise Exception, "unknown format %r" % format