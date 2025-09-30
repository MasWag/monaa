FROM alpine:3.22
LABEL maintainer="Masaki Waga <masakiwaga@gmail.com>"

# Install depending packages
RUN apk add --no-cache g++=14.2.0-r6 \
    make=4.4.1-r3 \
    cmake=3.31.7-r1 \
    bison=3.8.2-r2 \
    flex-dev=2.6.4-r7 \
    eigen-dev=3.4.0-r10 \
    boost-dev=1.84.0-r3 \
    wget=1.25.0-r1 \
    git=2.49.1-r0

# Download cmake configuration for eigen
RUN wget -q https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.bz2 && tar xf eigen-3.4.0.tar.bz2 && mv eigen-3.4.0 eigen

# Clone the MONAA source
RUN  git clone https://github.com/MasWag/monaa.git

# Build MONAA
RUN mkdir -p /monaa/build
WORKDIR /monaa/build
RUN cmake -DCMAKE_MODULE_PATH=/eigen/cmake -DCMAKE_BUILD_TYPE=Release .. && make monaa && make install

# Uninstall build only dependencies
WORKDIR /
RUN apk del --no-cache make \
    cmake \
    bison \
    flex-dev \
    eigen-dev \
    git && \
    rm -rf /eigen /eigen-3.4.0.tar.bz2

ENTRYPOINT ["/usr/local/bin/monaa"]
