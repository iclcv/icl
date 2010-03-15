/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLIO/IOFunctions.h                            **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Götting, Robert Haschke   **
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

#ifndef ICL_IO_FUNCTIONS_H
#define ICL_IO_FUNCTIONS_H

#include <string>
#include <ICLCore/Types.h>

/// icl namespace
namespace icl {

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
  void labelImage(ImgBase *image,const std::string &label);

} //namespace icl

#endif
