/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLGeom/examples/intrinsic-camera-calibration-tools.h  **
** Module : ICLGeom                                                **
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
*********************************************************************/

#ifndef INTRINSIC_CAMERA_CALIBRATION_TOOLS_H
#define INTRINSIC_CAMERA_CALIBRATION_TOOLS_H

#include <ICLCore/Img.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/FixedVector.h>

namespace icl{
  typedef FixedColVector<double,2> P64f;
  typedef FixedMatrix<double,2,2> M64f;

  struct CalibrationStep{
    Img32f colorImage;
    Img32f image;
    std::vector<Point32f> points;
    Point32f &operator[](int idx) { return points[idx]; }
    const Point32f &operator[](int idx) const { return points[idx]; }

    void showSelf() const{
      std::cout << "Calibration Step npoints: " << points.size() << "image: " << image << std::endl;
    }
  };

  struct CalibrationData{
    std::vector<CalibrationStep> data;
    int nx,ny;
    int nloops() const { return (int) data.size(); }
    int dim() const { return nx*ny; }
    const CalibrationStep &operator[](int idx) const { return data[idx]; }
    CalibrationStep &operator[](int idx){ return data[idx]; }
    
    void showSelf() const{
      std::cout << "Calibration Data  " << data.size() << " slices nx:" << nx << " ny:" << ny << std::endl;
      for(unsigned int i=0;i<data.size();++i){
        data[i].showSelf();
      }
    }
  };


  double get_fitting_error(const CalibrationData &data, double dist_factor[4]);
  
  double check_error(const std::vector<double> &x, const std::vector<double> &y, int num, double dist_factor[4]);
  
  double calc_distortion2(const CalibrationData &data, double dist_factor[4]);
  
  double get_size_factor(double dist_factor[4], int xsize, int ysize);
  
  void calc_distortion(const CalibrationData &data,int xsize, int ysize, double dist_factor[4]);

  void calc_distortion_stochastic_search(const CalibrationData &data,
                                         int xsize, int ysize, double dist_factor[4]);


  void apply_pca(const std::vector<P64f> &input, M64f &evec, P64f &eval, P64f &mean);
}
#endif
