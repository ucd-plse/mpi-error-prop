import argparse
import os
import csv
import logging

# TODO before master merge, document directory structure

def collect(results_dir, library, phase):
  # Create a map from dropping site to line
  bugs_found = dict()

  # a map from bug id to line
  original_bugs = dict()

  # Open phase-0 unsaved.csv
  for p in range(phase):
    phase_file = open(os.path.join(results_dir, "{}-fixpoint-results".format(library), "phase-{}".format(p), "unsaved.csv"))
    phase_reader = csv.reader(phase_file)

    # Assume we have header
    next(phase_reader)

    for r in phase_reader:
        unsaved_id = r[0]
        drop_site = r[2]
        bugs_found[drop_site] = r
        original_bugs[unsaved_id] = r
    phase_file.close()

  new_bugs = []

  # List folders in phase-1 dir
  phase_dir = os.path.join(results_dir, "{}-fixpoint-results".format(library), "phase-{}".format(phase))
  phase_bug_dirs = [di for di in os.listdir(phase_dir) if os.path.isdir(os.path.join(phase_dir, di))]

  # dropping/origin pair tuples that produced new bugs
  phase_fixes = set()

  # For each folder, attempt to open folder/parsed_unsaved.csv
  for d in phase_bug_dirs:
    # Read parsed_unsaved.csv
    bug_csv_path = os.path.join(phase_dir, d, "parsed_unsaved.csv")
    try:
        bug_file = open(bug_csv_path, 'r')
    except:
        logging.warn("Unable to open {}".format(bug_csv_path))
        continue
    bug_reader = csv.reader(bug_file)

    # Skip the header produced by report_parser.py
    next(bug_reader)

    for r in bug_reader:
        drop_site = r[2]
        # If it is a new bug
        if drop_site not in bugs_found:
          logging.info("Fix for bug {} produced new dropping site: {}".format(d, drop_site))

          # Add line to new csv
          new_bugs.append(r)

          # Prevent duplicates if new bug found multiple times
          bugs_found[drop_site] = r

          # Record dropping/origin pair for the original bug
          original_bug = original_bugs[d]
          phase_fixes.add((original_bug[2], original_bug[7]))

    bug_file.close()

  # Write out new csv as phase-1/unsaved.csv
  new_bugs_path = os.path.join(phase_dir, "unsaved.csv")
  new_bugs_file = open(new_bugs_path, 'w')
  new_bugs_writer = csv.writer(new_bugs_file, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)

  new_bugs_file.write("Error Code, Call Sites to Dropping Function, Type, Validity, FP Type, Dropping Function, Definition of Dropping Function, SHA, Examiner, Date, Notes,\n")
  for i in range(0, len(new_bugs)):
    new_bug = new_bugs[i][1:]
    bug_id = i + 2
    drop_site = new_bug[2]
    origin_site = new_bug[7]
    new_bugs_writer.writerow([bug_id] + new_bug)

  phase_fixes_path = os.path.join(phase_dir, "phasefixes.txt")
  phase_fixes_file = open(phase_fixes_path, 'w')
  for pf in phase_fixes:
    phase_fixes_file.write("{} {}\n".format(pf[0], pf[1]))

  new_bugs_file.close()
  phase_fixes_file.close()

if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)

    parser = argparse.ArgumentParser()
    parser.add_argument('--library', help="mpich or mvapich", required=True)
    parser.add_argument('--results', help="Local copy of directory holding phases (must have parsed_unsaved.csv files)", required=True)
    parser.add_argument('--phase', type=int, help="phase number", required=True)
    args = parser.parse_args()
    collect(args.results, args.library, args.phase)
