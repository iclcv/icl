.. include:: ../js.rst

.. _download:

#####################
Download Instructions
#####################

Source Code
"""""""""""

The ICL's source code can be found on Github: `iclcv/icl <https://github.com/iclcv/icl>`_.


Ubuntu Packages
"""""""""""""""

Ubuntu packages can be obtained from the `Github releases <https://github.com/iclcv/icl/releases>`_
page or by using the `ICL PPA <(https://launchpad.net/~iclcv/+archive/ubuntu/icl>`_::

    sudo add-apt-repository ppa:iclcv/icl
    sudo apt-get update
    sudo apt-get install icl-tools

The ICL build is divided into four packages:

* ``icl-core`` - Image Component Library
* ``icl-dev`` - ICL headers and project creation tools
* ``icl-doc`` - ICL sphinx manual and api documentation
* ``icl-tools`` - ICL Applications

Currently, we provide packages for the Ubuntu releases ``trusty``, 
``xenial`` and ``artful``.
The packages are built with `PCL <http://pointclouds.org>`_ features enabled.
Users of ``trusty`` need to add a third party PPA to their system as described `here <http://pointclouds.org/downloads/linux.html>`_::

    sudo add-apt-repository ppa:v-launchpad-jochen-sprickerhof-de/pcl
    sudo apt-get update
    # installing icl-core will automatically pull this dependency
    sudo apt-get install libpcl-all


Homebrew Recipes
""""""""""""""""

We provide `Homebrew <https://brew.sh/index_de.html>`_ recipes which can be used
to install ICL in three different flavours::

    # has to be done just once
    $ brew tap iclcv/homebrew-formulas
    # enables features of OpenCV, Qt, OpenGL, ImageMagick, LibAV, LibDC and LibEigen
    $ brew install icl  
    # base features plus additional functionality based on rsb, protobuf, freenect, pcl and bullet
    $ brew install icl --with-extra
    # extra features plus libusb, zmq and openni support 
    brew install icl --with-full


Windows Installers
""""""""""""""""""

Windows MSI installers for Win32 or Win64 can be downloaded from `Github releases <https://github.com/iclcv/icl/releases>`_ or from the `AppVeyor CI-Servers <https://ci.appveyor.com/project/aleneum/icl>`_.
Users to have to install 3rd-party dependencies (pthreads-win32, Qt5, Visual Studio Redistributables)
to work with the contained binaries.
