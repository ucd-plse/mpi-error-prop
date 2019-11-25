#!/bin/bash

# This script attempts to apply a set of fixes to a library and generates 
# a bitcode file that is suitable for analysis by MPIErrorProp.

# Name is an overall name to assign to this run
NAME="$1"

PHASELINES="$2"

# Should be mpich or mvapich
LIBNAME="$3"

# Dropped is a single dropping site. 
DROPPED="$4"
ORIGIN="$5"

if [ -z "$NAME" ]; then
    echo "Missing name"
    exit 1
fi
if [ -z "$LIBNAME" ]; then 
    echo "Missing LIBNAME"
    exit 1
fi

# Setup directory to store results
if [ "$LIBNAME" == "mpich" ]; then
    cp -r /implementations/mpich-3.3 /d/${LIBNAME}-${NAME}-none-return
elif [ "$LIBNAME" == "mvapich" ]; then
    pushd /d
    cp -r /implementations/mvapich2-2.3.1.tar.gz .
    tar xf mvapich2-2.3.1.tar.gz
    mv mvapich2-2.3.1 ${LIBNAME}-${NAME}-none-return
    rm /d/mvapich2-2.3.1.tar.gz
    patch ${LIBNAME}-${NAME}-none-return/src/include/mpi.h.in < /implementations/mvapich_errorcodes.patch
    patch ${LIBNAME}-${NAME}-none-return/src/mpi/errhan/errutil.c < /implementations/mvapich_create_code.patch
    popd
fi

cp ${PHASELINES} /d/${NAME}.txt
sed -i 's/$/ none return/' /d/${NAME}.txt
if [ -n "$DROPPED" ]; then
    echo "$DROPPED $ORIGIN none return" >> /d/${NAME}.txt
fi
LINE_INPUTS="/d/${NAME}.txt"

# Apply fix
echo ./inject_lines.py /d/${LIBNAME}-${NAME}-none-return ${LINE_INPUTS} 
/scripts/injection/inject_lines.py /d/${LIBNAME}-${NAME}-none-return ${LINE_INPUTS} 

# Recompile library 
cd /d/${LIBNAME}-${NAME}-none-return

if [ "$LIBNAME" == "mpich" ]; then
    ./configure CC=gclang CFLAGS="-O0 -g" CXXFLAGS="-O0 -g" --prefix=/d/${LIBNAME}-install-${NAME}-none-return --enable-fortran=no --enable-shared=yes --enable-static=yes
elif [ "$LIBNAME" == "mvapich" ]; then
    ./configure CC=gclang --disable-fast --prefix=/d/${LIBNAME}-install-${NAME}-none-return MPICHLIB_CFLAGS=-O0 MPICHLIB_FFLAGS=-O0 MPICHLIB_CXXFLAGS=-O0 MPICHLIB_FCFLAGS=-O0 CFLAGS="-O0 -g" CXXFLAGS="-O0 -g" --enable-fortran=no --enable-shared=yes --enable-static=yes --disable-mcast
fi

make -j$(nproc) 2>&1 | tee /d/compile.log
make install
get-bc -b /d/${LIBNAME}-install-${NAME}-none-return/lib/libmpi.so

if [ $? -ne 0 ]; then
    echo "Failed to apply fix"
    exit 1
fi
