#!/bin/bash

echo "[fault-injection.sh]: $1 $2 $3 $4 $5 $6 $7 $8"

NAME="$1"
DROPPED="$2"
ORIGIN="$3"
INJECTION_STRATEGY="$4"
FIX_STRATEGY="$5"
TARGET="$6"
LIBRARY="$7"
PHASE="$8"
EXPECT_CRASH="$9"

readonly USAGE="driver.sh NAME DROPPED ORIGIN INJECTION_STRATEGY FIX_STRATEGY TARGET LIBRARY PHASE"

if [ -z "$NAME" ]; then
	echo "Must pass run name"
	echo $USAGE
	exit 3
fi
if [ -z "$DROPPED" ]; then
	echo "Must pass line number of dropped site"
	echo $USAGE
	exit 3
fi
if [ -z "$ORIGIN" ]; then
	echo "Must pass line number of origin site"
	echo $USAGE
	exit 3
fi
if [ -z "$INJECTION_STRATEGY" ]; then
	echo "Must pass injection strategy"
	echo $USAGE
	exit 3
fi
if [ -z "$FIX_STRATEGY" ]; then
	echo "Must pass fix strategy"
	echo $USAGE
	exit 3
fi
if [ -z "$TARGET" ]; then
	echo $USAGE
	exit 3
fi
if [ -z "$LIBRARY" ]; then
	echo "Must pass library 'mpich' or 'mvapich'"
	echo $USAGE
	exit 3
fi
if [ -z "$PHASE" ]; then
	echo "Must pass phase"
	echo $USAGE
	exit 3
fi

mkdir -p /d

# The list of fixes to be applied
# Apply all fixes from the current phase plus the one fix passed in args
# The fixes from the current phase are applied because that is what is 
# required to expose the bug reports for the current phase
cp /results/${LIBRARY}_phase${PHASE}_fixes.txt /d/${NAME}.txt
sed -i 's/$/ none return/' /d/${NAME}.txt
echo "$DROPPED $ORIGIN $INJECTION_STRATEGY $FIX_STRATEGY" >> /d/${NAME}.txt
LINE_INPUTS="/d/${NAME}.txt"

# Perform source code injection
./inject.sh ${NAME} ${LINE_INPUTS} ${LIBRARY}

./test.sh ${TARGET} ${NAME} ${LIBRARY} 2>&1 | tee /d/${NAME}.log

CRASHED="grep Segmentation /d/${NAME}.log"
if [ "$EXPECT_CRASH" == "1" ]; then
	if [ -z "$CRASHED" ]; then
		echo "Expected crash, but no crash found."
		exit 1
	else
		echo "Expected crash, and crash found."
		exit 0
	fi
elif [ "$EXPECT_CRASH" == "0" ]; then
	if [ -z "$CRASHED" ]; then
		echo "Did not expect crash, and no crash found."
		exit 0
	else
		echo "Did not expect crash, and crash found."
		exit 1
	fi
fi

exit 0
