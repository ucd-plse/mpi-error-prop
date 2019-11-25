#!/usr/bin/python3

import sys
import os.path
import logging
import argparse

nomem_cov_string = """
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

extern void *__libc_malloc(size_t size);
void *malloc(size_t size) {
    if (magic_call == 12345) { printf("MALLOC FAULT INJECTED"); return (void*) 0; }
    return __libc_malloc(size);
}

extern void *__libc_calloc(size_t num, size_t size);
void *calloc(size_t num, size_t size) {
    if (magic_call == 12345) { printf("CALLOC FAULT INJECTED"); return (void*) 0; }
    return __libc_calloc(num, size);
}

extern void *__libc_realloc(void *ptr, size_t size);
void *realloc(void *ptr, size_t size) {
    if (magic_call == 12345) { printf("REALLOC FAULT INJECTED"); return (void*) 0; }
    return __libc_realloc(ptr, size);
}
"""

nomem_string = """
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

extern void *__libc_malloc(size_t size);
void *malloc(size_t size) {
    if (magic_call == 12345) { return (void*) 0; }
    return __libc_malloc(size);
}

extern void *__libc_calloc(size_t num, size_t size);
void *calloc(size_t num, size_t size) {
    if (magic_call == 12345) { return (void*) 0; }
    return __libc_calloc(num, size);
}

extern void *__libc_realloc(void *ptr, size_t size);
void *realloc(void *ptr, size_t size) {
    if (magic_call == 12345) { return (void*) 0; }
    return __libc_realloc(ptr, size);
}
"""

def inject_lines(directory_prefix, input_lines_path):
    with open(input_lines_path, 'r') as f:
        lines = [x.strip() for x in f.readlines()]

    for l in lines:
        dropping_site, origin_site, injection_strategy, fix_strategy = l.split()
        _inject_line(directory_prefix, dropping_site, origin_site, injection_strategy, fix_strategy)


def _inject_line(directory_prefix, dropping_site, origin_site, injection_strategy, fix_strategy):
    # These strings must not create additional lines in the source file.
    # pre_injection_string_call and post_injection_string_call
    # correspond to the fix strategy.
    pre_injection_string_call = 'magic_call = 12345;'
    post_injection_string_call = 'magic_call = 0;'

    # These strings must not create additional lines in the source file.
    # post_injection_string_error and post_injection_string_error
    # correspond to the injection strategy.
    pre_injection_string_error = ''
    post_injection_string_error = ''

    # It is OK for these strings to create additional lines
    file_prepend_string_call = ''
    file_prepend_string_error = ''

    if injection_strategy == "coverage":
        pre_injection_string_error = 'if (magic_call == 12345) { printf("INJECTION ERROR COVERED\\n"); }'
    elif injection_strategy == "return":
        pre_injection_string_error = 'if (magic_call == 12345) { return MPI_ERR_OTHER; }'
    elif injection_strategy == "nomem":
        file_prepend_string_error = nomem_string
    elif injection_strategy == "nomemcov":
        file_prepend_string_error = nomem_cov_string
    elif injection_strategy == "none":
        # fixed bitcode injection doesn't copy the header files
        pre_injection_string_call = ''
        post_injection_string_call = ''
    else:
        raise Exception("Unknown injection strategy!")

    if fix_strategy == "coverage":
        pre_injection_string_call += ' printf("INJECTION CALL COVERED\\n");'
    if fix_strategy == "nomemcov":
        pass
    elif fix_strategy == "error_code":
        pre_injection_string_call += ' *error_code = '
        post_injection_string_call += ' if (*error_code != MPI_SUCCESS) { return; }'
    elif fix_strategy == "return":
        pre_injection_string_call += 'int injection_return_var = '
        post_injection_string_call += 'if (injection_return_var != MPI_SUCCESS) { return injection_return_var; }'
    elif fix_strategy == "none":
        pass
    else:
        raise Exception("Unknown fix strategy!")

    logging.info("dropping_site: {}".format(dropping_site))
    logging.info("origin_site: {}".format(origin_site))
    logging.info("injection strategy: {}".format(injection_strategy))
    logging.info("fix strategy: {}".format(fix_strategy))

    dropping_file = dropping_site.split(":")[0]
    origin_file = origin_site.split(":")[0]
    dropping_line_number = int(dropping_site.split(":")[1])
    origin_line_number = int(origin_site.split(":")[1])

    dropping_target_path = os.path.join(directory_prefix, dropping_file)
    origin_target_path = os.path.join(directory_prefix, origin_file)

    if not os.path.isfile(dropping_target_path):
        dropping_target_path = os.path.join(directory_prefix, "src/mpi/romio")
        dropping_target_path = os.path.join(dropping_target_path, dropping_file)
        if not os.path.isfile(dropping_target_path):
            print("Does not exist: " + dropping_target_path)

    if not os.path.isfile(origin_target_path):
        origin_target_path = os.path.join(directory_prefix, "src/mpi/romio")
        origin_target_path = os.path.join(origin_target_path, origin_file)
        if not os.path.isfile(origin_target_path):
            print("Does not exist: " + origin_target_path)

    # Dropping site first because nomem alters line numbers
    # Assumption here is for multiple injections nomem will be last
    # Inject fix at dropping site
    __inject_line(dropping_target_path, dropping_line_number, pre_injection_string_call, post_injection_string_call, file_prepend_string_call)

    # Inject error at origin site
    __inject_line(origin_target_path, origin_line_number, pre_injection_string_error, post_injection_string_error, file_prepend_string_error)

def __inject_line(target_path, line_number, pre_injection_string, post_injection_string, file_prepend_string):
    control_keywords = set(["if", "else"])
    not_control_keywords = set(["endif", "ifdef", "ifndef"])

    assert(os.path.isfile(target_path))
    logging.info("Injecting into " + target_path + " at line " + str(line_number))
    logging.info("pre-injection string is '{}'".format(pre_injection_string))
    logging.info("post-injection string is '{}'".format(post_injection_string))

    with open(target_path, 'r') as f:
        lines = f.readlines()

    new_lines = []

    line_index = 0
    while line_index < len(lines):
        if line_index == (line_number - 1):
            prev_text_index = line_number - 2
            prev_text = lines[prev_text_index]
            # Ignore lines with comments
            while prev_text.strip().startswith('//') or prev_text.strip().startswith('/*') or prev_text.strip().endswith('*/'):
                prev_text_index -= 1
                prev_text = lines[prev_text_index]

            use_braces = False
            for c in control_keywords:
                if c in prev_text and "{" not in prev_text:
                    use_braces = True
                    for nc in not_control_keywords:
                        if nc in prev_text:
                            use_braces = False
                    if use_braces:
                        break
            if use_braces:
                if prev_text in new_lines:
                    new_lines[new_lines.index(prev_text)] += " {"

            # Check if function call is being assigned to a variable
            if new_lines and new_lines[-1].strip().endswith('='):
                line_index -= 1
                new_lines.pop(-1)

            original_code = lines[line_index]

            # Check for multiline functions
            if original_code.strip().endswith(',') or original_code.strip().endswith('='):
                while not original_code.strip().endswith(';'):
                    line_index += 1
                    original_code += lines[line_index]

            if use_braces:
                post_injection_string += " }"

            # Check for pre-processors in the next line
            if lines[line_index+1].strip().startswith("#"):
                original_code = original_code.strip()+" "
                post_injection_string += "\n"
            new_lines.append(pre_injection_string + original_code + post_injection_string)
        else:
            new_lines.append(lines[line_index])
        line_index += 1

    new_lines.insert(0, file_prepend_string)

    with open(target_path, 'w') as f:
        for l in new_lines:
            f.write(l)


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG, format='[injection] %(asctime)s %(levelname)s %(message)s')
    parser = argparse.ArgumentParser()
    parser.add_argument('dir', help="Directory containing injection target")
    parser.add_argument('input', help="Input lines path")
    args = parser.parse_args()

    directory_prefix = args.dir
    input_lines_path = args.input
    logging.info("directory prefix: {}".format(directory_prefix))
    logging.info("input_lines_path: {}".format(input_lines_path))
    inject_lines(directory_prefix, input_lines_path)
