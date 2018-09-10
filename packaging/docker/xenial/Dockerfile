FROM ubuntu:xenial

LABEL description="Ubuntu Xenial ICL Build Environment"
ENV USER=user USER_ID=1000 USER_GID=1000


RUN apt-get update
RUN apt-get upgrade -y

RUN apt-get install -y build-essential git cmake python-pip dirmngr \
                       devscripts equivs apt-file debhelper sudo wget \
                       libjpeg-dev libpng-dev clang

RUN wget -q -O /tmp/cmake.tar.gz --no-check-certificate \
  https://cmake.org/files/v3.12/cmake-3.12.1-Linux-x86_64.tar.gz && \
  tar -xaf /tmp/cmake.tar.gz --strip-components=1 -C /usr/local && \
  rm /tmp/cmake.tar.gz

RUN apt-get clean

RUN groupadd --gid ${USER_GID} ${USER}
RUN useradd --uid ${USER_ID} --gid ${USER_GID} --create-home --shell /bin/bash ${USER}

WORKDIR /home/user
CMD ["workspace/packaging/scripts/docker-bootstrap.sh"]
