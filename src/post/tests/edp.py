from SCons.Script import Builder, File, Scanner


########################################################################
#
#  run error detection propagation analysis on a weighted pushdown system
#


__edp_action = [['$__VALGRIND', '$EDP', '$EDP_FLAGS', '--error-codes=$ERROR_CODES', '--temps', '--tentative', '--query=$SOURCE', '>$TARGET']]


def __edp_target_scanner(node, env, path):
    dependencies = [env['EDP'], env['ERROR_CODES']]
    dependencies += env.ValgrindTargetScanner(node, path)
    return dependencies


def __edp_emitter(target, source, env):
    return env.ValgrindEmitter(target, source)


__edp_builder = Builder(
    action=__edp_action,
    src_suffix='.wpds',
    src_builder='CilWpds',
    suffix='.out',
    target_scanner=Scanner(__edp_target_scanner),
    single_source=True,
    emitter=__edp_emitter,
    )


########################################################################


def generate(env):
    if 'EDP' in env['BUILDERS']:
        return

    here = File(__file__).dir
    env.AppendUnique(
        EDP=here.File('../main/main'),
        BUILDERS={'EDP': __edp_builder},
        )


def exists(env):
    return True
