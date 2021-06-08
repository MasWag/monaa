FROM alpine:3.10.2
LABEL maintainer "Masaki Waga <masakiwaga@gmail.com>"

# Install depending packages
RUN apk add --no-cache g++=8.3.0-r0 make=4.2.1-r2 cmake=3.14.5-r0 bison=3.3.2-r0 flex-dev=2.6.4-r2 eigen-dev=3.3.7-r0 boost-dev=1.69.0-r4 git=2.22.5-r0

# Download cmake configuration for eigen
RUN wget -q https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.tar.bz2 && tar xvf eigen-3.3.7.tar.bz2 && mv eigen-3.3.7 eigen

# Download the MONAA source
RUN git clone https://github.com/MasWag/monaa.git

# Build MONAA
RUN mkdir -p monaa/build 
WORKDIR ${HOME}/monaa/build
RUN cmake -DCMAKE_MODULE_PATH=/eigen/cmake -DCMAKE_BUILD_TYPE=Release .. && make monaa && make install

# Uninstall build only dependencies
RUN apk del --no-cache make cmake bison flex eigen-dev doxygen git && rm -rf /monaa /eigen

ENTRYPOINT ["/usr/local/bin/monaa"]
