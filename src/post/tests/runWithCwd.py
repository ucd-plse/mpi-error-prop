from SCons.Script import Dir, File
from os.path import relpath


########################################################################
#
#  helper for running commands in temporarily-changed subdirectories
#


def generator(argv):

    def __generator(target, source, env, for_signature):
        baseDir = Dir(env.get('basedir'))
        if baseDir and baseDir != Dir('#'):
            targetPaths = [relpath(node.abspath, baseDir.abspath) for node in target]
            sourcePaths = [relpath(node.abspath, baseDir.abspath) for node in source]
            here = File(__file__).dir
            helper = here.File('../tests/run-with-cwd')
            prefix = [helper, baseDir]
        else:
            targetPaths = target
            sourcePaths = source
            prefix = []
        return [prefix + argv + targetPaths + sourcePaths]

    return __generator
