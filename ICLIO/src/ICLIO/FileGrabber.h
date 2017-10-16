/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FileGrabber.h                          **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLIO/Grabber.h>
#include <ICLIO/FileGrabberPlugin.h>

namespace icl{
  namespace io{

    /// Grabber implementation to grab from files \ingroup FILEIO_G \ingroup GRABBER_G
    /** This implementation of a file grabber class provides an internally used
        and expendable plugin-based interface for reading files of different types.

        \section EX Example
        \code
        FileGrabber g("image/image*.jpg"); //
        while(true){
          const core::ImgBase *image = g.grab();
          ...
        }
        \endcode
    **/
    class ICLIO_API FileGrabber : public Grabber {
      public:

        /// for the internal plugin concept
        friend class FileGrabberPluginMapInitializer;

        /// Create a NULL FileGrabber
        FileGrabber();

        /// Create a file grabber with given pattern and parameters
        /** \section BUF Buffering
          Images are buffered internally using the original image core::format i.e. most images are
          stored as icl8u images. At first during the grab(..) call, the images are converted
          automatically to the desired parameters (dependent on the desired params flag)

          @param pattern file pattern (e.g. images/ *.jpg ...)
          @param buffer flag to determine, whether images should be pre-buffered. If the flag is
                        set to true, images are buffered immediately in the constructor. If
                        exceptions that occur during this buffering procedure are <b>not</b>
                        caught internally (i.e. these exceptions are thrown). This is necessary
                        because, the FileGrabber can become inconsistent in this case, which
                        must be reported explicitly using an exception.\n

                        To pre-buffer images in a error-tolerant way, use the constructor without
                        a set buffer flag and call bufferImages(false) after the FileGrabber
                        instantiation.
          @param ignoreDesiredParams In some cases, grabbed images should not be adapted to the
                                     given parameters using a converter, but the original image
                                     parameters stored in the file header should be restored. In
                                     this case the ignoreDesiredParams flag must be set to true
                                     in the constructor or by using the setIgnoreDesiredParams(bool)
                                     function. To ensure compatibility to other grabbers, the
                                     "ignore desired params" flag is set to false by default.

          */
        FileGrabber(const std::string &pattern, bool buffer=false, bool ignoreDesiredParams=false)
        throw(utils::FileNotFoundException);

        /// Destructor
        virtual ~FileGrabber();

        /// grab implementation
        virtual const core::ImgBase *acquireImage();

        /// returns the count of files that are available
        unsigned int getFileCount() const;

        /// returns the next to-be-grabbed file name
        const std::string &getNextFileName() const;

        /// pre-buffers all images internally
        /** This function is called automatically,if the "buffer"-flag of the constructor is set.
          The function internally pre-buffers all images, that are contained in the current
          FileList. During this procedure, some exception may occur:
          - <b>FileNotFoundException</b>, if a file list entry references a non-existing file
          - <b>InvalidFileException</b>, if no FileGrabberPlugin can be found to read files
            of the given core::format (determined by the file postfix e.g. ".icl" or ".ppm")
          - <b>InvalidFileFormatException</b>, if the FileGrabberPlugin encounters a file section
            with invalid contents (e.g. if a file header is corrupted, or if the file contains
            not enough data elements to fill an image with size determined by the file header)
          - <b>ICLException</b> some "not-more-specified" errors (e.g if a file could not be read
            due to missing permissions)

          To skip these exceptions by just leaving out files that could not be read by any reason,
          the omitExceptions flag can be set (default). In this case, exceptions mentioned above
          are caught implicitly. \n
          However if not even a single file could be read, a FileNotFoundException is thrown, to indicate,
          that the FileGrabber's grab/prev and next function will not work properly. \n

          To indicate which files could be buffered, a reference to the internal file list is
          returned.
          */
        void bufferImages(bool omitExceptions=true);

        /// internally skips the next to-be-grabbed image
        /** E.g. if the image directory "images/" contains 6 valid ".jpg"-files,
          the following code grabs all images with even indices (0,2, and 4)
          \code
          FileGrabber g("images/ *.jpg");
          for(int i=0;i<g.getFileCount()/2;++i){
            g.grab();
            g.next();
          }
          \endcode
      */
        void next();

        /// internally sets the next-image pointer back
        /** \code
          FileGrabber g(...);
          g.grab(); // grabs image A
          g.grab(); // grabs image B
          g.prev();
          g.grab(); // again grabs image B
          \endcode
      */
        void prev();


        /// forces the filegrabber to use a plugin for the given suffix
        /** suffix must be something like png or csv (without a trailing .)
          By default, the forced plugin type string is "". In this case,
          the filename suffix is evaluated to determine the appropriate
          grabber plugin.
      */
        void forcePluginType(const std::string &suffix);

      private:
        /// grab implementation called bz acquireImage().
        const core::ImgBase *grabImage();
        /// adds FileGrabbers properties to Configurable.
        void addProperties();
        /// callback function for property changes.
        void processPropertyChange(const utils::Configurable::Property &p);
        /// updates properties values.
        void updateProperties(const core::ImgBase* img);

        struct Data;
        Data *m_data;
        utils::Mutex m_propertyMutex;
        bool m_updatingProperties;
    };

  } // namespace io
}

