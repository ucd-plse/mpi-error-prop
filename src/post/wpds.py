from SCons.Script import *


########################################################################
#
#  generate a builder for weighted pushdown systems
#


def WPDSBuilder(env, engine, action, suffix='.wpds', target_scanner=None, **kwargs):

    def wpds_generator(target, source, env, for_signature):
        actions = [action]
        if env['WPDS_SCHEMA']:
            actions.append('xmllint --noout --schema ${WPDS_SCHEMA[0]} $TARGET')
        return actions

    def wpds_target_scanner(node, env, path):
        deps = env[engine] 

        # Prefer WPDS_ERROR_CODES over ERROR_CODES
        # This allows WPDS to use different error codes from main
        # (necessary for locations analysis)
        if 'WPDS_ERROR_CODES' in env:
            deps = [deps, env['WPDS_ERROR_CODES']]
        else:
            deps = [env[engine], env['ERROR_CODES']]

        if env['WPDS_SCHEMA']:
            deps += env['WPDS_SCHEMA']
        if env['WPDS_MAIN_FUNCTION_FILE']:
            deps.append(env['WPDS_MAIN_FUNCTION_FILE'])
        if target_scanner:
            deps += target_scanner(node, env, path)
        return deps

    return Builder(generator=wpds_generator, target_scanner=Scanner(wpds_target_scanner), suffix=suffix, **kwargs)


########################################################################


def generate(env):
    if hasattr(env, 'WPDSBuilder'):
        return

    env.AddMethod(WPDSBuilder)

    here = File(__file__).dir
    env.AppendUnique(
        WPDS_SCHEMA=map(here.File, [
            'error-prop.xsd',
            'wpds.xsd',
        ]),
        WPDS_MAIN_FUNCTION_FILE='',
        )


def exists(env):
    return True
