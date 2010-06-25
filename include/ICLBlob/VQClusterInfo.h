/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/VQClusterInfo.h                        **
** Module : ICLBlob                                                **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef VQCLUSTERINFO_H
#define VQCLUSTERINFO_H

#include <vector>

namespace icl{

  /// Utiltiy class for accessing meta-information about VQClusters \ingroup G_UTILS
  /** The dimension of a VQCluster info is equal to the center/
      prototype/cluster-count of the top-level VQ2D object. For each 
      prototype/center/cluster the cluster info provides some meta
      information like cluster mean or clusters major axis (length
      and angle) 
  **/
  class VQClusterInfo{
    /// internally used float vector type
    typedef std::vector<float> fvec;  
    public:

    /// creates a new cluster info with given dimension
    VQClusterInfo(int dim=0): m_iDim(0), m_pfCenters(0),
      /* {{{ open */

      m_pfPCAInfos(0), m_pfErrors(0), m_pvecElems(0){
      if(dim){
        resize(dim);
      }
    }

    /* }}} */

    /// Destructor
    ~VQClusterInfo(){
      /* {{{ open */

      resize(0);
    }

    /* }}} */
    
    /// resets the dimension of this cluster info class
    /** old data content is lost */
    void resize(int dim){
      /* {{{ open */

      if(m_iDim == dim) return;
      m_iDim = dim;
      if(m_pfCenters){
        delete m_pfCenters;
        delete m_pfPCAInfos;
        delete m_pfErrors;
        delete m_pvecElems;
      }
      if(dim){
        m_pfCenters = new float[dim*2];
        m_pfPCAInfos = new float[dim*3];
        m_pfErrors = new float[dim];
        m_pvecElems = new fvec[dim];
      }else{
        m_pfCenters = 0;
        m_pfPCAInfos = 0;
        m_pfErrors = 0;
        m_pvecElems = 0;
      }
    }

    /* }}} */
    
    /// adds a data element for given prototype index 
    void addElem(int prototypeIndex, float x, float y){
      /* {{{ open */

      //      printf("adding index-%d dim-%d \n",prototypeIndex,m_iDim);
      m_pvecElems[prototypeIndex].push_back(x);
      m_pvecElems[prototypeIndex].push_back(y);
    }

    /* }}} */

    /// clears internal data with 0
    void clear(){
      /* {{{ open */
      for(int i=0;i<m_iDim;i++){
        m_pvecElems[i].clear();
      }
      std::fill(m_pfErrors,m_pfErrors+m_iDim,0.f);
      std::fill(m_pfCenters,m_pfCenters+2*m_iDim,0.f);
      std::fill(m_pfPCAInfos,m_pfPCAInfos+3*m_iDim,0.f);
    }

    /* }}} */

    /// returns the dimension of this cluster info
    int dim() const {
      /* {{{ open */

      return m_iDim; 
    }

    /* }}} */
    /// returns center of a cluster
    float *center(int index=0) const {
      /* {{{ open */

      return m_pfCenters+2*index;
    }

    /* }}} */
    
    /// returns the pca-information of cluster c 
    /** @return pca-information (data order: [len , len2, angle of axis 1]*/
    float *pcainfo(int index=0) const {
      /* {{{ open */

      return m_pfPCAInfos+3*index;
    }

    /* }}} */
    
    /// returns the quantisation error for cluster at given index
    float &error(int index=0) const {
      /* {{{ open */

      return m_pfErrors[index];
    }

    /* }}} */
    int size(int index) const {
      /* {{{ open */

      return (int)m_pvecElems[index].size();
    }

    /* }}} */

    /// returns a specific data element according to a certain prototype
    float *elem(int prototypeIndex, int elemIndex=0) const {
      /* {{{ open */

      return &((m_pvecElems[prototypeIndex])[2*elemIndex]);
    }

    /* }}} */
  
    private:
    /* {{{ members */

    int m_iDim;          ///!< center count
    float *m_pfCenters;  ///!< center data
    float *m_pfPCAInfos; ///!< pca information data
    float *m_pfErrors;   ///!< error data
    fvec *m_pvecElems;   ///!< data elements for each center

    /* }}} */
  };
}

#endif
