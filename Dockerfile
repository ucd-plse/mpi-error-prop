FROM ubuntu:xenial

RUN apt-get update \
    && apt-get install -y python3-pip wget vim-nox git autoconf \
    libyaml-tiny-perl libtool libibverbs-dev bison \
    llvm-3.6 clang-3.6 golang scons build-essential ocaml libxml2-utils \
    libboost1.58-dev byacc unzip sqlite3 libsqlite3-dev libconfig++-dev curl \
 	&& ln -s /usr/bin/clang-3.6 /usr/bin/clang \
 	&& ln -s /usr/lib/llvm-3.6/bin/clang++  /usr/bin/clang++ \
	&& ln -s /usr/bin/llvm-config-3.6 /usr/bin/llvm-config \
	&& ln -s /usr/bin/llvm-link-3.6 /usr/bin/llvm-link \
	&& export GOPATH="/opt/go" \
	&& go get github.com/SRI-CSL/gllvm/cmd/... \
    && echo 'PATH="$PATH:/opt/go/bin"' >> /root/.bashrc

RUN mkdir /implementations

# Kripke 
RUN cd /implementations \
    && wget https://github.com/Kitware/CMake/releases/download/v3.14.3/cmake-3.14.3.tar.gz \
    && tar xf cmake-3.14.3.tar.gz \
    && cd cmake-3.14.3 \
    && ./bootstrap -- -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr \
    && make -j$(nproc) \
    && make install \
    && cd .. \
    && rm cmake-3.14.3.tar.gz \
    && rm -fr cmake-3.14.3  \
    && git clone https://github.com/LLNL/Kripke \
    && cd Kripke \
    && git submodule update --init --recursive 

COPY src /mpi-error-prop
RUN cd /mpi-error-prop \
    && scons -c \
    && scons -j $(nproc)

RUN cd /implementations \
    && wget http://www.mpich.org/static/downloads/3.3/mpich-3.3.tar.gz 

COPY ./error-codes /mpi-error-prop/error-codes
COPY ./mpich-3.3 /implementations/mpich-3.3
COPY ./mvapich2-2.3.1.tar.gz /implementations/mvapich2-2.3.1.tar.gz
COPY ./mvapich_errorcodes.patch /implementations
COPY ./mvapich_create_code.patch /implementations
COPY ./scripts /scripts
COPY ./results /results
RUN chmod +x /scripts/injection/*
RUN chmod +x /scripts/*.sh
RUN mkdir /d

WORKDIR /scripts/injection

ENTRYPOINT [ "/scripts/entrypoint.sh" ]
