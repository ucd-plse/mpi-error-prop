from SCons.Script import *

here = File(__file__).dir


########################################################################
#
#  generate a weighted pushdown system from an LLVM bitcode file
#


def llvm_wpds_emitter(target, source, env):
    [wpds] = target
    calls = wpds.target_from_source('', '.may-must-call')
    globals = wpds.target_from_source('', '.may-must-modify-global')
    target = [wpds, calls, globals]
    return env.ValgrindEmitter(target, source)


def llvm_wpds_target_scanner(node, env, path):
    return env.ValgrindTargetScanner(node, path)


def plugin_ld_library_path(target, source, env, for_signature):
    compiler = env.WhereIs('$CXX')
    bindir = File(compiler).dir
    if bindir.path != '/usr/bin':
        libdir = Dir('lib', bindir.dir)
        envar = 'LD_LIBRARY_PATH=%s' % libdir
        return envar


def generate(env):
    builder_name = 'LlvmWpds'
    if builder_name in env['BUILDERS']:
        return

    env.Tool('wpds', toolpath=(here.dir,))

    builder = env.WPDSBuilder(
        engine='WPDS_PLUGIN',
        action=Action([
                ['$__PLUGIN_LD_LIBRARY_PATH', '$__VALGRIND', 'opt', '-load', '$WPDS_PLUGIN', '-lowerswitch', '-reg2mem', '-rules', '-sign=$ERROR_CODES_SIGN', '-error-codes=$ERROR_CODES', '-schema=${WPDS_SCHEMA[0].abspath}', '-output=${TARGETS[0]}', '-may-must-call', '-may-must-call-output=${TARGETS[1]}', '-may-must-modify-global', '-may-must-modify-global-output=${TARGETS[2]}', '-disable-output', '$SOURCES'],
                ['sort', '--output=${TARGETS[1]}', '${TARGETS[1]}'],
                ['sort', '--output=${TARGETS[2]}', '${TARGETS[2]}'],
                ]),
        src_suffix=['.bc', '.ll'],
        src_builder='Bitcode',
        target_scanner=llvm_wpds_target_scanner,
        emitter=llvm_wpds_emitter,
        )

    env.AppendUnique(
        BUILDERS={builder_name: builder},
        WPDS_PLUGIN=env.File('Wpds$SHLIBSUFFIX', here),
        __PLUGIN_LD_LIBRARY_PATH=plugin_ld_library_path,
        )


def exists(env):
    return True
