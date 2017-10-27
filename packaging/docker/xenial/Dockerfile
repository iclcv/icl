FROM ubuntu:xenial

LABEL description="Ubuntu Xenial ICL Build Environment"

RUN apt-get update
RUN apt-get install -y build-essential git cmake libjpeg-dev libpng-dev \
    ocl-icd-opencl-dev libeigen3-dev libopenni-dev libopencv-dev \
    libmagick++-dev qtbase5-dev qtmultimedia5-dev libqt5opengl5-dev \
    libfreenect-dev libbullet-dev \
    libpcl-dev libzmq-dev libglew-dev libusb-dev libudev-dev \
    libavcodec-dev libavresample-dev libavdevice-dev doxygen \
    python-sphinx python-pyparsing debhelper devscripts apt-file

RUN useradd --create-home -s /bin/bash user
RUN echo user:user | chpasswd
RUN adduser user sudo

WORKDIR /home/user
USER user
RUN apt-file update
#CMD ["cd workspace/icl && packaging/scripts/launchpad.sh"]
