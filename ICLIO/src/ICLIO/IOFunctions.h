/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/IOFunctions.h                          **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Goetting, Robert Haschke  **
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
#include <ICLCore/Types.h>
#include <string>

/// icl namespace
namespace icl {
  namespace io{

    /// draws a label into the upper left image corner \ingroup UTILS_G
    /** This utility function can be used e.g. to identify images in longer
        computation queues. Internally is uses a static map of hard-coded
        ascii-art letters ('a'-'z)'=('A'-'Z'), ('0'-'9') and ' '-'/' are defined yet.
        which associates letters to letter images and corresponding offsets.
        Some tests showed, that is runs very fast (about 100ns per call).
        Note, that no line-break mechanism is implemented, so the labeling
        is restricted to a single line, which is cropped, if the label would
        overlap with the right or bottom  image border.
    */
    ICLIO_API void labelImage(core::ImgBase *image, const std::string &label);

  } // namespace io
} //namespace icl
