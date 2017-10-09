/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FileWriterPluginCSV.h                  **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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
#include <ICLIO/FileWriterPlugin.h>

namespace icl{
  namespace io{

    /// Writer plugins for ".csv"-files (<b>Comma</b>-<b>Separated</b> <b>Values</b>) \ingroup FILEIO_G
    class ICLIO_API FileWriterPluginCSV : public FileWriterPlugin{
      public:

      /// write implementation
      virtual void write(utils::File &file, const core::ImgBase *image);

      /// static feature adaption function
      /** if the flag is set to true, the writer will encode image
          properties by extending the given filename
          @see FileGrabberPluginCSV
      **/
      static void setExtendFileName(bool value);

      private:
      /// static flag
      static bool s_bExtendFileName;
    };
  } // namespace io
}
