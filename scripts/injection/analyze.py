#!/usr/bin/python3

import subprocess
import csv
import sys
import os
import argparse
import pandas
import re
import itertools
import pickle
import logging
from collections import defaultdict
import matplotlib.pyplot as plt

# Order by increasing severity of unfixed behavior
# Must match messages in test_results()
severity = [
    "segfault",                     # 6
    "floating point exception",     # 5
    "timeout",                      # 4
    "test failure",                 # 3
    "test passes",                  # 2
    "assert",                       # 1
    "abort",
    "all tests pass",
    "dropping site",
    "origin site",
    "detected",
    "test build failure",
]

# IDs of messages that indicate error code was not detected
undetected_failures = set(["segfault", "floating point exception", "timeout",
    "test passes", "test failure", "assert"])


def test_results(log_path):
    # These are regular expression that can match multiple lines in a test log
    test_regexes = {
        ".*Segmentation fault.*": "segfault",
        ".*output.*INJECTION CALL.*": "dropping site",
        ".*output.*INJECTION ERROR.*": "origin site",
        ".*Floating point exception.*": "floating point exception",
        ".*error stack.*": "detected",
        ".*called MPI_Abort.*": "detected",
        ".*Assertion failed.*": "assert",
        ".*Failed to build.*": "test build failure",
        "Unexpected output in.+": "test failure",
        ".*INJECTED.*": "nomem triggered"
    }

    # These are regular expressions that can match only once in a test log
    file_regexes = {
        ".*Compile failed.*": "compile error",
        ".*All 1137 tests passed.*": "all tests pass",
    }

    # test name -> set([behavior])
    individual_test_results = defaultdict(set)

    # Summary result of all tests
    report_results = set()

    try:
        f = open(log_path, 'r', errors='replace')
    except FileNotFoundError:
        return (set(["timeout"]), None)

    lines = set(f.readlines())

    for l in lines:
        for regex, behavior in test_regexes.items():
            if re.match(regex, l):
                test_name = l.split(":")[0].split()[-1]
                individual_test_results[test_name].add(behavior)

                # test failure is added to the summary later
                # because it depends on the test not having any other behavior
                if behavior != "test failure":
                    report_results.add(behavior)
        for regex, behavior in file_regexes.items():
            if re.match(regex, l):
                report_results.add(behavior)

    for test_name, behavior in individual_test_results.items():
        if "test failure" in behavior:
            if len(behavior) == 1:
                report_results.add("test failure")
            else:
                behavior.remove("test failure")

    f.close()

    return (report_results, individual_test_results)


def pick_test(unfixed_test_results, fixed_test_results, unfixed_run_results):
    """
    Given a dictionary test_name -> set([test results]),
    returns a test, if one exists, where the error code is not
    detected in unfixed_test_results, but is detected in
    fixed_test_results.

    Returns None if no test can be selected
    """

    if not fixed_test_results:
        return (None, None)

    # test_name -> most severe unfixed behavior
    candidates = dict()

    unfixed_behaviors = dict()

    # sort for determinism
    fixed_test_names = sorted(fixed_test_results.keys())

    # Go through every test in fixed results
    for fixed_test_name in fixed_test_names:
        fixed_behavior = fixed_test_results[fixed_test_name]
        # check to see if error code is detected in fixed version
        if "detected" not in fixed_behavior:
            continue

        # If the run does not exist (file is missing), then behavior is timeout
        if not unfixed_test_results and "all tests pass" not in unfixed_run_results:
            unfixed_behavior = set(["timeout"])
        else:
            unfixed_behavior = unfixed_test_results[fixed_test_name]

        if "detected" in unfixed_behavior:
            continue

        # Save most severe unfixed behavior
        most_severe = sorted(unfixed_behavior, key=lambda x: severity.index(x))

        # Undetected in unfixed, and detected in fixed. Yay!
        if most_severe:
            candidates[fixed_test_name] = most_severe[0] 

    # sort test candidates by severity of most severe unfixed behavior per test
    sorted_candidates = sorted(candidates.keys(), 
        key=lambda x: severity.index(candidates[x]))

    # return the selected test where unfixed behavior is most severe
    if len(candidates) == 0:
        return (None, None)
    return (sorted_candidates[0], candidates[sorted_candidates[0]])


def covered_tests(full_test_results, report_id):
    """Returns sets of test names that are covered for the report id

    full_test_results: test_name -> test result
    (dropping, origin)
    """

    dropping = set()
    origin = set()

    coverage_none_results = full_test_results[report_id]["coverage-coverage"]

    if not coverage_none_results:
        return (dropping, origin)

    for test_name, behavior in coverage_none_results.items():
        if "dropping site" in behavior:
            dropping.add(test_name)   
        if "origin site" in behavior:
            origin.add(test_name)

    return (dropping, origin)




def unsaved_bugs(results_dir, library):
  """ Returns (phase, bug) tuples for all phases for the library """

  # Get list of phases using os.listdir
  library_bugs_dir = os.path.join(results_dir, "{}-fixpoint-results".format(library))
  phase_dirs = [x for x in os.listdir(library_bugs_dir) if 'phase' in x]

  # Read error-prop unsaved reports for the phase
  phase_bugs = []     # List of tuples (phase, bugid)
  for phase_dir in phase_dirs:
      unsaved_path = os.path.join(library_bugs_dir, phase_dir, "unsaved.csv")
      phase = int(phase_dir.replace('phase-', ''))
      try:
          f = open(unsaved_path)
      except:
          continue

      phase_reader = csv.reader(f)
      next(phase_reader)
      for r in phase_reader:
          phase_bugs.append((phase, int(r[0])))

      f.close()

  return phase_bugs


def fault_injection_log_path(results_dir, library, phase, bug, run):
    fault_injection_dir = os.path.join(results_dir, "fault-injection")
    job_str = "{}-phase{}-{}-testing-{}".format(library, phase, run, bug)
    log_dir = os.path.join(fault_injection_dir, job_str)
    log_file_path = os.path.join(log_dir, "{}.log".format(job_str))
    return log_file_path


def nomem_covered_tests(full_test_results, report_id):
    """Returns sets of test names that are covered in nomem log for the
    report id. Only the nomem_cov equivalent of origin site.
    """

    nomem_origin = set()

    print(report_id)
    nomem_cov_none_results = full_test_results[report_id]["nomemcov-nomemcov"]
    if not nomem_cov_none_results:
        return nomem_origin

    for test_name, behavior in nomem_cov_none_results.items():
        if "nomem triggered" in behavior:
            nomem_origin.add(test_name)

    return nomem_origin


def impacted_tests(run, full_test_results, report_id):
    # Find the number of covered tests, both ends covered for now
    drop_covered, origin_covered = covered_tests(full_test_results, report_id)

    test_names = set()

    # For each test that is covered but does not detect the error,
    return_none_results = full_test_results[report_id][run]
    if not return_none_results:
        return test_names

    for test_name, behavior in return_none_results.items():
        if test_name in origin_covered:
            test_names.add(test_name)

    return test_names


def tests_with_behavior(run, full_test_results, report_id, behavior_query):
    run_results = full_test_results[report_id][run]

    matching_test_names = set()
    if run_results == None:
        return matching_test_names

    for test_name, behaviors in run_results.items():
        if behavior_query in behaviors:
            matching_test_names.add(test_name)

    return matching_test_names

def _detected_bugs(full_test_results, run):
    detected = set()
    for report_id in full_test_results.keys():
        if tests_with_behavior(run, full_test_results, report_id, "detected"):
            detected.add(report_id)
    return detected

def _reproduced_bugs(full_test_results, run_results, run):
    # A report is reproduced if there exists a test that triggered injection
    # and it it was not detected
    reproduced = set()

    for report_id in full_test_results.keys():
        if "compile error" in run_results[report_id][run]:
            continue
        if "test build failure" in run_results[report_id][run]:
            continue

        if "return-" in run:
            _, covered = covered_tests(full_test_results, report_id)
        elif "nomem-" in run:
            covered = nomem_covered_tests(full_test_results, report_id)
        else:
            raise Exception("unknown run name")

        detected_tests = tests_with_behavior(run, full_test_results, report_id, "detected")

        reproduced_tests = covered.difference(detected_tests)
        if len(reproduced_tests) > 0:
            reproduced.add(report_id)

    return reproduced


def silent_failures(run, full_test_results):
    silent = set()
    for report_id in full_test_results.keys():
        if not full_test_results[report_id][run]:
            continue	

        # Get all the covered tests
        if "return-" in run:
            _, covered = covered_tests(full_test_results, report_id)
        elif "nomem-" in run:
            covered = nomem_covered_tests(full_test_results, report_id)
        else:
            raise Exception("unknown run name")

        # Get all the passing tests
        for test_name in covered:
            if test_name not in full_test_results[report_id][run]:
                silent.add(report_id)

    return silent

def avg_severity(run, full_test_results, report_id):
    global severity

    # Get the impacted tests.
    impacted = impacted_tests(run, full_test_results, report_id)
    if len(impacted) == 0:
        return 0.0

    total_severity = 0.0

    # For each impacted test, get the behavior
    return_none_results = full_test_results[report_id][run]
    for test_name in impacted:
        # TODO: take most severe behavior of any single tset
        behavior = "".join(next(iter(return_none_results[test_name])))
        behavior_severity = (len(severity) - severity.index(behavior)) - 6 
        # map segfault and fpexception to same
        if behavior_severity == 6:
            behavior_severity = 5
        total_severity += behavior_severity

    return total_severity / len(impacted)


def coverage_table(full_test_results):
    """Outputs the bug report IDs that have their dropping site covered
    """
    for report_id in full_test_results.keys(): 
        if full_test_results[report_id]["coverage-coverage"] != None:
            print(report_id)

def pareto_table(full_test_results):
    plt.style.use("ggplot")
    plt.rcParams['axes.edgecolor'] = "#777777"
    plt.rcParams['axes.facecolor'] = '#FFFFFF'
    fig, ax = plt.subplots()
    ax.set_xlabel("Average Severity")
    ax.set_ylabel("# Impacted Tests")

    return_none_x = []
    return_none_y = []
    for report_id in full_test_results.keys():
        num_impacted_tests = len(impacted_tests("return-none", full_test_results, report_id))
        avg_sev = avg_severity("return-none", full_test_results, report_id)
        if avg_sev < 1.0:
            continue
        return_none_x.append(avg_sev)
        return_none_y.append(num_impacted_tests)
        print(report_id, avg_sev, num_impacted_tests)

        if report_id == 112:
            mpi_comm_split_coords = (avg_sev, num_impacted_tests)
        if report_id == 118:
            mpi_comm_dup_coords = (avg_sev, num_impacted_tests)
    return_ax = ax.scatter(return_none_x, return_none_y, marker='D')

    nomem_none_x = []
    nomem_none_y = []
    for report_id in full_test_results.keys():
        num_impacted_tests = len(impacted_tests("nomem-none", full_test_results, report_id))
        avg_sev = avg_severity("nomem-none", full_test_results, report_id)
        if avg_sev < 1.0:
            continue
        nomem_none_x.append(avg_sev)
        nomem_none_y.append(num_impacted_tests)
        print(report_id, avg_sev, num_impacted_tests)
    nomem_ax = ax.scatter(nomem_none_x, nomem_none_y, alpha=0.5)

    leg = plt.legend([return_ax, nomem_ax],
        ["Return Error Code", "Memory Failure"],
        scatterpoints=1,
        loc='upper left',
        ncol=1,
        fontsize=16,
        frameon=True
    )

    ax.annotate('MPI_Comm_dup', xy=mpi_comm_dup_coords, xytext=(mpi_comm_split_coords[0]-3, mpi_comm_split_coords[1]+200), arrowprops=dict(facecolor='black', shrink=0.05, width=2), fontsize=16)
    ax.annotate('MPI_Comm_split', xy=mpi_comm_split_coords, xytext=(mpi_comm_split_coords[0]-3, mpi_comm_split_coords[1]+50), arrowprops=dict(facecolor='black', shrink=0.05, width=2), fontsize=16)

    ax.tick_params(axis='both', which='major', labelsize=14)
    ax.set_xticks([1, 2, 3, 4, 5])
    ax.set_xticklabels(["assert", "silent", "test fail", "hang", "crash"])

    plt.show()

def _job_names(results_dir, library, runs):
    """Returns a generator of (phase, run, bug, job name) tuples 
    based on the report  ids present in each phase unsaved.csv.
    """

    # Get list of phases using os.listdir
    library_bugs_dir = os.path.join(results_dir, "{}-fixpoint-results".format(library))
    phase_dirs = [x for x in os.listdir(library_bugs_dir) if 'phase' in x]

    # Read error-prop unsaved reports for the phase
    phase_bugs = []     # List of tuples (phase, bugid)
    for phase_dir in phase_dirs:
        unsaved_path = os.path.join(library_bugs_dir, phase_dir, "unsaved.csv")
        phase_job_str = phase_dir.replace('-', '')
        try:
            f = open(unsaved_path)
        except:
            continue

        phase_reader = csv.reader(f)
        next(phase_reader)
        for r in phase_reader:
            phase_bugs.append((phase_job_str, r[0]))

        f.close()

    # for each phase
    # read the unsaved csv file to get the list of report ids
    job_parts = [phase_bugs, runs]

    for j in itertools.product(*job_parts):
        phase = j[0][0]
        phasenum = int(j[0][0][5:])
        run = j[1]
        bug = j[0][1]
        yield (phasenum, run, bug, "{}-{}-{}-testing-{}".format(library, phase, run, bug))


def parse_logs(results_dir, library):
    runs = ["coverage-coverage", "nomem-none", "return-none", 
    "return-return", "nomemcov-nomemcov", "nomem-return"]

    # ID -> run_name -> test_name -> test result
    # Individual test results
    full_test_results = defaultdict(dict)

    # ID -> run_name -> report results
    # All behaviors observed in all tests for error-prop report
    run_results = defaultdict(dict)

    # Parse the log files
    # Produces run_results and full_test_results
    job_names = _job_names(results_dir, library, runs)
    for phase, run, bug, job_name in job_names:
        logging.debug(job_name)
        log_path = os.path.join(results_dir, "fault-injection", job_name, "{}.log".format(job_name))
        results = test_results(log_path)
        aggregate_report_results = results[0]
        run_results[(phase, int(bug))][run] = aggregate_report_results
        full_test_results[(phase, int(bug))][run] = results[1]
        logging.debug(aggregate_report_results)

    return (full_test_results, run_results)

def _bugs_with_behaviors(full_test_results, run, behaviors):
    """Returns set of crashing bugs for library and strategy
    full_test_results: parsed logs
    run: "injectionstrategy-fixstrategy"
    behaviors: list of behaviors from parsed test log
    """

    bugs = set()
    for bug in full_test_results.keys():
        if run not in full_test_results[bug]:
            continue
        r = full_test_results[bug][run]
        if not r:
            continue
        for test, test_behaviors in r.items():
            for b in behaviors:
                if b in test_behaviors:
                    bugs.add(bug)
    return bugs


def covered_bugs(results_dir, library):
    """ Returns (phase, bug) tuples for all bugs where dropping site is covered """
    full_test_results, _ = parse_logs(results_dir, library)
    return _covered_dropping_bugs(full_test_results)

def _covered_dropping_bugs(full_test_results):
    return _bugs_with_behaviors(full_test_results, "coverage-coverage", ['dropping site'])

def _covered_origin_bugs(full_test_results):
    return _bugs_with_behaviors(full_test_results, "coverage-coverage", ['origin site'])

def _crashing_bugs(full_test_results, run):
    """Returns set of crashing bugs for library and strategy"""
    return _bugs_with_behaviors(full_test_results, run, ['segfault', 'floating point exception'])


def _test_fail_bugs(full_test_results, run):
    return _bugs_with_behaviors(full_test_results, run, ['test failure'])

def _silent_fail_bugs(run_results, coverage_run, run):
    # For each bug
    # If coverage run covers origin site
    # and run_results are all_tests pass
    # then it is a silent fail bug
    pass

def _crashing_tests(full_test_results):
    crashingtests = set()

    report_ids = full_test_results.keys()
    for i in report_ids:
        if not full_test_results[i]["return-none"]:
            continue
        for test_name, behavior in full_test_results[i]["return-none"].items():
            if "segfault" in behavior:
                crashingtests.add(test_name)

        if not full_test_results[i]["nomem-none"]:
            continue
        for test_name, behavior in full_test_results[i]["nomem-none"].items():
            if "segfault" in behavior:
                crashingtests.add(test_name)

    return crashingtests

# UNUSED
def select_tests(full_test_results, run_results, fixed_run, unfixed_run):
    """
    fixed_run: str such as "return-return"
    unfixed_run: str such as "return-none"
    """
    # ID -> (selected test, unfixed behavior)
    selected_tests = dict() 

    for bug in full_test_results.keys():
        unfixed_full = full_test_results[bug][unfixed_run]
        fixed_full = full_test_results[bug][fixed_run]
        selected_test, unfixed_behavior = pick_test(unfixed_full, fixed_full, run_results[bug][unfixed_run])

        # Already picked a test, only update if more severe
        if selected_test and bug in selected_tests:
            if severity.index(unfixed_behavior) < severity.index(selected_tests[i][1]):
                selected_tests[bug] = (selected_test, unfixed_behavior)
        # Haven't yet picked a test, update
        elif selected_test:
            selected_tests[bug] = (selected_test, unfixed_behavior)

    #for bug in full_test_results.keys():
    #    if bug not in selected_tests:
    #        selected_tests[bug] = (None, None)

    return selected_tests


def analyze(results_dir, output_path, table):
    mpich_test_results, mpich_run_results = parse_logs(results_dir, "mpich")
    mvapich_test_results, mvapich_run_results = parse_logs(results_dir, "mvapich")

    # Table 3
    mpich_eci_covered = _covered_bugs(mpich_test_results)
    mpich_eci_crashes = _crashing_bugs(mpich_test_results, "return-none")
    mpich_eci_test_fails = _test_fail_bugs(mpich_test_results, "nomem-none")
    print("MPICH ECI covered: {}".format(mpich_eci_covered))
    print("MPICH ECI crashes: {}".format(len(mpich_eci_crashes)))
    print("MPICH ECI Test fails: {}".format(len(mpich_eci_test_fails)))

    mvapich_eci_crashes = _crashing_bugs(mvapich_test_results, "return-none")
    print("MVAPICH ECI covered: {}".format(len(mvapich_eci_crashes)))
    print("MVAPICH ECI crashes: {}".format(len(mvapich_eci_crashes)))

    mpich_mfi_crashes = _crashing_bugs(mpich_test_results, "nomem-none")
    mpich_mfi_test_fails = _test_fail_bugs(mpich_test_results, "nomem-none")
    print("MPICH MFI covered: ?")
    print("MPICH MFI crashes: {}".format(len(mpich_mfi_crashes)))
    print("MPICH MFI Test fails: {}".format(len(mpich_mfi_test_fails)))

    # Table 5
    mpich_eci_reproduced_before_fix = _reproduced_bugs(mpich_test_results, mpich_run_results, "return-none")
    mpich_eci_reproduced_after_fix = _reproduced_bugs(mpich_test_results, mpich_run_results, "return-return")

    return

    unfixed_runs = ["return-none", "nomem-none"]
    fixed_runs = ["return-return", "return-error_code"]

    if table == "pareto":
        pareto_table(full_test_results)
        return

    if table == "coverage":
        coverage_table(full_test_results)
        return


    unsaved_file = open(os.path.join(results_dir, "{}-fixpoint-results".format(library), "phase-{}".format(phase), "unsaved.csv"))
    unsaved_reader = csv.reader(unsaved_file)
    for r in unsaved_reader:
        unsaved_id = r[0]
        drop_site = r[2]
        origin_site = r[7]
        report_dropped[unsaved_id] = drop_site
        report_origin[unsaved_id] = origin_site

    df = pandas.DataFrame(columns=['ID', 'Dropping Site', 'Origin Site', 'Coverage',
        'Error Type', 'Selected Test', 'Test Unfixed', 'return/none', 'nomem/none', 'return/return',
        'return/error_code'])

    for i in report_ids:
        report = str(i)
        report_run_result = run_results[i]

        coverage = report_run_result["coverage-coverage"]

        drop_site = report_dropped[report]
        origin_site = report_origin[report]

        df = df.append({
            "ID": i,
            "Dropping Site": drop_site,
            "Origin Site": origin_site,
            "Selected Test": selected_tests[i][0],
            "Test Unfixed": selected_tests[i][1],
            "Coverage": ",".join(report_run_result["coverage-coverage"]),
            "return/none": ",".join(report_run_result["return-none"]),
            "return/return": ",".join(report_run_result["return-return"]),
            "return/error_code": ",".join(report_run_result["return-error_code"]),
            "nomem/none": ",".join(report_run_result["nomem-none"])
        }, ignore_index=True)

        df.to_csv(output_path, index=False)

    # Total number of unsaved reports
    preamble_unsaved_total = len(df)

    # How many unsaved reports have dropping site covered
    preamble_dropcovered = df["Coverage"].str.contains("dropping site").sum()

    # How many unsaved reports have origin site covered
    # (subset of those that have dropping site covered)
    preamble_origincovered = df["Coverage"].str.contains("origin site").sum()

    # Total number of tests
    preamble_totaltests = 1137

    preamble_crashingtests = len(crashingtests)

    preamble_crashingtestpct = round(float(preamble_crashingtests) / preamble_totaltests, 3) * 100

    print("Preamble")
    print("=========")
    print("\\newcommand{{\\unsavedreports}}[0]{{{}\\xspace}}".format(preamble_unsaved_total))
    print("\\newcommand{{\\dropcovered}}[0]{{{}\\xspace}}".format(preamble_dropcovered))
    print("\\newcommand{{\\origincovered}}[0]{{{}\\xspace}}".format(preamble_origincovered))
    print("\\newcommand{{\\totaltests}}[0]{{{}\\xspace}}".format(preamble_totaltests))
    print("\\newcommand{{\\crashingtests}}[0]{{{}\\xspace}}".format(preamble_crashingtests))
    print("\\newcommand{{\\crashingtestpct}}[0]{{{:.1f}\%\\xspace}}".format(preamble_crashingtestpct))
    print()

    print("Table: Consequences")
    print("============================")
    print(paper_table_consequences(df, full_test_results).to_latex(index=False))

    print("Table: Before and after fix")
    print("============================")
    print(paper_table_detection(full_test_results, run_results).to_latex(index=False))


def paper_table_detection(full_test_results, report_results):
    """Produces table listing number of reports detected before 
    and after fixes are applied.
    """

    table = pandas.DataFrame(columns=['Injection Strategy', 'Fixed?', 'Detected'])

    return_none_detected = detected_reports("return-none", full_test_results)
    return_return_detected = detected_reports("return-return", full_test_results)
    return_ec_detected = detected_reports("return-error_code", full_test_results)
    nomem_none_detected = detected_reports("nomem-none", full_test_results)
    nomem_return_detected = detected_reports("nomem-return", full_test_results)
    nomem_ec_detected = detected_reports("nomem-error_code", full_test_results)

    return_none_reproduced = reproduced_reports("return-none", full_test_results, report_results)
    return_return_reproduced = reproduced_reports("return-return", full_test_results, report_results)
    return_ec_reproduced = reproduced_reports("return-error_code", full_test_results, report_results)
    nomem_none_reproduced = reproduced_reports("nomem-none", full_test_results, report_results)
    nomem_return_reproduced = reproduced_reports("nomem-return", full_test_results, report_results)
    nomem_ec_reproduced = reproduced_reports("nomem-error_code", full_test_results, report_results)

    print("Either reproduced", len(nomem_none_reproduced.union(return_none_reproduced)))

    # How many are detected for return-none
    table = table.append({
        "Injection Strategy": "Error Code",
        "Fixed?": "No",
        "Detected": len(return_none_detected),
        "Reproduced": len(return_none_reproduced)
    }, ignore_index=True)

    # how many are detected when either fix is used
    table = table.append({
        "Injection Strategy": "Error Code",
        "Fixed?": "Yes",
        "Detected": len(return_return_detected.union(return_ec_detected)),
        "Reproduced": len(return_return_reproduced.union(return_ec_reproduced))
    }, ignore_index=True)

    # How many are detected for return-none
    table = table.append({
        "Injection Strategy": "Memory Failure",
        "Fixed?": "No",
        "Detected": len(nomem_none_detected),
        "Reproduced": len(nomem_none_reproduced)
    }, ignore_index=True)

    # how many are detected when either fix is used
    table = table.append({
        "Injection Strategy": "Memory Failure",
        "Fixed?": "Yes",
        "Detected": len(nomem_return_detected.union(nomem_ec_detected)),
        "Reproduced": len(nomem_return_reproduced.union(nomem_ec_reproduced))
    }, ignore_index=True)

    return table


def paper_table_consequences(dfr, full_test_results):
    """Produces the table listing consequences of injection.

    This table is produced by matching individual tests.

    Parameter: pandas dataframe of full injection spreadsheet
    """

    # This table should probably be for all 58
    # Show detected for the 44 and original behavior for the 14

    table = pandas.DataFrame(columns=['Injection Strategy', 'Crash', 'Hang', 
    'Test Failure'])

    covered = dfr[dfr["Coverage"].str.contains("origin site")]

    # Get sums of behaviors for return-none where fix worked
    col = "return/none"
    num_crashes = dfr[col].str.contains("segfault").sum()
    num_test_failures = dfr[col].str.contains("test failure").sum()
    num_test_passes = dfr[col].str.contains("test passes").sum()
    num_detected = dfr[col].str.contains("detected").sum()
    num_timeout = dfr[col].str.contains("timeout").sum()
    num_silent = len(silent_failures("return-none", full_test_results))

    table = table.append({
        "Injection Strategy": "Return Error Code",
        "Crash": num_crashes,
        "Hang": num_timeout,
        "Test Failure": num_test_failures,
        "Silent Failure": num_silent
    }, ignore_index=True)

    col = "nomem/none"
    num_crashes = dfr[col].str.contains("segfault").sum()
    num_test_failures = dfr[col].str.contains("test failure").sum()
    num_test_passes = dfr[col].str.contains("test passes").sum()
    num_detected = dfr[col].str.contains("detected").sum()
    num_timeout = dfr[col].str.contains("timeout").sum()
    num_silent = len(silent_failures("nomem-none", full_test_results))

    table = table.append({
        "Injection Strategy": "Memory Failure",
        "Crash": num_crashes,
        "Hang": num_timeout,
        "Test Failure": num_test_failures,
        "Silent Failure": num_silent
    }, ignore_index=True)

    return table 


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)

    parser = argparse.ArgumentParser()
    parser.add_argument('--results', help="Path to results directory", required=True)
    parser.add_argument('--output', help="Output file to create", required=False)
    parser.add_argument('--table', help="Table to create", required=False)
    args = parser.parse_args()

    analyze(args.results, args.output, args.table)
