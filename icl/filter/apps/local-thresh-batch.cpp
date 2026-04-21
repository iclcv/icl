// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#include <icl/core/CoreFunctions.h>
#include <icl/core/CCFunctions.h>
#include <icl/io/GenericGrabber.h>
#include <icl/io/FileWriter.h>
#include <icl/io/FileList.h>
#include <icl/filter/LocalThresholdOp.h>
#include <icl/utils/ConfigFile.h>
#include <icl/utils/ProgArg.h>
#include <iostream>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;
using namespace icl::io;
using namespace icl::filter;

int main(int argc, char **argv) {
  pa_explain("-input","image source (e.g. -input file 'images/*.png')")
            ("-config","XML config file with masksize, threshold, gammaslope")
            ("-output","output file pattern (use ##### for index)");

  pa_init(argc, argv,
          "[m]-input|-i(device,device-params) "
          "[m]-config|-c(cfg-filename) "
          "[m]-output|-o(output-file-pattern) "
          "-color");

  ConfigFile f(*pa("-config"));
  int masksize = f["config.masksize"];
  int thresh = f["config.threshold"];
  float gamma = f["config.gammaslope"];

  GenericGrabber grabber;
  grabber.init(pa("-i"));

  FileList fl;
  int maxSteps = -1;
  if(grabber.getType() == "file"){
    fl = FileList(*pa("-input",1));
    maxSteps = fl.size();
  }

  FileWriter w(*pa("-output"));
  LocalThresholdOp t;
  t.setMaskSize(masksize);
  t.setGlobalThreshold(thresh);
  t.setGammaSlope(gamma);

  int i = 0;
  while(maxSteps < 0 || maxSteps--){
    if(fl.size()){
      std::cout << "processing " << fl[i++] << " ...";
    }
    Image image = grabber.grabImage();
    if(!pa("-color") && image.getFormat() != formatGray){
      Img8u tmp(image.getSize(), formatGray);
      cc(image.ptr(), &tmp);
      image = tmp;
    }

    Image dst;
    t.apply(image, dst);
    w.write(dst.ptr());
    std::cout << " done → " << w.getFilenameGenerator().showNext() << std::endl;
  }
}
