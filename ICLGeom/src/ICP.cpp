/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICP.cpp                                    **
** Module : ICLGeom                                                **
** Authors: Christian Groszewski, Christof Elbrechter              **
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
#include <ICLGeom/ICP.h>

using namespace icl;
namespace icl{

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
    DynMatrix<icl64f> *mm2 = 0;
    DynMatrix<icl64f> *mm = 0;
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
      for(unsigned int i=0;i<lpointlist.size();++i){
        mm2 = lpointlist[i];
        mm = new DynMatrix<icl64f>(1,3);
        m_result.rotation.mult(*mm2,*mm);
        (*mm) = (*mm)+m_result.translation;
        lpointlist[i] = mm;
        delete mm2;
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
    double error = 0.0;
    double sumsq = 0.0;
    DynMatrix<icl64f> d(1,3);
    if(dat.size() == mod.size() && dat.size()>0 && mod.size()>0){
      for(unsigned int j=0;j<dat.size();++j){
        d = (*(dat.at(j)))-(*(mod.at(j)));
        sumsq = 0.0;
        for(unsigned k=0;k<d.rows();++k){
          sumsq += d[k]*d[k];
        }
        error += std::sqrt(sumsq);
      }
      error = error/dat.size();
    } else {
      error = -1.0;
    }
    //std::cout << "error: " << error << std::endl;
    return error;
  }
}
