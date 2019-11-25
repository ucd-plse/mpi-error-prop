#!/bin/bash

echo "[inject.sh]: $1 $2 $3"

NAME="$1"
LINE_INPUTS="$2"
LIBRARY="$3"
TEST_DIR="/d/${NAME}"
INSTALL_DIR="/d/install-${NAME}"

if [ -z "$NAME" ]; then
	echo "Must pass run name"
	exit 3
fi
if [ -z "$LINE_INPUTS" ]; then
	echo "Must pass inputs file"
	exit 3
fi

export PATH="$PATH:$INSTALL_DIR"

rm -fr $TEST_DIR
pushd /d

if [ "$LIBRARY" = "mpich" ]; then
	SRCFILE="mpich-3.3"
elif [ "$LIBRARY" = "mvapich" ]; then
	SRCFILE="mvapich2-2.3.1"
fi

cp /implementations/"$SRCFILE".tar.gz .
tar xf "$SRCFILE".tar.gz
mv "$SRCFILE" $TEST_DIR

popd
./inject_lines.py $TEST_DIR $LINE_INPUTS
