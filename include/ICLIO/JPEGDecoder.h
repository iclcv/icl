/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLIO/JPEGDecoder.h                            **
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

#ifndef ICL_JPEG_DECODER_H
#define ICL_JPEG_DECODER_H

#include <ICLIO/File.h>
#include <ICLCore/Types.h>
#include <ICLUtils/Exception.h>

namespace icl{
  /// Utility class for decoding JPEG-Data streams (with HAVE_LIBJPEG only)
  class JPEGDecoder{
    public:
    /// Decode JPEG-File (E.g. used for FileGrabberPluginJPEG)
    /** @param file must be opened in mode readBinary or not opend 
        @param destination image, which is adapted to the found image parameters
    */
    static void decode(File &file, ImgBase **dst) throw (InvalidFileFormatException);
    
    /// Decode a data stream (E.g. used for Decoding Motion-JPEG streams in unicap's DefaultConvertEngine)
    /** @param data jpeg data stream (must be valid, otherwise unpredictable behaviour occurs
        @param maxDataLen length of the given data pointer
                          <b>Note:</b>This is just an upper limit to avoid segmentation faults on
                          corrupted jpeg data (e.g. end-of-image-marker is missing). The given data
                          pointer can be much longer then the actual jpeg data. If that is the case,
                          libjpeg obviously reads only necessary bytes.
        @param dst destination image, which is adapted to the found images parameters */
    static void decode(const unsigned char *data,unsigned int maxDataLen,ImgBase **dst);

    private:
    /// internal utility function, which does all the work
    static void decode_internal(File *file,const unsigned char *data, 
                                unsigned int maxDataLen, ImgBase **dst) throw (InvalidFileFormatException);
  };
}

#endif // GUARD

