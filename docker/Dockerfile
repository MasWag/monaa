FROM alpine:3.10.2
MAINTAINER "Masaki Waga" <masakiwaga@gmail.com>

# Install depending packages
RUN apk add --no-cache g++ make cmake bison flex-dev eigen-dev boost-dev doxygen git 

# Download cmake configuration for eigen
RUN wget http://bitbucket.org/eigen/eigen/get/3.3.7.tar.gz && tar xvf 3.3.7.tar.gz && mv eigen-* eigen

# Download the MONAA source
RUN git clone https://github.com/MasWag/monaa.git

# Build MONAA
RUN mkdir -p monaa/build && cd monaa/build && cmake -DCMAKE_MODULE_PATH=/eigen/cmake -DCMAKE_BUILD_TYPE=Release .. && make monaa && make install

# Install build only dependencies
RUN apk del --no-cache make cmake bison flex eigen-dev doxygen git
RUN rm -rf /monaa /eigen

ENTRYPOINT ["/usr/local/bin/monaa"]