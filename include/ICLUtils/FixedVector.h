/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
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

/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
** University of Bielefeld                                                **
** Contact: nivision@techfak.uni-bielefeld.de                             **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
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

#ifndef ICL_FIXED_VECTOR_H
#define ICL_FIXED_VECTOR_H

#include <ICLUtils/FixedMatrix.h>

namespace icl{
  
  template<class T, int DIM>
  struct FixedColVector : public FixedMatrix<T,1,DIM>{
    typedef FixedMatrix<T,1,DIM> super;
    FixedColVector(){}
    FixedColVector(const T &init):super(init){}
    FixedColVector(const T *srcData):super(srcData){}
    FixedColVector(const FixedMatrix<T,1,DIM> &other):super(other){}
    FixedColVector(const T&v0,const T&v1, const T&v2=0, const T&v3=0, const T&v4=0, const T&v5=0, 
                   const T&v6=0,const T&v7=0, const T&v8=0, const T&v9=0, const T&v10=0, const T&v11=0):
    super(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11){}

    template<class Iterator>
    FixedColVector (const FixedMatrixPart<T,DIM,Iterator> &r):super(r){}

    template<class otherT, class Iterator>
    FixedColVector(const FixedMatrixPart<otherT,DIM,Iterator> &r):super(r){}

  
  };

  
  template<class T, int DIM>
  struct FixedRowVector : public FixedMatrix<T,DIM,1>{
    typedef FixedMatrix<T,DIM,1> super;
    FixedRowVector(){}
    FixedRowVector(const T &init):super(init){}
    FixedRowVector(const T *srcData):super(srcData){}
    FixedRowVector(const FixedMatrix<T,DIM,1> &other):super(other){}
    FixedRowVector(const T&v0,const T&v1, const T&v2=0, const T&v3=0, const T&v4=0, const T&v5=0, 
                   const T&v6=0,const T&v7=0, const T&v8=0, const T&v9=0, const T&v10=0, const T&v11=0):
    super(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11){}

    template<class Iterator>
    FixedRowVector (const FixedMatrixPart<T,DIM,Iterator> &r):super(r){}

    template<class otherT, class Iterator>
    FixedRowVector(const FixedMatrixPart<otherT,DIM,Iterator> &r):super(r){}

  };
  
  
  
}

#endif
