from SCons.Script import *


########################################################################
#
#  generate a weighted pushdown system from a C source file
#


def wpds_schema(target, source, env, for_signature):
    if env['WPDS_SCHEMA']:
        return ['--schema', '${WPDS_SCHEMA[0].abspath}']
    else:
        return []


def wpds_main_function_file(target, source, env, for_signature):
    funcfile = env['WPDS_MAIN_FUNCTION_FILE']
    if funcfile:
        return ['--functionfile', env.File(funcfile).abspath]
    else:
        return []


def wpds_emitter(target, source, env):
    [wpds] = target
    calls = wpds.target_from_source('', '.may-must-call')
    globals = wpds.target_from_source('', '.may-must-modify-global')
    target = [wpds, calls, globals]
    return target, source


def generate(env):
    builder_name = 'CilWpds'
    if builder_name in env['BUILDERS']:
        return

    here = File(__file__).dir
    env.Tool('wpds', toolpath=(here.dir,))
    env.Tool('cil', toolpath=(here.Dir('../../cil'),))

    builder = env.WPDSBuilder(
        engine='CILLY',
        emitter=wpds_emitter,
        action=Action([
                ['$CILLY', '--dowpds', '$WPDS_XML_FLAGS', '--queryfile', '$TARGET', '$__WPDS_SCHEMA', '$__WPDS_MAIN_FUNCTION_FILE', '--error-codes', '$ERROR_CODES', '$SOURCES', '--doMayMustCall', '--may-must-call-output', '${TARGETS[1]}', '--doMayMustModifyGlobal', '--may-must-modify-global-output', '${TARGETS[2]}'],
                ['sort', '--output=${TARGETS[1]}', '${TARGETS[1]}'],
                ['sort', '--output=${TARGETS[2]}', '${TARGETS[2]}'],
                ]),
        src_suffix='.i',
        src_builder='Preprocess',
        )

    env.AppendUnique(
        BUILDERS={builder_name: builder},
        WPDS_MAIN_FUNCTION_FILE=None,
        WPDS_XML_FLAGS=[],
        __WPDS_MAIN_FUNCTION_FILE=wpds_main_function_file,
        __WPDS_SCHEMA=wpds_schema,
        )


def exists(env):
    return True
