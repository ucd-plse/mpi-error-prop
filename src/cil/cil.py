import re

from SCons.Script import File


here = File(__file__).dir


def cil_target(context):
    context.Message('checking CIL build target ... ')
    guesser = here.File('config.guess')
    ok, guess = context.TryAction([[guesser, '>$TARGET']])
    if ok:
        if re.search('86.*linux', guess):
            target = 'x86_LINUX'
        elif re.search('86.*darwin', guess):
            target = 'x86_DARWIN'
        else:
            target = 'unknown'
            ok = False
    else:
        target = 'failed'

    context.Result(target)
    if ok:
        context.env['CIL_TARGET'] = target
        return target
    else:
        context.env.Exit(1)


def generate(env):
    if 'CILLY' in env: return

    config = env.Configure(custom_tests={'CilTarget': cil_target})
    config.CilTarget()
    config.Finish()

    suffix = {True: 'asm', False: 'byte'}[bool(env['NATIVECAML'])]
    cilly = env.File('obj/$CIL_TARGET/cilly.%s.exe' % suffix, here)
    env.AppendUnique(CILLY=cilly)


def exists(env):
    return True
