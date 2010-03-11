/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLBlob module of ICL                         **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef VQ2D_H
#define VQ2D_H

#include <stdlib.h>
#include <ICLBlob/VQVectorSet.h>
#include <ICLBlob/VQClusterInfo.h>
#include <ICLCore/Mathematics.h>
#include <ICLUtils/Time.h>

namespace icl{
  /// Support class for 2-Dimensional vector quatisation using KMeans \ingroup G_UTILS
  /** Vector Quantisation is internally applied using the <b>KMeans</b> algorithm
      which can be defines as follows:
      
      <pre>
      Given data set D of |D| elements with D = d_1,...,d_|D|
      Given center count K with 
      Optionally given data set C = c1,..,cK of initial centers (prototypes) of size |C|=K
      
      algorithm:
      
      for i = 1 to MAX-STEPS do
         for k = 1 to K
            Vc : = { di | |di-ck| < |di-co| for all o!=k } ( Voronoi cell for center c )
         endfor

         for k = 1 to K
            ck := mean(Vc)
         endfor
      
         // current quatisation is given by the set of centers C
         
         if current quatisation error < minError 
            break
         endif
      endfor
      </pre>
      
  **/
  class VQ2D{
    public:
    
    /// create a new VQ
    VQ2D(float *data=0, int dim=0, bool deepCopyData = false);
    
    /// data will be copied once
    void setData(float *data, int dim, bool deepCopy = false);
    
    /// retruns just the center information
    /** @param centers count of prototypes to use 
        @param steps count of steps to perform 
        @param mmqe mininum mean qauntisation error 
        @param qe quantisation error */
    const VQVectorSet &run(int centers, int steps, float mmqe, float &qe);
    
    /// calculates advanced features like local pca
    const VQClusterInfo &features();

    /// return the centers of all clusters
    const VQVectorSet &centers();
    
    protected:
    VQVectorSet *m_poCenters;       ///!< center data
    VQClusterInfo *m_poClusterInfo; ///!< additinal cluster information
    VQVectorSet *m_poData;          ///!< data
  };
}

#endif
