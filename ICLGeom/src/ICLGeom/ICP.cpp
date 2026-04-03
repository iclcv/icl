// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#include <ICLGeom/ICP.h>

using namespace icl::utils;
using namespace icl::math;

namespace icl::geom {
  ICP::Result::Result():rotation(3,3),translation(1,3),error(0.1){}

  ICP::ICP(std::vector<DynMatrix<icl64f> > &model) {
    kdt.buildTree(model);
  }

  ICP::ICP(std::vector<DynMatrix<icl64f>* > &model) {
    kdt.buildTree(model);
  }

  ICP::ICP(){}

  ICP::~ICP(){}

  const ICP::Result &ICP::apply(const std::vector<DynMatrix<icl64f>* > &pointlist){
    FixedMatrix<icl32f,4,4> mat;
    DynMatrix<icl64f> mat3(4,4);
    DynMatrix<icl32f> XsD(pointlist.size(),3),YsD(pointlist.size(),3);
    double eye[] = {1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    0.0, 0.0, 0.0, 1.0};
    DynMatrix<icl64f> mat2(4,4,eye);
    DynMatrix<icl64f> temp(4,4);
    double cerror = 0.0;
    std::vector<DynMatrix<icl64f>* > np;
    std::vector<DynMatrix<icl64f>* > lpointlist;
    for(unsigned int i=0;i<pointlist.size();++i){
      lpointlist.push_back(new DynMatrix<icl64f>(pointlist.at(0)->cols(),pointlist.at(0)->rows(),pointlist.at(i)->data(),true));
    }
    do{
      np.clear();
      m_result.error=cerror;
      for(unsigned int i=0;i<lpointlist.size();++i){
        DynMatrix<icl64f> *p = kdt.nearestNeighbour(lpointlist.at(i));
        np.push_back(p);
      }
      for(unsigned int i=0;i<lpointlist.size();++i){
        std::copy((np[i])->begin(),(np[i])->begin()+3, YsD.col_begin(i));
        std::copy((lpointlist[i])->begin(),(lpointlist[i])->begin()+3, XsD.col_begin(i));
      }
      mat = PoseEstimator::map(XsD,YsD);
      for(unsigned int i=0;i<16;++i){
        mat3[i] = mat[i];
      }
      for(unsigned int i=0;i<3;++i){
        m_result.translation[i] = mat3.at(3,i);
        for(unsigned int j=0;j<3;++j){
          m_result.rotation.at(i,j) = mat3.at(i,j);
        }
      }
      //SHOW(rotation);
      //SHOW(translation);
      //SHOW(*(np[0]));
      //SHOW(*(lpointlist[0]));
      // Transform each point: p' = R*p + t (in-place, no heap allocations)
      {
        DynMatrix<icl64f> tmp(1,3);
        for(unsigned int i=0;i<lpointlist.size();++i){
          m_result.rotation.mult(*lpointlist[i], tmp);
          tmp += m_result.translation;
          std::copy(tmp.begin(), tmp.end(), lpointlist[i]->begin());
        }
      }
      //SHOW(*(lpointlist[0]));
      mat3.mult(mat2,temp);
      mat2 = temp;
      cerror = error(lpointlist,np);
      std::cout << cerror << "   " << m_result.error << std::endl;
      /*if(std::abs(cerror-m_error)<0.01)
          break;*/

    }while(cerror != m_result.error);
    for(unsigned int i=0;i<3;++i){
      m_result.translation[i] = mat2.at(3,i);
      for(unsigned int j=0;j<3;++j){
        m_result.rotation.at(i,j) = mat2.at(i,j);
      }
    }
    for(unsigned int i=0;i<lpointlist.size();++i){
      delete lpointlist[i];
    }
    return m_result;
  }

  //TODO another way to compute rotation and translation
  DynMatrix<icl64f> *ICP::compute(const std::vector<DynMatrix<icl64f>* > &data,const std::vector<DynMatrix<icl64f>* > &model){
    DynMatrix<icl64f> mean_data(1,3);
    DynMatrix<icl64f> mean_model(1,3);
    for(unsigned int i=0;i<model.size();++i){
      mean_data += (*(data[i]));
      mean_model += (*(model[i]));
    }
    for(unsigned int i=0;i<3;++i){
      mean_data[i] = mean_data[i]/data.size();
      mean_model[i] = mean_model[i]/model.size();
    }
    for(unsigned int i=0;i<data.size();++i){

    }
    return 0;
  }

  double ICP::error(const std::vector<DynMatrix<icl64f>* > &dat, const std::vector<DynMatrix<icl64f>* > &mod){
    if(dat.empty() || mod.empty() || dat.size() != mod.size()) return -1.0;
    double error = 0.0;
    for(unsigned int j=0;j<dat.size();++j){
      double sumsq = 0.0;
      const auto &a = *dat[j], &b = *mod[j];
      for(unsigned k=0;k<a.rows();++k){
        double dk = a[k] - b[k];
        sumsq += dk*dk;
      }
      error += std::sqrt(sumsq);
    }
    return error / dat.size();
  }
  } // namespace icl::geom