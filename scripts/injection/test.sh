#!/bin/bash

TARGET="$1"
NAME="$2"
LIBRARY="$3"
TEST_DIR="/d/${NAME}"
INSTALL_DIR="/d/install-${NAME}"

if [ -z "${NAME}" ]; then
    echo "Must pass run name"
    exit 3
fi
if [ -z "$TARGET" ]; then
    echo "Must pass target"
    exit 3
fi

export PATH="$PATH:$INSTALL_DIR"

pushd $TEST_DIR

if [ "$LIBRARY" = "mpich" ]; then
    CONFIGFLAGS="--disable-fortran"
elif [ "$LIBRARY" = "mvapich" ]; then
    CONFIGFLAGS="--disable-fortran --disable-mcast"
fi

if [ "$TARGET" = "testing" ]; then
    CONFIGFLAGS="$CONFIGFLAGS --prefix=$INSTALL_DIR"
fi

./configure $CONFIGFLAGS

popd
cp mpi_"$LIBRARY".h $TEST_DIR/src/include/mpi.h
cp init_"$LIBRARY".c $TEST_DIR/src/mpi/init/init.c
cd $TEST_DIR
make -j$(nproc)

rm -fr $INSTALL_DIR
mkdir $INSTALL_DIR
make install

if [ "$LIBRARY" = "mvapich" ]; then
    sed -ri 's/(export NOXMLCLOSE) && (cd mpi)/\1 \&\& MV2_ENABLE_AFFINITY=0 \&\& export MV2_ENABLE_AFFINITY \&\& \2/g' $TEST_DIR/test/Makefile
fi

if [ "$TARGET" = "testing" ]; then
    if [ ! -f "/d/install-${NAME}/bin/mpicc" ]; then
        echo "Compile failed"
        exit 2
    fi

    export MPITEST_THREADLEVEL_DEFAULT="single"
    export MV2_SMP_USE_CMA=0
    make testing
elif [ "$TARGET" = "kripke" ]; then
    if [ ! -f "/usr/local/bin/mpicc" ]; then
        echo "compile failed"
        exit 2
    fi

    cd /implementations/Kripke
    mkdir build && cd build
    cmake .. -DENABLE_MPI=1 -DENABLE_MPI_WRAPPER=1
    make -j$(nproc)
    bin/kripke.exe
elif [ "$TARGET" = "miniamr" ]; then
    if [ ! -f "/usr/local/bin/mpicc" ]; then
        echo "compile failed"
        exit 2
    fi

    cd /d
    git clone https://github.com/Mantevo/miniAMR
    cd miniAMR/ref
    sed -i 's/^CC.*/CC = mpicc/' Makefile
    sed -i 's/^LDLIBS.*/LDLIBS = -lm -lmpi/' Makefile
    make
    export LD_LIBRARY_PATH="/usr/local/lib"
    ./ma.x
elif [ "$TARGET" = "minife" ]; then
    if [ ! -f "/usr/local/bin/mpicc" ]; then
        echo "compile failed"
        exit 2
    fi

    cd /d
    git clone https://github.com/Mantevo/miniFE
    cd miniFE/ref/src
    make
    export LD_LIBRARY_PATH="/usr/local/lib"
    mpirun -np 4 ./miniFE.x
fi
