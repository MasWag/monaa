FROM alpine:3.20.0
LABEL maintainer="Masaki Waga <masakiwaga@gmail.com>"

# Install depending packages
RUN apk add --no-cache g++=13.2.1_git20240309-r0 make=4.4.1-r2 cmake=3.30.5-r0 bison=3.8.2-r1 flex-dev=2.6.4-r6 eigen-dev=3.4.0-r10 boost-dev=1.84.0-r1 git=2.46.2-r0	

# Download cmake configuration for eigen
RUN wget -q https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.bz2 && tar xvf eigen-3.4.0.tar.bz2 && mv eigen-3.4.0 eigen

# Download the MONAA source
RUN git clone https://github.com/MasWag/monaa.git

# Build MONAA
RUN mkdir -p monaa/build 
WORKDIR /monaa/build
RUN cmake -DCMAKE_MODULE_PATH=/eigen/cmake -DCMAKE_BUILD_TYPE=Release .. && make monaa && make install

# Uninstall build only dependencies
WORKDIR /
RUN apk del --no-cache make cmake bison flex eigen-dev doxygen git && rm -rf /monaa /eigen /eigen-3.4.0.tar.bz2

ENTRYPOINT ["/usr/local/bin/monaa"]
