Installing MONAA
================

MacOS
-----

On MacOS, you can install MONAA using [Homebrew](https://brew.sh).

1. First, you have to setup Homebrew if you have not set it up yet.

```bash
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

2. Then, you can install MONAA using Homebrew :)

```bash
brew install maswag/scientific/monaa
```

If you want to use the latest (unreleased) version, you need `--HEAD`

```bash
brew install --HEAD maswag/scientific/monaa
```

Arch Linux
----------

1. Install the required packages

```bash
pacman -S --needed base-devel make cmake bison flex eigen boost doxygen git
```

2. Download the MONAA source

```bash
git clone https://github.com/MasWag/monaa.git
```

3. Build MONAA

```bash
cd monaa
mkdir build
cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make && make install
```

Ubuntu 18.04 (bionics)
----------------------

1. Install the required packages

```bash
apt-get install build-essential make cmake bison flex libeigen3-dev libboost-all-dev doxygen git
```

2. Download the MONAA source

```bash
git clone https://github.com/MasWag/monaa.git
```

3. Build MONAA

```bash
cd monaa
mkdir build
cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make && make install
```

Docker
------


1. Install [docker](https://www.docker.com/)

2. pull the docker image

```bash
docker pull maswag/monaa
```

[Docker Hub page](https://cloud.docker.com/u/maswag/repository/docker/maswag/monaa)
