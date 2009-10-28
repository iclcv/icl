#ifndef ICL_IO_FUNCTIONS_H
#define ICL_IO_FUNCTIONS_H

#include <string>
#include <iclTypes.h>

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
