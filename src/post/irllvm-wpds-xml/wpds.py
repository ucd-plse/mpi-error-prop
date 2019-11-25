from SCons.Script import *

here = File(__file__).dir


########################################################################
#
#  generate a weighted pushdown system from an LLVM bitcode file
#

def error_codes(target, source, env, for_signature):
    if 'WPDS_ERROR_CODES' in env:
        return env['WPDS_ERROR_CODES']
    else:
        return env['ERROR_CODES']

def schema(target, source, env, for_signature):
    if env['WPDS_SCHEMA']:
        return '${WPDS_SCHEMA[0].abspath}'
    else:
        return ''    

def llvm_wpds_emitter(target, source, env):
    [wpds] = target
    calls = wpds.target_from_source('', '.may-must-call')
    globals = wpds.target_from_source('', '.may-must-modify-global')
    locations = wpds.target_from_source('', '.locations')
    target = [wpds, calls, globals, locations]
    return env.ValgrindEmitter(target, source)


def llvm_wpds_target_scanner(node, env, path):
    return env.ValgrindTargetScanner(node, path)


def generate(env):
    builder_name = 'irLlvmWpds'
    if builder_name in env['BUILDERS']:
        return

    env.Tool('wpds', toolpath=(here.dir,))

    cmd = ['post/irllvm-wpds-xml/frontend', '-c', '$FRONTEND_CONFIG', '-o', '${TARGETS[0]}', '-b', '$SOURCES', '$OPT_FLAGS', '-s', '$__SCHEMA']

    builder = env.WPDSBuilder(
        engine='WPDS_PLUGIN',
        action=Action([cmd]),
        src_suffix=['.bc', '.ll'],
        src_builder='Bitcode',
        target_scanner=llvm_wpds_target_scanner,
        emitter=llvm_wpds_emitter,
        )

    env.AppendUnique(
        BUILDERS={builder_name: builder},
        WPDS_PLUGIN=env.File('libwpds$SHLIBSUFFIX', here),
        __ERROR_CODES=error_codes,
        __SCHEMA=schema
        )


def exists(env):
    return True
