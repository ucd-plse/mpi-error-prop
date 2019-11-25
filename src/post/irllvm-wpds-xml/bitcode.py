from SCons.Script import *

import runWithCwd


########################################################################
#
#  generate LLVM bitcode from C/C++ source using the Clang front end
#


__bitcode_builder = Builder(
    generator=runWithCwd.generator(['clang', '-c', '-emit-llvm', '-g', '-o']),
    src_suffix=['.c', '.cpp'],
    suffix='.bc',
    source_scanner=CScanner,
    )


########################################################################


def generate(env):
    if 'Bitcode' in env['BUILDERS']:
        return

    env.AppendUnique(
        BUILDERS={'Bitcode': __bitcode_builder},
        )


def exists(env):
    return env.WhereIs('clang')
