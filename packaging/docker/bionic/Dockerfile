FROM ubuntu:bionic

LABEL description="Ubuntu Bionic ICL Build Environment"
ARG EXTRA_PACKAGES

ENV TZ=Europe/Berlin
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update
RUN apt-get upgrade -y

# add intel repositories
RUN apt-get update && apt-get install -y ca-certificates cmake dirmngr devscripts equivs debhelper

ADD https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB /root/
RUN apt-key add /root/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB
RUN sh -c 'echo deb https://apt.repos.intel.com/mkl all main > /etc/apt/sources.list.d/intel-mkl.list'
RUN sh -c 'echo deb https://apt.repos.intel.com/ipp all main > /etc/apt/sources.list.d/intel-ipp.list'

RUN apt-get update && apt-get install -y cmake libjpeg-dev libpng-dev ocl-icd-opencl-dev opencl-headers \
	libeigen3-dev libopenni-dev libopencv-dev libmagick++-dev qtbase5-dev qtmultimedia5-dev libqt5opengl5-dev libfreenect-dev \
	libpcl-dev libzmq3-dev libglew-dev libusb-dev libavcodec-dev libavresample-dev libavdevice-dev doxygen python-sphinx \
	python-pyparsing libbullet-dev lcov ${EXTRA_PACKAGES}

RUN useradd --create-home -s /bin/bash user
RUN echo user:user | chpasswd

WORKDIR /home/user
USER user
CMD ["workspace/icl/packaging/scripts/ubuntu-build.sh"]
