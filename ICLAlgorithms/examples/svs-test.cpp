/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLAlgorithms/examples/svs-test.cpp                    **
** Module : ICLAlgorithms                                          **
** Authors: Christof Elbrechter, Andre Justus                      **
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

#include <ICLAlgorithms/SVS.h>
#include <ICLQuick/Common.h>
#include <ICLUtils/Timer.h>
#include <iterator>
#include <fstream>
#include <ICLIO/File.h>

const char *inifile_light[] = {
  "# SVS Engine v 3.2 Stereo Camera Parameter File",
  "[left camera]",
  "pwidth 640", 
  "pheight 480", 
  "dpx 0.007000", 
  "dpy 0.007000", 
  "sx 1.000000", 
  "Cx 320.526995", 
  "Cy 240.809716", 
  "f 511.363092", 
  "fy 513.712339", 
  "alpha 0.000000", 
  "kappa1 -0.159408", 
  "kappa2 0.161214", 
  "kappa3 0.000000", 
  "tau1 0.000000", 
  "tau2 0.000000", 
  "proj", 
  "5.140000e+002 0.000000e+000 3.065270e+002 0.000000e+000", 
  "0.000000e+000 5.140000e+002 2.757325e+002 0.000000e+000", 
  "0.000000e+000 0.000000e+000 1.000000e+000 0.000000e+000", 
  "rect", 
  "9.990557e-001 -1.115309e-003 -4.343369e-002", 
  "1.107551e-003 9.999993e-001 -2.026861e-004", 
  "4.343389e-002 1.543897e-004 9.990563e-001", 
  "",
  "[right camera]",
  "pwidth 640", 
  "pheight 480", 
  "dpx 0.007000", 
  "dpy 0.007000", 
  "sx 1.000000", 
  "Cx 320.916494", 
  "Cy 240.655358", 
  "f 521.225593", 
  "fy 523.525160", 
  "alpha 0.000000", 
  "kappa1 -0.152761", 
  "kappa2 0.142905", 
  "kappa3 0.000000", 
  "tau1 0.000000", 
  "tau2 0.000000", 
"proj", 
  "5.140000e+002 0.000000e+000 3.239165e+002 -4.570280e+004", 
  "0.000000e+000 5.140000e+002 2.757325e+002 0.000000e+000", 
  "0.000000e+000 0.000000e+000 1.000000e+000 0.000000e+000", 
"rect", 
  "9.988248e-001 1.767150e-003 -4.843450e-002", 
  "-1.758498e-003 9.999985e-001 2.212436e-004", 
  "4.843481e-002 -1.358116e-004 9.988263e-001", 
  "",
  0 
  };

const char *inifile[] = {
  "# SVS Engine v 3.2 Stereo Camera Parameter File",
  "",
  "[image]",
  "max_linelen 640", 
  "max_lines 480", 
  "max_decimation 1", 
  "max_binning 1", 
  "max_framediv 1", 
  "gamma 0",
  "color_right 0", 
  "color 1", 
  "ix 0", 
  "iy 0", 
  "vergence 0.000000", 
  "rectified 1", 
  "width 640", 
  "height 480", 
  "linelen 640", 
  "lines 480", 
  "decimation 1", 
  "binning 1", 
  "framediv 0", 
  "subwindow 0", 
  "have_rect 1", 
  "autogain 0", 
  "autoexposure 0", 
  "autowhite 0", 
  "autobrightness 0", 
  "gain 50", 
  "exposure 50", 
  "contrast 50", 
  "brightness 50", 
  "saturation 50",
  "red 0", 
  "blue 0", 
  "",
  "[stereo]",
  "convx 11", 
  "convy 11",
  "corrxsize 15", 
  "corrysize 15",
  "thresh 7", 
  "unique 10", 
  "lr 1", 
  "ndisp 32", 
  "dpp 16", 
  "offx 0", 
  "offy 0", 
  "frame 1.050000", 
  "",
  "[external]",
  "Tx -88.915959",
  "Ty -0.157128", 
  "Tz 4.306600", 
  "Rx 0.000422", 
  "Ry 0.005006", 
  "Rz 0.002866", 
  "",
  "[left camera]",
  "pwidth 640", 
  "pheight 480", 
  "dpx 0.007000", 
  "dpy 0.007000", 
  "sx 1.000000", 
  "Cx 306.526995", 
  "Cy 286.809716", 
  "f 511.363092", 
  "fy 513.712339", 
  "alpha 0.000000", 
  "kappa1 -0.159408", 
  "kappa2 0.161214", 
  "kappa3 0.000000", 
  "tau1 0.000000", 
  "tau2 0.000000", 
  "proj", 
  "5.140000e+002 0.000000e+000 3.065270e+002 0.000000e+000", 
  "0.000000e+000 5.140000e+002 2.757325e+002 0.000000e+000", 
  "0.000000e+000 0.000000e+000 1.000000e+000 0.000000e+000", 
  "rect", 
  "9.990557e-001 -1.115309e-003 -4.343369e-002", 
  "1.107551e-003 9.999993e-001 -2.026861e-004", 
  "4.343389e-002 1.543897e-004 9.990563e-001", 
  "",
  "[right camera]",
  "pwidth 640", 
  "pheight 480", 
  "dpx 0.007000", 
  "dpy 0.007000", 
  "sx 1.000000", 
  "Cx 323.916494", 
  "Cy 264.655358", 
  "f 521.225593", 
  "fy 523.525160", 
  "alpha 0.000000", 
  "kappa1 -0.152761", 
  "kappa2 0.142905", 
  "kappa3 0.000000", 
  "tau1 0.000000", 
  "tau2 0.000000", 
"proj", 
  "5.140000e+002 0.000000e+000 3.239165e+002 -4.570280e+004", 
  "0.000000e+000 5.140000e+002 2.757325e+002 0.000000e+000", 
  "0.000000e+000 0.000000e+000 1.000000e+000 0.000000e+000", 
"rect", 
  "9.988248e-001 1.767150e-003 -4.843450e-002", 
  "-1.758498e-003 9.999985e-001 2.212436e-004", 
  "4.843481e-002 -1.358116e-004 9.988263e-001", 
  "",
  "[global]",
  "GTx 0.000000", 
  "GTy 0.000000", 
  "GTz 0.000000", 
  "GRx 0.000000", 
  "GRy 0.000000", 
  "GRz 0.000000",
  0 
};



int main(){
  {
    std::ofstream o("cfg-374374943464543.ini");
    for(int i=0;inifile[i];++i){
      o << inifile[i] << std::endl;
    }
    //    std::copy(inifile,inifile+(121-7),std::ostream_iterator<std::string>(o,"\n"));
  }

  ImgBase *outImg=0;

  Img8u imgL,imgR;
  
  try{
    imgL = cvt8u(scale(load("demoImages/face320-cal-L.pgm"),320,240)); /// Why??
    imgR = cvt8u(scale(load("demoImages/face320-cal-R.pgm"),320,240));
  }catch(const FileNotFoundException &){
    std::cout << "demo images that are necessary for this demo could not be found\n"
              << "in ./demoImages/face320-cal-*.pgm (* is L or R). You can run\n"
              << "this demo from your $ICL_SOURCE_ROOT/ICLAlgorithms/example dir.\n";
    exit(-1);
  }
  SVS svs;
  Timer timer;

  printf("\nBenchmarking SVS\n");
  timer.start();
  int N=5;
  printf("Doing %d iterations\n",N);
  for(int i=0;i<N;i++){
    svs.load(&imgL,&imgR);
    svs.loadCalibration("cfg-374374943464543.ini");
    svs.printvars();

    timer.stopSubTimer("Iterations of Load done");
    svs.doStereo();
    outImg=svs.getDisparity();
  }
  timer.stop("Iterations of get_disparity done");
  //File("cfg-374374943464543.ini").erase();
  show((label(cvt(imgL),"left image"),
        label(cvt(imgR),"right image"),
        label(cvt(outImg),"depth-image")));
}
