# MPIErrorProp

This is the artifact accompanying the submission "Detecting and Reproducing
Error-Code Propagation Bugs in MPI Implementations."

## Getting Started

This artifact is packaged as a docker image generated from the `Dockerfile`
in the root of the repository. Therefore the host machine needs to have
`docker` installed. If you receive a permissions error when running the
commands in this README, check to make sure your user is in the `docker`
group. The artifact was tested on hosts running Ubuntu 16.04 and Ubuntu
18.04.

To obtain a pre-built version of the docker image:

```
docker pull ucdavisplse/mpi-error-prop-artifact
```

It should not be necessary to enter the container directly. Each step
below contains a separate `docker run` command.

__The host machine must have at least 32GB of RAM.__

The purpose of this artifact is to facilitate reproducibility by 
including commands to reproduce all of the data in the paper. Source code
for MPIErrorProp is included, and scripts are provided to produce the 
data in the paper. Reproducing all of the data for the bug reports takes 
thousands of hours of computation. Therefore it has been broken down so 
that individual bug reports can be reproduced. Still, each command takes 
15 to 30 minutes, and following all of the examples require several hours 
of computation on a standard computer.

Section 1 runs MPIErrorProp on MPICH and MVAPICH, generating bug reports.

Section 2 shows how to use fault injection to study the consequences of a
bug report (in the paper we used this method on all bug reports).

Section 3 compares the fault injection results before and after a partial 
propagation fix has been applied.

Section 4 shows how to apply fault injection to real-world MPI programs.

## 1. Bug Reports

MPIErrorProp currently supports two different MPI implementations: MPICH and
MVAPICH2. MPIErrorProp runs in multiple iterations (sometimes referred to as
phases) that interleave error code propagation analysis and repair. From our
results, it takes three iterations to complete for MPICH or MVAPICH. All
iterations are required to produce the full set of bugs presented in Table 2,
but are not required for reproducing other steps in the artifact.
Intermediate files are provided so that each step can be completed
independently.

To produce the results of the first iteration (without any fixes applied),
use the following command (takes 15 to 30 minutes):

```
# Iteration 0
# Generate csv of MPICH data used to create Table 2 
docker run --rm ucdavisplse/mpi-error-prop-artifact mpich-bugs 0 \
    | tee mpich_bugs_0.csv
```

The output csv will have a few extra lines of log messages at the top. 

Subsequent iterations, after fixes have been applied, can be run by changing
the number a the end of the command to `1` or `2` (examples below). Each
iteration should lead to an increased number of bugs in the csv output
(iteration 0 + new bugs - fixed bugs). The list of bugs MPICH bugs fixed that
led to new bugs in iteration N is provided in
`results/mpich_phaseN_fixes.txt`. 


(Each iteration takes 15 to 30 minutes)
```
# Iteration 1
docker run -it --rm ucdavisplse/mpi-error-prop-artifact mpich-bugs 1 \
    | tee mpich_bugs_1.csv

# Iteration 2
docker run -it --rm ucdavisplse/mpi-error-prop-artifact mpich-bugs 2 \
    | tee mpich_bugs_2.csv
```

Copies of our inspection of the results from all iterations, including
de-duplication of the results between iterations and false positive marking,
are provided in the `results` folder. The manually reviewed files are named
`*_inspected.csv`.

To run MPIErrorProp on MVAPICH, simply swap `mvapich-bugs` for `mpich-bugs`.

(Each iteration takes 15 to 30 minutes)

```
# Iteration 0
# Generate csv of MPICH data used to create Table 2 
docker run -it --rm ucdavisplse/mpi-error-prop-artifact mvapich-bugs 0

# Iteration 1
docker run -it --rm ucdavisplse/mpi-error-prop-artifact mvapich-bugs 1

# Iteration 2
docker run -it --rm ucdavisplse/mpi-error-prop-artifact mvapich-bugs 2
```

## 2. Fault injection: determining consequences of a bug

Section 4.2 of the submission is a study of the consequences of the bugs 
reported by MPIErrorProp. These consequences are exposed by fault injection.

As described in section 3.4, performing fault injection to study a bug
requires a dropping site and an origin site. The dropping site of a bug
corresponds column 3 of the csv created for Table 2, and the origin site
corersponds to column 8. The results in Table 3 of the submission were
produced by performing fault injection using two different strategies for
every eligible bug in the Table 2 results (where eligibility is determined by
code coverage).

Because performing fault injection requires running the MPICH or MVAPICH 
test suite, it is extremely time consuming to produce the entire set of 
Table 3 results. We describe here how to perform fault injection for a 
single bug and collect the result. Table 3 is a tally of the results for 
all bugs.

To perform fault injection for a single bug, execute the following command:

(This command takes 15-30 minutes to run)
```
docker run --rm ucdavisplse/mpi-error-prop-artifact fault-injection \
    NAME DROPPING_SITE ORIGIN_SITE INJECTION_STRATEGY FIX_STRATEGY \
    TARGET LIBRARY ITERATION
```

There are eight parameters:

- NAME: A name for the injection experiment. This can be any string that is valid to include in a path.
- DROPPING_SITE: The source location of the dropping site as reported by MPIErrorProp.
- ORIGIN_SITE: The source location of the origin site as reported by MPIErrorProp.
- INJECTION_STRATEGY: One of [`return`, `nomem`, `none`]. Passing `return` corresponds to using the `ECI` injection strategy, passing `nomem` corresponds to using the `MFI` injection strategy, and `none` logs coverage only.
- FIX_STRATEGY: One of [`return`, `none`]. If `return` is passed, then an attempt
will be made to apply a partial propagation fix to the dropping site. If `none` is passed,
then the dropping site will be left unrepaired.
- TARGET: One of ['testing', 'kripke', 'miniamr', 'minife']. If `testing` is passed, then the test suite corresponding to the MPI implementation will be run with the modified library. Otherwise one of the applications 
will be used instead.
- LIBRARY: One of ['mpich', 'mvapich']
- ITERATION: Which iteration this particular bug belongs to. 

For example, in our results the MPICH phase 0 bug report identified by
dropping site `src/mpi/comm/comm_split.c:328` and origin site
`src/mpi/comm/commutil.c:247` produced a segmentation in the test suite under
fault under fault injection. This consequence can be reproduced using the
following command:

(This command takes 15 to 30 minutes to run)
```
docker run --rm ucdavisplse/mpi-error-prop-artifact fault-injection \
    example-return-none \
    src/mpi/comm/comm_split.c:328 src/mpi/comm/commutil.c:247 \
    return none testing mpich 0 \
    | tee example-return-none.log.txt
```

This command should take between 20 minutes and 60 minutes to run depending
on the performance of the host machine. This will print the test suite log to
the screen and save the output to the file `example-return-none.log.txt`. The
log files were analyzed by matching known patterns of text in the log files.
For examle, if the text 'Segmentation fault' appears in the log file, we know
that a crash occurred in one of the tests because that string does not
normally appear in the log file.

The following text should be present in `example-return-none.log.txt`, indicating that the `attric` test (among others) crashed.

```
Unexpected output in attric: 
Unexpected output in attric: ===================================================================================
Unexpected output in attric: =   BAD TERMINATION OF ONE OF YOUR APPLICATION PROCESSES
Unexpected output in attric: =   PID 16063 RUNNING AT efa2a45c0265
Unexpected output in attric: =   EXIT CODE: 139
Unexpected output in attric: =   CLEANING UP REMAINING PROCESSES
Unexpected output in attric: =   YOU CAN IGNORE THE BELOW CLEANUP MESSAGES
Unexpected output in attric: ===================================================================================
Unexpected output in attric: YOUR APPLICATION TERMINATED WITH THE EXIT STRING: Segmentation fault (signal 11)
Unexpected output in attric: This typically refers to a problem with your application.
Unexpected output in attric: Please see the FAQ page for debugging suggestions
```

### 3. Consequences after a fix 

The impact of a partial propagation fix on the consequences of a bug can be ascertained by changing the fix
strategy from `none` to `return`.

(This command takes 15 to 30 minutes to run)
```
docker run --rm ucdavisplse/mpi-error-prop-artifact fault-injection \
    example-return-return \
    src/mpi/comm/comm_split.c:328 src/mpi/comm/commutil.c:247 \
    return return testing mpich 0 \
    | tee example-return-return.log.txt
```

The `attric` test now produces a fatal error instead of crashing, which
indicates that MPICH has detected the error.

```
Looking in ./attr/testlist
Unexpected output in attric: Fatal error in PMPI_Comm_split: Other MPI error, error stack:
Unexpected output in attric: PMPI_Comm_split(507): MPI_Comm_split(comm=0x84000005, color=0, key=0, new_comm=0x7ffdec16bdf8) failed
Unexpected output in attric: PMPI_Comm_split(489): 
Unexpected output in attric: (unknown)(): Other MPI error
Unexpected output in attric: Fatal error in PMPI_Comm_split: Other MPI error, error stack:
Unexpected output in attric: PMPI_Comm_split(507): MPI_Comm_split(comm=0x84000006, color=0, key=1, new_comm=0x7fff601b2278) failed
Unexpected output in attric: PMPI_Comm_split(489): 
Unexpected output in attric: (unknown)(): Other MPI error
Unexpected output in attric: Fatal error in PMPI_Comm_split: Other MPI error, error stack:
Unexpected output in attric: PMPI_Comm_split(507): MPI_Comm_split(comm=0x84000005, color=0, key=0, new_comm=0x7ffc4dbaaca8) failed
Unexpected output in attric: PMPI_Comm_split(489): 
Unexpected output in attric: (unknown)(): Other MPI error
Unexpected output in attric: Fatal error in PMPI_Comm_split: Other MPI error, error stack:
Unexpected output in attric: PMPI_Comm_split(507): MPI_Comm_split(comm=0x84000003, color=0, key=1, new_comm=0x7fff3d8cb048) failed
Unexpected output in attric: PMPI_Comm_split(489): 
Unexpected output in attric: (unknown)(): Other MPI error
```

### Full Logs

Full fault injection logs for all of the bugs reports in MPICH and MVAPICH
are available 
[here](http://web.cs.ucdavis.edu/~rubio/includes/mpi-error-prop-artifact-logs.tgz). This file is 22G uncompressed.

A quick sanity check of the claim that 60% of the tests crashed during 
fault injection can be done using the following command, after uncompressing
the full set of fault injection logs. This counts the unique test names
that resulted in a Segmentation fault. There are a total of 1137 tests
in MPICH.

```
cd fault-injection
grep -r 'Segmentation fault' mpich-* \
    | grep 'Unexpected output' \
    | cut -d':' -f2 \
    | cut -d' ' -f4 \
    | sort -u \
    | wc
```

## 4. Consequences for Real MPI Programs

Section 4.2 also contains an examination of the consequences of error code
propagation bugs for the real MPI programs `Kripke`, `miniAMR`, and `miniFE`.

### Kripke

The bug report defined by the dropping site / origin site pair 
(`src/mpi/comm/comm_split.c:368`, `src/mpi/comm/commutil.c:247`) will result
in a segmentation fault when it is activated by the application Kripke.

To see the behavior of Kripke run:

(This command takes about 10 minutes to run)
```
docker run --rm ucdavisplse/mpi-error-prop-artifact fault-injection \
    kripke-example src/mpi/comm/comm_split.c:368 src/mpi/comm/commutil.c:247 \
    nomem none kripke mpich 0 \
    | tee kripke-example.log.txt
```

This can be compared with behavior after a partial propagation fix is applied:

(This command atkes about 10 minutes to run)
```
docker run --rm ucdavisplse/mpi-error-prop-artifact fault-injection \
    kripke-example-return-none \
    src/mpi/comm/comm_split.c:368 src/mpi/comm/commutil.c:247 \
    nomem return kripke mpich 0
```

The above command should show that the memory allocation failure is detected 
after the application of the partial propagation fix.

### MiniAMR

The same bug that crashes Kripke will also crash miniAMR.

To see the behavior of miniAMR under fault injection without a fix, run:

(This command takes about 10 minutes to run)
```
docker run --rm ucdavisplse/mpi-error-prop-artifact fault-injection \
    miniamr-example src/mpi/comm/comm_split.c:368 src/mpi/comm/commutil.c:247 \
    nomem none miniamr mpich 0 \
    | tee miniamr-example.log.txt
```

The above command should produce a segmentation fault when miniAMR runs.

This can be compared with behavior after a partial propagation fix is applied:

(This command takes about 10 minutes to run)
```
docker run --rm ucdavisplse/mpi-error-prop-artifact fault-injection \
    miniamr-example-return-none \
    src/mpi/comm/comm_split.c:368 src/mpi/comm/commutil.c:247 \
    nomem return miniamr mpich 0
```

The above command should show that the out of memory erorr is detected 
after the application of the partial propagation fix.

### MiniFE

To see the behavior of miniAMR under fault injection without a fix, run:

(This command takes about 10 minutes to run)
```
docker run --rm ucdavisplse/mpi-error-prop-artifact fault-injection \
    minife-example \
    src/mpi_t/mpit_initthread.c:51 src/util/cvar/mpir_cvars.c:1515 \
    return none minife mpich 0 \
    | tee minife-example.log.txt
```

The above command should produce a segmentation fault.
