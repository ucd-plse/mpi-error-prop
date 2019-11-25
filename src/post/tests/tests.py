from ConfigParser import RawConfigParser
from itertools import imap
from os.path import join
from SCons.Script import *


# Passing tests deleted If: 53 || 
# 41, 42 artificial main
# empty tests 60, 29, 55, 12
# pointers 8-11

here = File(__file__).dir


def needsTestSuite(target):
    """check whether this target needs test suite details"""

    if str(target) == 'test':
        return True

    if str(target).startswith('test-'):
        return True

    if isinstance(target, str):
        target = Entry('#' + target)

    for frontend in ('cil', 'llvm'):
        testsdir = here.Dir('../%s-wpds-xml/tests' % frontend)
        if target.is_under(testsdir): return True
        if testsdir.is_under(target): return True

    return False


def Tests(env):
    """return sequence of all enabled tests, ordered by test number"""
    return sorted(env['tests'].iteritems(), key=lambda item: int(item[0].name[4:]))


def generate(env):
    if 'tests' in env:
        return

    env['tests'] = {}
    env.AddMethod(Tests)

    # skip test suite details unless something actually needs them
    if not any(imap(needsTestSuite, BUILD_TARGETS)):
        return

    defaults = here.File('defaults.conf')

    # If TEST=# variable is specified, only get that test
    pattern = "test%(TEST)s" % env

    for subdir in env.Glob(join(here.abspath, pattern)):
        if not subdir.isdir():
            continue
        config = subdir.File('test.conf')
        parser = RawConfigParser()
        parser.read([defaults.abspath, config.abspath])
        if parser.get('create wpds', 'enabled') != 'none':
            env['tests'][subdir] = parser


def exists(env):
    return True
