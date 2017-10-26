FROM ubuntu:trusty

LABEL description="Ubuntu Trusty ICL Build Environment"

RUN apt-get update
RUN apt-get install -y software-properties-common
RUN add-apt-repository -y ppa:v-launchpad-jochen-sprickerhof-de/pcl
RUN apt-get update
RUN apt-get install -y build-essential git cmake3 libjpeg-dev libpng-dev \
    ocl-icd-opencl-dev libeigen3-dev libopenni-dev libopencv-dev \
    libmagick++-dev qtbase5-dev qtmultimedia5-dev libqt5opengl5-dev \
    libfreenect-dev \
    libpcl-all libzmq-dev libglew-dev libusb-dev libudev-dev \
    libavcodec-dev libavresample-dev libavdevice-dev doxygen \
    python-sphinx python-pyparsing debhelper devscripts

RUN useradd --create-home -s /bin/bash user
RUN echo user:user | chpasswd
RUN adduser user sudo

WORKDIR /home/user
USER user
