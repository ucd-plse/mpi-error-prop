from SCons.Script import *


########################################################################
#
#  compress a weighted pushdown system by eliminating uninteresting
#  sequences of identity transformations
#


def compress_emitter(target, source, env):
    suffix = '-c.wpds'
    [source] = source
    assert not source.name.endswith(suffix), '"%s" is already compressed' % source
    target = source.target_from_source('', suffix)
    return env.ValgrindEmitter([target], [source])


def compress_target_scanner(node, env, path):
    return env.ValgrindTargetScanner(node, path)


def generate(env):
    builder_name = 'CompressWpds'
    if builder_name in env['BUILDERS']:
        return

    here = File(__file__).dir
    for tool in 'valgrind', 'wpds':
        env.Tool(tool, toolpath=(here.dir,))

    builder = env.WPDSBuilder(
        engine='COMPRESS_WPDS',
        action='$__VALGRIND $COMPRESS_WPDS $SOURCE >$TARGET',
        src_suffix='.wpds',
        src_builder='CilWpds',
        emitter=compress_emitter,
        single_source=True,
        target_scanner=compress_target_scanner,
        )

    env.AppendUnique(
        BUILDERS={builder_name: builder},
        COMPRESS_WPDS=here.File('compress-wpds'),
        )


def exists(env):
    return True
