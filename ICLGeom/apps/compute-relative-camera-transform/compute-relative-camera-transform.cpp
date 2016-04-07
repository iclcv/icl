/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/apps/show-scene/compute-relative-camera-       **
** transform.cpp                                                   **
** Module : ICLGeom                                                **
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

#include <ICLQt/Common.h>
#include <ICLGeom/Camera.h>
#include <fstream>

Mat compute_relative_transform(const Camera &s, const Camera &d){
  Mat ms = s.getInvCSTransformationMatrix();
  Mat md = d.getInvCSTransformationMatrix();
  //    Mat rel = d.getInvCSTransformationMatrix() * s.getCSTransformationMatrix();
  
  Mat3 Rs = ms.part<0,0,3,3>();
  Mat3 Rd = md.part<0,0,3,3>();
  
  Vec3 Ts = ms.part<3,0,1,3>();
  Vec3 Td = md.part<3,0,1,3>();
  
  Mat3 Rrel = Rs.transp() * Rd;
  Vec3 Trel = Td - Ts;
  
  Mat T = Rrel.resize<4,4>(0);
  T.col(3) = Trel.resize<1,4>(1);

  return T;
}

std::ostream &out(){
  if(pa("-o")){
    static std::ofstream s((*pa("-o")).c_str());
    return s;
  }else{
    return std::cout;
  }
}

int main(int n, char**ppc){
  pa_init(n,ppc,
          "-compute-transform|-c(src-xml-file,dst-xml-file) "
          "-add-transform|-a(src-xml-file,transform-output-file) "
          "-add-camera-template|-t(xml-camera-or-udist-file) "
          "-output-file|-o(filename) -how-to|-explain|-?");

  if(pa("-how-to")){
#define say(x) std::cout << x << std::endl
    say("This tool allows relative camera tranformations to be caculated, and to");
    say("apply a resulting relative camera transform to another camera. Here's an");
    say("example:");
    say("We assume that we have a low resolution C1 camera, that cannot be calibrated");
    say("well. In order to perform the calibration, we can however, rigidly attach");
    say("another camera C2 that has a better resolution so it can be calibrated more");
    say("easily. Now, in a very special setup, with an optimal positioning of the");
    say("two rigidly mounted cameras wrt. a calibration object, we assume the ");
    say("calibration to be possible, which results in two calibration files C1.xml");
    say("and C2.xml. In a real setup, we now want to only calibrate the high resolution");
    say("camera C2 in order to then compute the resulting position of C1. To this end,");
    say("firstly the relative transform between C1 and C2 has to be extracted. This is");
    say("achieved by using compute-relative-camera-transform with the -c argument:");
    say("");
    say("compute-relative-camera-transform -c C2.xml C1.xml -o rel.dat");
    say("");
    say("This will compute the relative transformation from C2 to C1 and store the");
    say("result in a file named rel.dat. Subsequently, the two cameras can be mounted");
    say("in the real setup. In this step, it is very important that the relative");
    say("transform between the two cameras is not altered. In the new setup, the");
    say("high-resolution camera is calibrated resulting in a calibration file C2s.xml.");
    say("Now, all data to assemble the calibration file for the low-resolution camera,");
    say("C1s.xml is available. To this end, compute-relative-camera-transform is called");
    say("with the -a sub-argument:");
    say("");
    say("compute-relative-camera-transform -a C2s.xml rel.dat -o C1s.xml -t C1.xml");
    say("");
    say("This will extract the extrinsic camera transform of C2 and add the relative");
    say("transform defined in rel.dat to it. The result is written to C1s.xml.");
    say("In this case, we use the -t argument to provide a template configuration");
    say("for the resulting camera, so that the intrinsic parameters of C1s.xml are");
    say("identical to those of C1.xml. Please note that the sub-argument of -t");
    say("can either be a camera calibration filename or an image-undistortion");
    say("filename. If -t is not used, the resulting camera will");
    say("use the intrinsic parameters of the given source camera -- here, C2s.xml");
    return 0;
  }
  
  if(pa("-c")){
    Mat T = compute_relative_transform(Camera(*pa("-c",0)),
                                       Camera(*pa("-c",1)));
    
    out() << T << std::endl;
  }else if(pa("-a")){
    Camera c(*pa("-a"));
    std::ifstream str((*pa("-a",1)).c_str());
    Mat T;
    str >> T;

    Mat Tcur = c.getInvCSTransformationMatrix();
    Mat3 Rcur = Tcur.part<0,0,3,3>();
    Vec Pcur = Tcur.part<3,0,1,4>();
    
    Mat3 Rrel = T.part<0,0,3,3>();
    Vec Prel = T.part<3,0,1,4>();
    
    if(pa("-t")){
      c = Camera::create_camera_from_calibration_or_udist_file(*pa("-t"));
    }
    Mat3 Rnew = Rcur * Rrel;
    Vec Pnew = Pcur + Prel;

    c.setPosition(Pnew);    
    c.setRotation(Rnew.transp());

    Mat Tnew = compute_relative_transform(Camera(*pa("-a")),c);

    std::cout << "transformation error matrix: " << std::endl
              <<  (Tnew - T) << std::endl;

    out() << c << std::endl;
  }

  
  
}

