/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/CCLUT.h                            **
** Module : ICLCore                                                **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <ICLUtils/ConsoleProgress.h>
#include <vector>
#include <string>

namespace icl{
  namespace core{

    class ICLCore_API CCLUT{
      public:
      CCLUT(format srcFmt, format dstFmt);
      void cc(const ImgBase *src, ImgBase *dst, bool roiOnly=false);

      private:
      Img8u m_oLUT;
      // format m_eSrcFmt;
      // format m_eDstFmt;
    };


    /// TODO basisclass CCLUT
    /// unterklassen CCLUT3x3 usw
    /** evtl: kleinere luts durch internes runtersamplen der
        farbinfos z.B. factor 2 oder 4
        2: 3*2 statt 3*16mb

        zusätzliche beschleunigung:
        statt yuvToRGB vielleicht ippiYUVToHLS direkt im grabber!
    **/
    /******************
        class CCLUT{
        static const int OFS1 = 256;
        static const int OFS2 = 65536;

        public:
        CCLUT(format srcFmt, format dstFmt){
        m_iSrcChan = getChannelsOfFormat(srcFmt);
        m_iDstChan = getChannelsOfFormat(dstFmt);

        int DIM = (int)pow(256,m_iSrcChan);

        // alloc lut
        memset(m_apucLUT,0,3*sizeof(icl8u*));
        for(int i=0;i<m_iDstChan;++i){
        m_apucLUT[i] = new icl8u[DIM];
        }

        /// this must be much bigger
        Img8u bufSrc(Size(256*256*256,3),srcFmt);
        Img8u bufDst(Size(256*256*256,3),dstFmt);

        for(int r=0;r<256;r++){
        std::fill(bufSrc.getData(0)+r*OFS2, bufSrc.getData(0)+(r+1)*OFS2, icl8u(r));
        icl8u *pg = bufSrc.getData(1)+r*OFS2;
        for(int g=0;g<256;g++){
        std::fill(pg+g*OFS1,pg+(g+1)*OFS1, icl8u(g));
        icl8u *pb = bufSrc.getData(2)+r*OFS2 + g*OFS1;
        for(int b=0;b<256;b++){
        pb[b] = icl8u(b);
        }
        }
        }
        }
        icl8u *m_apucLUT[3];

        inline void convert3to3(const icl8u &s1,
        const icl8u &s2,const
        icl8u &s3, icl8u s1,
        icl8u &d1, icl8u &d2, icl8u &d3){
        int idx = s1+OFS1*s2+OFS3*s3;
        d1 = m_apucLUT[0][idx];
        d2 = m_apucLUT[1][idx];
        d3 = m_apucLUT[2][idx];
        }
        };
        */
  } // namespace core
}
