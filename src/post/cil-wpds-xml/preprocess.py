from SCons.Defaults import ObjSourceScan
from SCons.Script import *

import runWithCwd


########################################################################


preprocess_builder = Builder(
    generator=runWithCwd.generator(['$CC', '$CPPFLAGS', '$_CPPDEFFLAGS', '$_CPPINCFLAGS', '-E', '-o']),
    suffix='.i',
    src_suffix='.c',
    source_scanner=ObjSourceScan,
    single_source=True
    )


########################################################################


def generate(env):
    env.AppendUnique(BUILDERS={'Preprocess': preprocess_builder})


def exists(env):
    return True
