/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLBlob/src/VQ2D.cpp                                   **
** Module : ICLBlob                                                **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

#include <ICLBlob/VQ2D.h>
#include <math.h>

namespace icl{
  
  inline float distance(float *a, float *b){
    // {{{ open

    return sqrt( pow(a[0]-b[0],2)+pow(a[1]-b[1],2) );
  }

  // }}}
  inline int nn(float *data, int dim, float* point, float &dist){
    // {{{ open
    int bestIndex = 0;
    dist = distance(data,point);
    float currDist = -1;
    for(int i=1 ; i<dim ; ++i){
      currDist = distance(data+2*i,point); 
      if( currDist < dist){
        bestIndex = i;
        dist = currDist;
      }
    }
    return bestIndex;
  } 
  // }}}
  inline void copy_elem(float *src, int isrc, float *dst, int idst){
    // {{{ open

    dst[2*idst] = src[2*isrc];
    dst[2*idst+1] = src[2*isrc+1];
  }

  // }}}

  VQ2D::VQ2D(float *data, int dim, bool deepCopyData){
    // {{{ open

    m_poCenters = 0;
    m_poClusterInfo = 0;
    if(dim && data){
      m_poData = new VQVectorSet(data,dim, deepCopyData);
    }else{
      m_poData = new VQVectorSet();
    }
    m_poCenters = new VQVectorSet();
    m_poClusterInfo = new VQClusterInfo();
  }

  // }}}
  
  void VQ2D::setData(float *data, int dim, bool deepCopyData){
    // {{{ open
    ICLASSERT_RETURN( data );
    ICLASSERT_RETURN( dim );

    delete m_poData;
    m_poData = new VQVectorSet(data,dim,deepCopyData); 
  }

  // }}}

  const VQVectorSet &VQ2D::run(int k, int maxSteps, float mmqe, float &qe){
    // {{{ open

	 randomSeed();

    // temporary variables
    unsigned int n = m_poData->dim();
    float *data = m_poData->data();
    m_poCenters->resize(k);
    float *centers = m_poCenters->data();

    // check if all parameters are valid!
    ICLASSERT_RETURN_VAL( k>=0 ,*m_poCenters);
    ICLASSERT_RETURN_VAL( n>=0 ,*m_poCenters);
        
    /**************************************************
    ** Random initialisation of the center vectors ****      >> TODO other strategy or
    ***************************************************        give them from anywhere else */
    for(int p=0 ; p<k ; ++p){
      copy_elem(data,random(n-1),centers,p);
    } 
   

    std::vector<float> vecx(k),vecy(k); // mean x/y accumulators
    std::vector<int> vecn(k); // cluster size counters

    
    for(int step = 0 ; step<maxSteps ; ++step){
      float errBuf(0);
      
      // clean mean-akkumulators and elem counter
      std::fill(vecx.begin(),vecx.end(),0);
      std::fill(vecy.begin(),vecy.end(),0);
      std::fill(vecn.begin(),vecn.end(),0);

      /*************************************************
      ** 1.Step calculating voronoi cells **************
      *************************************************/
      for(unsigned int i = 0 ; i<n; ++i){
        int iNN = nn(centers,k,data+2*i,errBuf);
        qe += errBuf;
        vecx[iNN] += data[2*i];
        vecy[iNN] += data[2*i+1];
        vecn[iNN]++;
      }
 
      // test if minimum quantisation error has already been reached
      if(qe/n <= mmqe) break;
      
      /*************************************************
      ** 2.Step updating centers to voronoi means ******
      *************************************************/
      for(int i = 0 ; i<k ; ++i){     
        if(!vecn[i]){
          centers[2*i]=centers[2*i+1]=-1;
        }
        else{
          centers[2*i]  = vecx[i]/vecn[i];
          centers[2*i+1]= vecy[i]/vecn[i];
        }     
      }
      
    }
    qe/=n;
    return *m_poCenters;
  }
  // }}}

  const VQClusterInfo &VQ2D::features(){
    // {{{ open
    int k = m_poCenters->dim();
    int n = m_poData->dim();
    VQVectorSet &centers = *m_poCenters;
    VQVectorSet &data = *m_poData;
    VQClusterInfo &info = *m_poClusterInfo;
    info.resize(k);
    info.clear();
    
    float err, *p;
    int inn;
    for(int i=0;i<n;i++){
      inn = nn(centers.data(),k,data[i],err);
      p = data[i];
      info.addElem(inn,p[0],p[1]);
      info.error(inn)+=err;
    }
    float x(0),y(0),mxx(0),mxy(0),myy(0),l1(0),l2(0);
    for(int i=0 ; i<k ;  i++){
      int s = info.size(i);
      info.error(i)/=s;
      for(int j=0 ; j<s ; j++){
        x = info.elem(i,j)[0];
        y = info.elem(i,j)[1];
        //mx += x;
        //my += y;
        mxx += x*x;
        myy += y*y;
        mxy += x*y;
      }
      mxx/=s;
      mxy/=s;
      myy/=s;

      info.center(i)[0] = (centers[i])[0];//mx/s;
      info.center(i)[1] = (centers[i])[1];//my/s;
      
      l1 = (mxx+myy)/2 + sqrt( pow(mxx-myy,2)/4 +mxy*mxy );
      l2 = (mxx+myy)/2 - sqrt( pow(mxx-myy,2)/4 +mxy*mxy );
      
      info.pcainfo(i)[0] = atan2(l1-mxx, mxy);       
      info.pcainfo(i)[1] = sqrt(l1);
      info.pcainfo(i)[2] = sqrt(l2);
    }
    return info;
  }
  // }}}
  
  const VQVectorSet &VQ2D::centers(){
    // {{{ open

    return *m_poCenters; 
  }

  // }}}
}
