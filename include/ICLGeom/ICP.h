/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLGeom/ICP.h                                  **
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
#ifndef ICL_ICP_H_
#define ICL_ICP_H_

#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/KDTree.h>
#include <ICLGeom/PoseEstimator.h>

namespace icl{

  /// Implementaiton of the Iterator Closest Point (ICP) Algorithm
  /** TODO: Add Documentation 
      What about a fixed 3D-Version that uses 3D-Fixed Matrix data?
  */
  class ICP : public Uncopyable{
    public:
    /// Simple result structure
    struct Result{
      Result();
      DynMatrix<icl64f> rotation;    //!< Model rotation matrix
      DynMatrix<icl64f> translation; //!< Model translation matrix
      double error;                  //!< Error value
    };

    private:
    /// rotation, translation and error value    Result m_result;
    Result m_result;
    
    /// internal data structure for efficient search
    KDTree kdt;
    
    /// internally used utility function
    DynMatrix<icl64f> *compute(const std::vector<DynMatrix<icl64f>* > &data,
                               const std::vector<DynMatrix<icl64f>* > &model);
    public:
    
  
    
    /// constructor with given model data
    /** TODO is the data passed shallowly or deeply */ 
    ICP(std::vector<DynMatrix<icl64f> > &model);
    
    /// constructor with given model data
    /** TODO is the data passed shallowly or deeply */ 
    ICP(std::vector<DynMatrix<icl64f>* > &model);
    
    /// Empty constructor without given model data
    ICP();
    
    /// Destructor
    ~ICP();
    
    
    /// applies the ICP on given point cloude
    const Result &apply(const std::vector<DynMatrix<icl64f>* > &pointlist);
    
    /// computes th error between given model and data cloude
    static double error(const std::vector<DynMatrix<icl64f>* > &dat,
                        const std::vector<DynMatrix<icl64f>* > &mod);

#if 0
    // hope we dont need that anymore ...
    /// Returns the error from last icp(..) call
    inline double getError(){
      return m_result.error;
    }
    
    /// returns the resulting rotation matrix from the last icp(..) call
    inline const DynMatrix<icl64f>& getRotation() const{
      return m_result.rotation;
    }
    
    /// returns the resulting translation matrix from the last icp(..) call
    inline const DynMatrix<icl64f>& getTranslation() const{
      return m_result.translation;
    }
#endif
  };
}

#endif /* ICL_ICP_H_ */
