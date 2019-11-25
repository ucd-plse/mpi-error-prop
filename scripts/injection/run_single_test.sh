#!/bin/bash
#
# Script runs all tests for given TESTNAME
#
# Input Parameter #1: Your local installation of mpich, pointing to the test/mpi folder
#		      Example: $HOME/mpich-3.3/test/mpi
# Input Parameter #2: string TESTNAME for which all relevant tests will be run

TOP_DIR=$1
TESTNAME=$2

# Get directory where test is stored
SRCDIR=$(dirname "$(find "$TOP_DIR" -type f -name "$TESTNAME"*.c* -o -name "$TESTNAME"*.o| head -1)")

# Copy corresponding line(s) from original testfile to a new testfile
grep "^$TESTNAME " "$SRCDIR"/testlist > "$SRCDIR"/"new-testfile"

cd "$SRCDIR"
$TOP_DIR/runtests -srcdir=. -tests=new-testfile \
	-xmlfile=$TOP_DIR/summary_"$TESTNAME".xml \
	-tapfile=$TOP_DIR/summary_"$TESTNAME".tap \
	-junitfile=$TOP_DIR/summary_"$TESTNAME".junit.xml
