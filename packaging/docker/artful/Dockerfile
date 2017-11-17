FROM ubuntu:artful

LABEL description="Ubuntu Artful ICL Build Environment"

RUN apt-get update
RUN apt-get upgrade -y

# add intel repositories
ADD https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB /root/
ADD https://apt.repos.intel.com/setup/intelproducts.list /etc/apt/sources.list.d/
RUN apt-key add /root/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
RUN apt-get install -y ca-certificates
RUN apt-get update || echo 1

RUN apt-get install -y build-essential git cmake python-pip dirmngr \
                       devscripts equivs apt-file debhelper sudo
RUN apt-get clean

RUN useradd --create-home -s /bin/bash user
RUN echo user:user | chpasswd
RUN adduser user sudo
RUN echo "user ALL=(ALL:ALL) NOPASSWD: /usr/bin/mk-build-deps" >> /etc/sudoers

WORKDIR /home/user
USER user
CMD ["workspace/icl/packaging/scripts/build-ubuntu-packages.sh"]
