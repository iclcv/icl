/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/FileWriterPluginImageMagick.h            **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_FILE_WRITER_PLUGIN_IMAGEMAGICK_H
#define ICL_FILE_WRITER_PLUGIN_IMAGEMAGICK_H

#include <ICLIO/FileWriterPlugin.h>

namespace icl{
  /// Interface class for writing images using an ImageMagick++ wrapper  \ingroup FILEIO_G
  /** ImageMagick provides reading and writing routines for many file formats: The following
      list shows all formats currently implemented for ICL's FileWriter.
      Notify ICL's support if a common format, supported by ImageMagick is missing!
      
      <pre>
      "png", "gif", "pdf",  "ps",  "avs", "bmp", "cgm",  "cin",   "cur",  "cut", "dcx",
      "dib", "dng", "dot",  "dpx", "emf", "epdf","epi",  "eps",   "eps2", "eps3",
      "epsf","epsi","ept",  "fax", "gplt","gray","hpgl", "html",  "ico",  "info",
      "jbig","jng", "jp2",  "jpc", "man", "mat", "miff", "mono",  "mng",  "mpeg","m2v",
      "mpc", "msl", "mtv",  "mvg", "palm","pbm", "pcd",  "pcds",  "pcl",  "pcx", "pdb",
      "pfa", "pfb", "picon","pict","pix", "ps",  "ps2",  "ps3",   "psd",  "ptif","pwp",
      "rad", "rgb", "pgba", "rla", "rle", "sct", "sfw",  "sgi",   "shtml","sun", "svg",
      "tga", "tiff","tim",  "ttf", "txt", "uil", "uyuv", "vicar", "viff", "wbmp",
      "wmf", "wpg", "xbm",  "xcf", "xpm", "xwd", "ydbcr","ycbcra","yuv"
      </pre>
      

      Use the following linux shell command for a list of supported formats of your 
      ImageMagick library:
      <pre>
      > identify -list format
      </pre>
      
  */
  class FileWriterPluginImageMagick : public FileWriterPlugin{
    public:
    /// creates a plugin
    FileWriterPluginImageMagick();
    
    /// Destructor
    virtual ~FileWriterPluginImageMagick();
    
    /// pure virtual writing function
    virtual void write(File &file, const ImgBase *image);

    /// InternalData storage class
    class InternalData;

    private:
    /// Pointer to internal data storage
    InternalData *m_data;
  };
}
#endif
