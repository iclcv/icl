#ifndef TESTIMAGES_H
#define TESTIMAGES_H

#include "Img.h"
#include <string>

namespace icl{
  class TestImages{
    public:
    /// creates a new testimage instance
    /** possible values for name are
        - women
        - house
        - tree
    */
    template<class T> 
    Img<T> 
    create(string name, const Size &size=Size(320,240),format f=formatRGB);

    /// writes the image to the disc an shows it using xv.
    void xv(ImgI *image, string tmpName="./tmp_image.pgm",long msec_to_rm_call=1000);
  };
}

#endif
