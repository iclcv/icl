/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/ChromaClassifierIO.h                     **
** Module : ICLQt                                                  **
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

#ifndef ICL_CHROMA_CLASSIFIER_IO_H
#define ICL_CHROMA_CLASSIFIER_IO_H

#include <ICLCC/Parable.h>
#include <ICLCC/ChromaClassifier.h>
#include <ICLCC/ChromaAndRGBClassifier.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/StringUtils.h>

namespace icl{
  
  class ChromaClassifierIO{
    public:
    static void save(const ChromaClassifier &cc, 
                     const std::string &filename, 
                     const std::string &name="chroma-classifier");

    static void save(const ChromaAndRGBClassifier &carc, 
                     const std::string &filename);
    
    static ChromaClassifier load(const std::string &filename, 
                                 const std::string &name="chroma-classifier");
    
    static ChromaAndRGBClassifier loadRGB(const std::string &filename);
  };
}

#endif
