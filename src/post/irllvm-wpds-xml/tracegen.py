from SCons.Script import Builder, File, Scanner

TRACEGEN_BIN = 'post/irllvm-wpds-xml/tracegen'

def generate_actions(source, target, env, for_signature):
    if "CONFIG_FILE" in env:
        cmd = '%s -c %s -b %s -p %s > %s' % (TRACEGEN_BIN, env["CONFIG_FILE"], source[1], env['PREDS_PATH'], target[0])
    else:
        cmd = '%s -e negative-error-codes.txt -b %s -p %s > %s' % (TRACEGEN_BIN, source[1], env['PREDS_PATH'], target[0])
    return cmd

# We need to add the tracegen executable as a dependency
# to ensure that it is built before any tests use this tool
def tracegen_scanner(node, env, path):
    return ['../../tracegen']


__tracegen_builder = Builder(
    generator=generate_actions,
    target_scanner=Scanner(tracegen_scanner)
    )

def generate(env):
    if 'TRACEGEN' in env['BUILDERS']:
        return

    env.AppendUnique(
        BUILDERS={'TRACEGEN': __tracegen_builder},
        )

def exists():
    return True
