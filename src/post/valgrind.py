from SCons.Builder import Builder
from SCons.Scanner import Scanner


########################################################################
#
#  helper settings for running various commands under valgrind
#


def __var_valgrind(target, source, env, for_signature):
    if env['VALGRIND']:
        return ['valgrind', '--tool=memcheck', '--leak-check=full', '--error-exitcode=1', '$__VALGRIND_SUPPRESSIONS', '--log-file=${TARGET}.log']
    else:
        return []


def __var_valgrind_suppressions(target, source, env, for_signature):
    if env['VALGRIND_SUPPRESSIONS']:
        return '--suppressions=$VALGRIND_SUPPRESSIONS'
    else:
        return ''


def ValgrindEmitter(env, target, source):
    target0 = target[0]
    log = target0.abspath + '.log'
    env.Clean(target0, log)
    if env['VALGRIND']:
        target.append(log)
    return target, source


def ValgrindTargetScanner(env, node, path):
    if env['VALGRIND'] and env['VALGRIND_SUPPRESSIONS']:
        return env['VALGRIND_SUPPRESSIONS']
    else:
        return []


def generate(env):
    if hasattr(env, 'ValgrindEmitter'):
        return

    env.AddMethod(ValgrindEmitter)
    env.AddMethod(ValgrindTargetScanner)

    env.AppendUnique(
        VALGRIND_SUPPRESSIONS='',
        __VALGRIND=__var_valgrind,
        __VALGRIND_SUPPRESSIONS=__var_valgrind_suppressions,
        )


def exists(env):
    return True
