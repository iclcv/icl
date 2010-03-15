/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLIO/FileGrabberPlugin.h                      **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#ifndef ICL_FILE_GRABBER_PLUGIN_H
#define ICL_FILE_GRABBER_PLUGIN_H

#include <ICLIO/File.h>
#include <ICLCore/Img.h>

namespace icl{
  /// interface for ImageGrabber Plugins for reading different file types \ingroup FILEIO_G
  class FileGrabberPlugin{
    public:
#ifdef HAVE_LIBJPEG
    friend class JPEGDecoder;
#endif

    virtual ~FileGrabberPlugin() {}
    /// pure virtual grab function
    virtual void grab(File &file, ImgBase **dest)=0;

    protected:
    /// Internally used collection of image parameters
    struct HeaderInfo{
      format imageFormat; ///!< format
      depth imageDepth;   ///!< depth
      Rect roi;           ///!< roi rect
      Time time;          ///!< time stamp
      Size size;          ///!< image size
      int channelCount;   ///!< image channel count
      int imageCount;     ///!< image count (e.g. when writing color images as .pgm gray-scale images)
    };
  };  
}

#endif
