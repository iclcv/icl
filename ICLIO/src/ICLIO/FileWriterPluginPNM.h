/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FileWriterPluginPNM.h                  **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Goetting                  **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLIO/FileWriterPlugin.h>
#include <ICLUtils/Mutex.h>
#include <vector>
#include <ICLCore/Types.h>

namespace icl{
  namespace io{
    
    /// Writer plugin to write images as ".ppm", ".pgm", ".pnm" and ".icl" \ingroup FILEIO_G
    class FileWriterPluginPNM : public FileWriterPlugin{
      public:
      /// write implementation
      virtual void write(utils::File &file, const core::ImgBase *image);
      
      private:
      /// internal mutex to protect the buffer
      utils::Mutex m_oBufferMutex;
      
      /// internal data conversion buffer
      std::vector<icl8u> m_vecBuffer;
    };
  } // namespace io
}
