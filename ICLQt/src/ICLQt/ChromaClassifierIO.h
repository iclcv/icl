/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ChromaClassifierIO.h                   **
** Module : ICLQt                                                  **
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
#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/StringUtils.h>
#include <ICLCore/Parable.h>
#include <ICLCore/ChromaClassifier.h>
#include <ICLCore/ChromaAndRGBClassifier.h>

namespace icl{
  namespace qt{

    class ICLQt_API ChromaClassifierIO{
      public:
      static void save(const core::ChromaClassifier &cc,
                       const std::string &filename,
                       const std::string &name="chroma-classifier");

      static void save(const core::ChromaAndRGBClassifier &carc,
                       const std::string &filename);

      static core::ChromaClassifier load(const std::string &filename,
                                   const std::string &name="chroma-classifier");

      static core::ChromaAndRGBClassifier loadRGB(const std::string &filename);
    };
  } // namespace qt
}

