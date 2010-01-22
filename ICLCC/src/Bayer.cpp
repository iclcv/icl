#include <ICLCC/Bayer.h>
#include <ICLCore/Img.h>
#include <ICLCC/CCFunctions.h>

namespace icl {
  
  BayerConverter::BayerConverter(bayerConverterMethod eConvMethod, 
                                 bayerPattern eBayerPattern,
                                 Size s) {
     // Initialize variables
     m_eBayerPattern = eBayerPattern;
     m_eConvMethod = eConvMethod;
     m_vRGBInterImg.resize(s.width * s.height * 3); 
  }
  
  
  BayerConverter::~BayerConverter() { }

  void BayerConverter::apply(const Img8u *src, ImgBase **dst) {
    // {{{ open
    
    // Check bayer and dst image compatibility
    ensureCompatible(dst, depth8u, src->getSize(), 3, formatRGB);
    
    // Select interpolation method
    switch (m_eConvMethod) {
      case nearestNeighbor: 
        FUNCTION_LOG("Nearest Neighbor interpolation");
        nnInterpolation(src);
        break;
        
      case bilinear:
        FUNCTION_LOG("Bilinear interpolation");
        bilinearInterpolation(src);
        break;

      case hqLinear:
        FUNCTION_LOG("High Quality Linear interpolation");
        hqLinearInterpolation(src);
        break;

      case edgeSense:
        FUNCTION_LOG("Edge Sense interpolation");
        edgeSenseInterpolation(src);
        break;

      case simple:
        FUNCTION_LOG("Simple interpolation");
        simpleInterpolation(src);
        break;
        
      default:
        nnInterpolation(src);
    }

    // convert data order from interleaved to planar
    interleavedToPlanar(&m_vRGBInterImg[0], (*dst)->asImg<icl8u>());
  }

// }}}
  
  void BayerConverter::nnInterpolation(const Img<icl8u> *poBayerImg) {
    // {{{ open

    // Variable deklaration
    int iWidth = poBayerImg->getWidth();
    int iHeight = poBayerImg->getHeight();
    int i, imax, iinc;
    const int iBayerStep = iWidth;
    const int iRGBStep = 3 * iWidth;
    const icl8u *bayer = poBayerImg->getData(0);
    icl8u *pucRGBInterImg = &m_vRGBInterImg[0];
    
    int blue = m_eBayerPattern == bayerPattern_BGGR
      || m_eBayerPattern == bayerPattern_GBRG ? -1 : 1;
    int start_with_green = m_eBayerPattern == bayerPattern_GBRG
      || m_eBayerPattern == bayerPattern_GRBG;
    
    //add black border
    imax = iWidth * iHeight * 3;
    for (i = iWidth * (iHeight - 1) * 3; i < imax; i++) {
      pucRGBInterImg[i] = 0;
    }
    iinc = (iWidth - 1) * 3;
    for (i = (iWidth - 1) * 3; i < imax; i += iinc) {
      pucRGBInterImg[i++] = 0;
      pucRGBInterImg[i++] = 0;
      pucRGBInterImg[i++] = 0;
    }
    
    pucRGBInterImg += 1;
    iWidth -= 1;
    iHeight -= 1;

    for (; iHeight>=0; iHeight--, bayer += iBayerStep, pucRGBInterImg += iRGBStep) {
      const icl8u *bayerEnd = bayer + iWidth;

      if (start_with_green) {
        pucRGBInterImg[-blue] = bayer[1];
        pucRGBInterImg[0] = bayer[iBayerStep + 1];
        pucRGBInterImg[blue] = bayer[iBayerStep];
        bayer++;
        pucRGBInterImg += 3;
      }
      
      if (blue > 0) {
        for (; bayer <= bayerEnd - 2; bayer += 2, pucRGBInterImg += 6) {
          pucRGBInterImg[-1] = bayer[0];
          pucRGBInterImg[0] = bayer[1];
          pucRGBInterImg[1] = bayer[iBayerStep + 1];
          
          pucRGBInterImg[2] = bayer[2];
          pucRGBInterImg[3] = bayer[iBayerStep + 2];
          pucRGBInterImg[4] = bayer[iBayerStep + 1];
        }
      } else {
        for (; bayer <= bayerEnd - 2; bayer += 2, pucRGBInterImg += 6) {
          pucRGBInterImg[1] = bayer[0];
          pucRGBInterImg[0] = bayer[1];
          pucRGBInterImg[-1] = bayer[iBayerStep + 1];
          
          pucRGBInterImg[4] = bayer[2];
          pucRGBInterImg[3] = bayer[iBayerStep + 2];
          pucRGBInterImg[2] = bayer[iBayerStep + 1];
        }
      }
      
      if (bayer < bayerEnd) {
        pucRGBInterImg[-blue] = bayer[0];
        pucRGBInterImg[0] = bayer[1];
        pucRGBInterImg[blue] = bayer[iBayerStep + 1];
        bayer++;
        pucRGBInterImg += 3;
      }
      
      bayer -= iWidth;
      pucRGBInterImg -= iWidth * 3;
      
      blue = -blue;
      start_with_green = !start_with_green;
    }
  }

// }}}
  
  void BayerConverter::bilinearInterpolation(const Img<icl8u> *poBayerImg) {
    // {{{ open

    int iWidth = poBayerImg->getWidth();
    int iHeight = poBayerImg->getHeight();
    const int iBayerStep = iWidth;
    const int iRGBStep = 3 * iWidth;
    const icl8u *bayer = poBayerImg->getData(0);
    icl8u *pucRGBInterImg = &m_vRGBInterImg[0];

    int blue = m_eBayerPattern == bayerPattern_BGGR
      || m_eBayerPattern == bayerPattern_GBRG ? -1 : 1;
    int start_with_green = m_eBayerPattern == bayerPattern_GBRG
      || m_eBayerPattern == bayerPattern_GRBG;
    
    clearBorders(pucRGBInterImg, iWidth, iHeight, 1);
    pucRGBInterImg += iRGBStep + 3 + 1;
    iHeight -= 2;
    iWidth -= 2;
    
    for (; iHeight--; bayer += iBayerStep, pucRGBInterImg += iRGBStep) {
      int t0, t1;
      const icl8u *bayerEnd = bayer + iWidth;
      
      if (start_with_green) {
        /* OpenCV has a bug in the next line, which was
           t0 = (bayer[0] + bayer[iBayerStep * 2] + 1) >> 1; */
        t0 = (bayer[1] + bayer[iBayerStep * 2 + 1] + 1) >> 1;
        t1 = (bayer[iBayerStep] + bayer[iBayerStep + 2] + 1) >> 1;
        pucRGBInterImg[-blue] = (icl8u) t0;
        pucRGBInterImg[0] = bayer[iBayerStep + 1];
        pucRGBInterImg[blue] = (icl8u) t1;
        bayer++;
        pucRGBInterImg += 3;
      }
      
      if (blue > 0) {
        for (; bayer <= bayerEnd - 2; bayer += 2, pucRGBInterImg += 6) {
          t0 = (bayer[0] + bayer[2] + bayer[iBayerStep * 2] +
                bayer[iBayerStep * 2 + 2] + 2) >> 2;
          t1 = (bayer[1] + bayer[iBayerStep] +
                bayer[iBayerStep + 2] + bayer[iBayerStep * 2 + 1] +
                2) >> 2;
          pucRGBInterImg[-1] = (icl8u) t0;
          pucRGBInterImg[0] = (icl8u) t1;
          pucRGBInterImg[1] = bayer[iBayerStep + 1];
          
          t0 = (bayer[2] + bayer[iBayerStep * 2 + 2] + 1) >> 1;
          t1 = (bayer[iBayerStep + 1] + bayer[iBayerStep + 3] +
                1) >> 1;
          pucRGBInterImg[2] = (icl8u) t0;
          pucRGBInterImg[3] = bayer[iBayerStep + 2];
          pucRGBInterImg[4] = (icl8u) t1;
        }
      } else {
        for (; bayer <= bayerEnd - 2; bayer += 2, pucRGBInterImg += 6) {
          t0 = (bayer[0] + bayer[2] + bayer[iBayerStep * 2] +
                bayer[iBayerStep * 2 + 2] + 2) >> 2;
          t1 = (bayer[1] + bayer[iBayerStep] +
                bayer[iBayerStep + 2] + bayer[iBayerStep * 2 + 1] +
                2) >> 2;
          pucRGBInterImg[1] = (icl8u) t0;
          pucRGBInterImg[0] = (icl8u) t1;
          pucRGBInterImg[-1] = bayer[iBayerStep + 1];
          
          t0 = (bayer[2] + bayer[iBayerStep * 2 + 2] + 1) >> 1;
          t1 = (bayer[iBayerStep + 1] + bayer[iBayerStep + 3] +
                1) >> 1;
          pucRGBInterImg[4] = (icl8u) t0;
          pucRGBInterImg[3] = bayer[iBayerStep + 2];
          pucRGBInterImg[2] = (icl8u) t1;
        }
      }
      
      if (bayer < bayerEnd) {
        t0 = (bayer[0] + bayer[2] + bayer[iBayerStep * 2] +
              bayer[iBayerStep * 2 + 2] + 2) >> 2;
        t1 = (bayer[1] + bayer[iBayerStep] +
              bayer[iBayerStep + 2] + bayer[iBayerStep * 2 + 1] +
              2) >> 2;
        pucRGBInterImg[-blue] = (icl8u) t0;
        pucRGBInterImg[0] = (icl8u) t1;
        pucRGBInterImg[blue] = bayer[iBayerStep + 1];
        bayer++;
        pucRGBInterImg += 3;
      }
      
      bayer -= iWidth;
      pucRGBInterImg -= iWidth * 3;
      
      blue = -blue;
      start_with_green = !start_with_green;
    }
  }

// }}}

  void BayerConverter::hqLinearInterpolation(const Img<icl8u> *poBayerImg) {
    // {{{ open

    // Variable initialization
    int iWidth = poBayerImg->getWidth();
    int iHeight = poBayerImg->getHeight();
    const int iBayerStep = iWidth;
    const int iRGBStep = 3 * iWidth;
    const icl8u *bayer = poBayerImg->getData(0);
    icl8u *pucRGBInterImg = &m_vRGBInterImg[0];
      
    int blue = m_eBayerPattern == bayerPattern_BGGR
      || m_eBayerPattern == bayerPattern_GBRG ? -1 : 1;
    int start_with_green = m_eBayerPattern == bayerPattern_GBRG
      || m_eBayerPattern == bayerPattern_GRBG;

    clearBorders(pucRGBInterImg, iWidth, iHeight, 1);
    pucRGBInterImg += 2 * iRGBStep + 6 + 1;
    iHeight -= 4;
    iWidth -= 4;

    /* We begin with a (+1 line,+1 column) offset with respect to bilinear decoding, so start_with_green is the same, but blue is opposite */
    blue = -blue;

    for (; iHeight--; bayer += iBayerStep, pucRGBInterImg += iRGBStep) {
      int t0, t1;
      const icl8u *bayerEnd = bayer + iWidth;
      const int iBayerStep2 = iBayerStep * 2;
      const int iBayerStep3 = iBayerStep * 3;
      const int iBayerStep4 = iBayerStep * 4;

      if (start_with_green) {
        /* at green pixel */
        pucRGBInterImg[0] = bayer[iBayerStep2 + 2];
        t0 = pucRGBInterImg[0] * 5
          + ((bayer[iBayerStep + 2] + bayer[iBayerStep3 + 2]) << 2)
          - bayer[2]
          - bayer[iBayerStep + 1]
          - bayer[iBayerStep + 3]
          - bayer[iBayerStep3 + 1]
          - bayer[iBayerStep3 + 3]
          - bayer[iBayerStep4 + 2]
          + ((bayer[iBayerStep2] + bayer[iBayerStep2 + 4] + 1) >> 1);
        t1 = pucRGBInterImg[0] * 5 +
          ((bayer[iBayerStep2 + 1] + bayer[iBayerStep2 + 3]) << 2)
          - bayer[iBayerStep2]
          - bayer[iBayerStep + 1]
          - bayer[iBayerStep + 3]
          - bayer[iBayerStep3 + 1]
          - bayer[iBayerStep3 + 3]
          - bayer[iBayerStep2 + 4]
          + ((bayer[2] + bayer[iBayerStep4 + 2] + 1) >> 1);
        t0 = (t0 + 4) >> 3;
        clip(&t0, &pucRGBInterImg[-blue]);
        t1 = (t1 + 4) >> 3;
        clip(&t1, &pucRGBInterImg[blue]);
        bayer++;
        pucRGBInterImg += 3;
      }

      if (blue > 0) {
        for (; bayer <= bayerEnd - 2; bayer += 2, pucRGBInterImg += 6) {
          /* B at B */
          pucRGBInterImg[1] = bayer[iBayerStep2 + 2];
          /* R at B */
          t0 = ((bayer[iBayerStep + 1] + bayer[iBayerStep + 3] +
                 bayer[iBayerStep3 + 1] + bayer[iBayerStep3 + 3]) << 1)
            -
            (((bayer[2] + bayer[iBayerStep2] +
               bayer[iBayerStep2 + 4] + bayer[iBayerStep4 +
                                             2]) * 3 + 1) >> 1)
            + pucRGBInterImg[1] * 6;
          /* G at B */
          t1 = ((bayer[iBayerStep + 2] + bayer[iBayerStep2 + 1] +
                 bayer[iBayerStep2 + 3] + bayer[iBayerStep3 + 2]) << 1)
            - (bayer[2] + bayer[iBayerStep2] +
               bayer[iBayerStep2 + 4] + bayer[iBayerStep4 + 2])
            + (pucRGBInterImg[1] << 2);
          t0 = (t0 + 4) >> 3;
          clip(&t0, &pucRGBInterImg[-1]);
          t1 = (t1 + 4) >> 3;
          clip(&t1, &pucRGBInterImg[0]);
          /* at green pixel */
          pucRGBInterImg[3] = bayer[iBayerStep2 + 3];
          t0 = pucRGBInterImg[3] * 5
            + ((bayer[iBayerStep + 3] + bayer[iBayerStep3 + 3]) << 2)
            - bayer[3]
            - bayer[iBayerStep + 2]
            - bayer[iBayerStep + 4]
            - bayer[iBayerStep3 + 2]
            - bayer[iBayerStep3 + 4]
            - bayer[iBayerStep4 + 3]
            +
            ((bayer[iBayerStep2 + 1] + bayer[iBayerStep2 + 5] +
              1) >> 1);
          t1 = pucRGBInterImg[3] * 5 +
            ((bayer[iBayerStep2 + 2] + bayer[iBayerStep2 + 4]) << 2)
            - bayer[iBayerStep2 + 1]
            - bayer[iBayerStep + 2]
            - bayer[iBayerStep + 4]
            - bayer[iBayerStep3 + 2]
            - bayer[iBayerStep3 + 4]
            - bayer[iBayerStep2 + 5]
            + ((bayer[3] + bayer[iBayerStep4 + 3] + 1) >> 1);
          t0 = (t0 + 4) >> 3;
          clip(&t0, &pucRGBInterImg[2]);
          t1 = (t1 + 4) >> 3;
          clip(&t1, &pucRGBInterImg[4]);
        }
      } else {
        for (; bayer <= bayerEnd - 2; bayer += 2, pucRGBInterImg += 6) {
          /* R at R */
          pucRGBInterImg[-1] = bayer[iBayerStep2 + 2];
          /* B at R */
          t0 = ((bayer[iBayerStep + 1] + bayer[iBayerStep + 3] +
                 bayer[iBayerStep3 + 1] + bayer[iBayerStep3 + 3]) << 1)
            -
            (((bayer[2] + bayer[iBayerStep2] +
               bayer[iBayerStep2 + 4] + bayer[iBayerStep4 +
                                             2]) * 3 + 1) >> 1)
            + pucRGBInterImg[-1] * 6;
          /* G at R */
          t1 = ((bayer[iBayerStep + 2] + bayer[iBayerStep2 + 1] +
                 bayer[iBayerStep2 + 3] + bayer[iBayerStep * 3 +
                                               2]) << 1)
            - (bayer[2] + bayer[iBayerStep2] +
               bayer[iBayerStep2 + 4] + bayer[iBayerStep4 + 2])
            + (pucRGBInterImg[-1] << 2);
          t0 = (t0 + 4) >> 3;
          clip(&t0, &pucRGBInterImg[1]);
          t1 = (t1 + 4) >> 3;
          clip(&t1, &pucRGBInterImg[0]);

          /* at green pixel */
          pucRGBInterImg[3] = bayer[iBayerStep2 + 3];
          t0 = pucRGBInterImg[3] * 5
            + ((bayer[iBayerStep + 3] + bayer[iBayerStep3 + 3]) << 2)
            - bayer[3]
            - bayer[iBayerStep + 2]
            - bayer[iBayerStep + 4]
            - bayer[iBayerStep3 + 2]
            - bayer[iBayerStep3 + 4]
            - bayer[iBayerStep4 + 3]
            +
            ((bayer[iBayerStep2 + 1] + bayer[iBayerStep2 + 5] +
              1) >> 1);
          t1 = pucRGBInterImg[3] * 5 +
            ((bayer[iBayerStep2 + 2] + bayer[iBayerStep2 + 4]) << 2)
            - bayer[iBayerStep2 + 1]
            - bayer[iBayerStep + 2]
            - bayer[iBayerStep + 4]
            - bayer[iBayerStep3 + 2]
            - bayer[iBayerStep3 + 4]
            - bayer[iBayerStep2 + 5]
            + ((bayer[3] + bayer[iBayerStep4 + 3] + 1) >> 1);
          t0 = (t0 + 4) >> 3;
          clip(&t0, &pucRGBInterImg[4]);
          t1 = (t1 + 4) >> 3;
          clip(&t1, &pucRGBInterImg[2]);
        }
      }

      if (bayer < bayerEnd) {
        /* B at B */
        pucRGBInterImg[blue] = bayer[iBayerStep2 + 2];
        /* R at B */
        t0 = ((bayer[iBayerStep + 1] + bayer[iBayerStep + 3] +
               bayer[iBayerStep3 + 1] + bayer[iBayerStep3 + 3]) << 1)
          -
          (((bayer[2] + bayer[iBayerStep2] +
             bayer[iBayerStep2 + 4] + bayer[iBayerStep4 +
                                           2]) * 3 + 1) >> 1)
          + pucRGBInterImg[blue] * 6;
        /* G at B */
        t1 = (((bayer[iBayerStep + 2] + bayer[iBayerStep2 + 1] +
                bayer[iBayerStep2 + 3] + bayer[iBayerStep3 + 2])) << 1)
          - (bayer[2] + bayer[iBayerStep2] +
             bayer[iBayerStep2 + 4] + bayer[iBayerStep4 + 2])
          + (pucRGBInterImg[blue] << 2);
        t0 = (t0 + 4) >> 3;
        clip(&t0, &pucRGBInterImg[-blue]);
        t1 = (t1 + 4) >> 3;
        clip(&t1, &pucRGBInterImg[0]);
        bayer++;
        pucRGBInterImg += 3;
      }

      bayer -= iWidth;
      pucRGBInterImg -= iWidth * 3;

      blue = -blue;
      start_with_green = !start_with_green;
    }
  }

// }}}
  
  void BayerConverter::edgeSenseInterpolation(const Img<icl8u> *poBayerImg) {
    // {{{ open

    // Variable initialization
    icl8u *outR, *outG, *outB;
    int iWidth = poBayerImg->getWidth();
    int iHeight = poBayerImg->getHeight();
    const icl8u *bayer = poBayerImg->getData(0);
    icl8u *pucRGBInterImg = &m_vRGBInterImg[0];
    register int i3, j3, base;
    int i, j, dh, dv, tmp;
    int iWidth3 = iWidth*3;
    
    // iWidth and iHeight should be even
    switch (m_eBayerPattern) {
      case bayerPattern_GRBG:
      case bayerPattern_BGGR:
        outR = &pucRGBInterImg[0];
        outG = &pucRGBInterImg[1];
        outB = &pucRGBInterImg[2];
        break;
      case bayerPattern_GBRG:
      case bayerPattern_RGGB:
        outR = &pucRGBInterImg[2];
        outG = &pucRGBInterImg[1];
        outB = &pucRGBInterImg[0];
        break;
      default:
        ERROR_LOG("Bad bayer pattern ID:" << m_eBayerPattern);
        return;
        break;
    }
    
    switch (m_eBayerPattern) {
      case bayerPattern_GRBG:	
      case bayerPattern_GBRG:
        // copy original RGB data to output images
        for (i = 0, i3=0; 
             i < iHeight*iWidth; i += (iWidth<<1), i3 += (iWidth3<<1)) {
          for (j = 0, j3=0; j < iWidth; j += 2, j3+=6) {
            base=i3+j3;
            outG[base]           = bayer[i + j];
            outG[base + iWidth3 + 3] = bayer[i + j + iWidth + 1];
            outR[base + 3]       = bayer[i + j + 1];
            outB[base + iWidth3]     = bayer[i + j + iWidth];
          }
        }
        // process GREEN channel
        for (i3= 3*iWidth3; i3 < (iHeight - 2)*iWidth3; i3 += (iWidth3<<1)) {
          for (j3=6; j3 < iWidth3 - 9; j3+=6) {
            base=i3+j3;
            dh = abs(((outB[base - 6] +
                       outB[base + 6]) >> 1) -
                     outB[base]);
            dv = abs(((outB[base - (iWidth3<<1)] +
                       outB[base + (iWidth3<<1)]) >> 1) -
                     outB[base]);
            tmp = (((outG[base - 3]   + outG[base + 3]) >> 1) * (dh<=dv) +
                   ((outG[base - iWidth3] + outG[base + iWidth3]) >> 1) * 
                   (dh>dv));
            //tmp = (dh==dv) ? tmp>>1 : tmp;
            clip(&tmp, &outG[base]);
          }
        }
	
        for (i3=2*iWidth3; i3 < (iHeight - 3)*iWidth3; i3 += (iWidth3<<1)) {
          for (j3=9; j3 < iWidth3 - 6; j3+=6) {
            base=i3+j3;
            dh = abs(((outR[base - 6] +
                       outR[base + 6]) >>1 ) -
                     outR[base]);
            dv = abs(((outR[base - (iWidth3<<1)] +
                       outR[base + (iWidth3<<1)]) >>1 ) -
                     outR[base]);
            tmp = (((outG[base - 3]   + outG[base + 3]) >> 1) * (dh<=dv) +
                   ((outG[base - iWidth3] + outG[base + iWidth3]) >> 1) * 
                   (dh>dv));
            //tmp = (dh==dv) ? tmp>>1 : tmp;
            clip(&tmp, &outG[base]);
          }
        }
        // process RED channel
        for (i3=0; i3 < (iHeight - 1)*iWidth3; i3 += (iWidth3<<1)) {
          for (j3=6; j3 < iWidth3 - 3; j3+=6) {
            base=i3+j3;
            tmp = outG[base] +
              ((outR[base - 3] -
                outG[base - 3] +
                outR[base + 3] -
                outG[base + 3]) >> 1);
            clip(&tmp, &outR[base]);
          }
        }
        for (i3=iWidth3; i3 < (iHeight - 2)*iWidth3; i3 += (iWidth3<<1)) {
          for (j3=3; j3 < iWidth3; j3+=6) {
            base=i3+j3;
            tmp = outG[base] +
              ((outR[base - iWidth3] -
                outG[base - iWidth3] +
                outR[base + iWidth3] -
                outG[base + iWidth3]) >> 1);
            clip(&tmp, &outR[base]);
          }
          for (j3=6; j3 < iWidth3 - 3; j3+=6) {
            base=i3+j3;
            tmp = outG[base] +
              ((outR[base - iWidth3 - 3] -
                outG[base - iWidth3 - 3] +
                outR[base - iWidth3 + 3] -
                outG[base - iWidth3 + 3] +
                outR[base + iWidth3 - 3] -
                outG[base + iWidth3 - 3] +
                outR[base + iWidth3 + 3] -
                outG[base + iWidth3 + 3]) >> 2);
            clip(&tmp, &outR[base]);
          }
        }

        // process BLUE channel
        for (i3=iWidth3; i3 < iHeight*iWidth3; i3 += (iWidth3<<1)) {
          for (j3=3; j3 < iWidth3 - 6; j3+=6) {
            base=i3+j3;
            tmp = outG[base] +
              ((outB[base - 3] -
                outG[base - 3] +
                outB[base + 3] -
                outG[base + 3]) >> 1);
            clip(&tmp, &outB[base]);
          }
        }
        for (i3=2*iWidth3; i3 < (iHeight - 1)*iWidth3; i3 += (iWidth3<<1)) {
          for (j3=0; j3 < iWidth3 - 3; j3+=6) {
            base=i3+j3;
            tmp = outG[base] +
              ((outB[base - iWidth3] -
                outG[base - iWidth3] +
                outB[base + iWidth3] -
                outG[base + iWidth3]) >> 1);
            clip(&tmp, &outB[base]);
          }
          for (j3=3; j3 < iWidth3 - 6; j3+=6) {
            base=i3+j3;
            tmp = outG[base] +
              ((outB[base - iWidth3 - 3] -
                outG[base - iWidth3 - 3] +
                outB[base - iWidth3 + 3] -
                outG[base - iWidth3 + 3] +
                outB[base + iWidth3 - 3] -
                outG[base + iWidth3 - 3] +
                outB[base + iWidth3 + 3] -
                outG[base + iWidth3 + 3]) >> 2);
            clip(&tmp, &outB[base]);
          }
        }
        break;

      case bayerPattern_BGGR:
      case bayerPattern_RGGB:
        // copy original RGB data to output images
        for (i = 0, i3=0; 
             i < iHeight*iWidth; i += (iWidth<<1), i3 += (iWidth3<<1)) {
          for (j = 0, j3=0; j < iWidth; j += 2, j3+=6) {
            base=i3+j3;
            outB[base] = bayer[i + j];
            outR[base + iWidth3 + 3] = bayer[i + iWidth + (j + 1)];
            outG[base + 3] = bayer[i + j + 1];
            outG[base + iWidth3] = bayer[i + iWidth + j];
          }
        }
        // process GREEN channel
        for (i3=2*iWidth3; i3 < (iHeight - 2)*iWidth3; i3 += (iWidth3<<1)) {
          for (j3=6; j3 < iWidth3 - 9; j3+=6) {
            base=i3+j3;
            dh = abs(((outB[base - 6] +
                       outB[base + 6]) >> 1) -
                     outB[base]);
            dv = abs(((outB[base - (iWidth3<<1)] +
                       outB[base + (iWidth3<<1)]) >> 1) -
                     outB[base]);
            tmp = (((outG[base - 3]   + outG[base + 3]) >> 1) * (dh<=dv) +
                   ((outG[base - iWidth3] + outG[base + iWidth3]) >> 1) * 
                   (dh>dv));
            clip(&tmp, &outG[base]);
          }
        }
        for (i3=3*iWidth3; i3 < (iHeight - 3)*iWidth3; i3 += (iWidth3<<1)) {
          for (j3=9; j3 < iWidth3 - 6; j3+=6) {
            base=i3+j3;
            dh = abs(((outR[base - 6] +
                       outR[base + 6]) >> 1) -
                     outR[base]);
            dv = abs(((outR[base - (iWidth3<<1)] +
                       outR[base + (iWidth3<<1)]) >> 1) -
                     outR[base]);
            tmp = (((outG[base - 3]   + outG[base + 3]) >> 1) * (dh<=dv) +
                   ((outG[base - iWidth3] + outG[base + iWidth3]) >> 1) * 
                   (dh>dv));
            clip(&tmp, &outG[base]);
          }
        }
        // process RED channel
        for (i3=iWidth3; i3 < (iHeight - 1)*iWidth3; i3 += (iWidth3<<1)) {
          for (j3=6; j3 < iWidth3 - 3; j3+=6) {
            base=i3+j3;
            tmp = outG[base] +
              ((outR[base - 3] -
                outG[base - 3] +
                outR[base + 3] -
                outG[base + 3]) >>1);
            clip(&tmp, &outR[base]);
          }
        }
        for (i3=2*iWidth3; i3 < (iHeight - 2)*iWidth3; i3 += (iWidth3<<1)) {
          for (j3=3; j3 < iWidth3; j3+=6) {
            base=i3+j3;
            tmp = outG[base] +
              ((outR[base - iWidth3] -
                outG[base - iWidth3] +
                outR[base + iWidth3] -
                outG[base + iWidth3]) >> 1);
            clip(&tmp, &outR[base]);
          }
          for (j3=6; j3 < iWidth3 - 3; j3+=6) {
            base=i3+j3;
            tmp = outG[base] +
              ((outR[base - iWidth3 - 3] -
                outG[base - iWidth3 - 3] +
                outR[base - iWidth3 + 3] -
                outG[base - iWidth3 + 3] +
                outR[base + iWidth3 - 3] -
                outG[base + iWidth3 - 3] +
                outR[base + iWidth3 + 3] -
                outG[base + iWidth3 + 3]) >> 2);
            clip(&tmp, &outR[base]);
          }
        }
      
        // process BLUE channel
        for (i = 0,i3=0; 
             i < iHeight*iWidth; i += (iWidth<<1), i3 += (iWidth3<<1)) {
          for (j = 1, j3=3; j < iWidth - 2; j += 2, j3+=6) {
            base=i3+j3;
            tmp = outG[base] +
              ((outB[base - 3] -
                outG[base - 3] +
                outB[base + 3] -
                outG[base + 3]) >> 1);
            clip(&tmp, &outB[base]);
          }
        }
        for (i3=iWidth3; i3 < (iHeight - 1)*iWidth3; i3 += (iWidth3<<1)) {
          for (j3=0; j3 < iWidth3 - 3; j3+=6) {
            base=i3+j3;
            tmp = outG[base] +
              ((outB[base - iWidth3] -
                outG[base - iWidth3] +
                outB[base + iWidth3] -
                outG[base + iWidth3]) >> 1);
            clip(&tmp, &outB[base]);
          }
          for (j3=3; j3 < iWidth3 - 6; j3+=6) {
            base=i3+j3;
            tmp = outG[base] +
              ((outB[base - iWidth3 - 3] -
                outG[base - iWidth3 - 3] +
                outB[base - iWidth3 + 3] -
                outG[base - iWidth3 + 3] +
                outB[base + iWidth3 - 3] -
                outG[base + iWidth3 - 3] +
                outB[base + iWidth3 + 3] -
                outG[base + iWidth3 + 3]) >> 2);
            clip(&tmp, &outB[base]);
          }
        }
        break;
      default:
        ERROR_LOG("Bad bayer pattern ID: " << m_eBayerPattern);
        return;
        break;
    }
    
    clearBorders(pucRGBInterImg, iWidth, iHeight, 3);
  }

// }}}
  
  void BayerConverter::simpleInterpolation(const Img<icl8u> *poBayerImg) {
    // {{{ open

    // Variable initialization
    int iWidth = poBayerImg->getWidth();
    int iHeight = poBayerImg->getHeight();
    int i, imax, iinc;
    const int iBayerStep = iWidth;
    const int iRGBStep = 3 * iWidth;
    const icl8u *bayer = poBayerImg->getData(0);
    icl8u *pucRGBInterImg = &m_vRGBInterImg[0];
    
    int blue = m_eBayerPattern == bayerPattern_BGGR
      || m_eBayerPattern == bayerPattern_GBRG ? -1 : 1;
    int start_with_green = m_eBayerPattern == bayerPattern_GBRG
      || m_eBayerPattern == bayerPattern_GRBG;
    
    /* add black border */
    imax = iWidth * iHeight * 3;
    for (i = iWidth * (iHeight - 1) * 3; i < imax; i++) {
      pucRGBInterImg[i] = 0;
    }
    iinc = (iWidth - 1) * 3;
    for (i = (iWidth - 1) * 3; i < imax; i += iinc) {
      pucRGBInterImg[i++] = 0;
      pucRGBInterImg[i++] = 0;
      pucRGBInterImg[i++] = 0;
    }

    pucRGBInterImg += 1;
    iWidth -= 1;
    iHeight -= 1;

    for (; iHeight--; bayer += iBayerStep, pucRGBInterImg += iRGBStep) {
      const unsigned char *bayerEnd = bayer + iWidth;

      if (start_with_green) {
        pucRGBInterImg[-blue] = bayer[1];
        pucRGBInterImg[0] = (bayer[0] + bayer[iBayerStep + 1] + 1) >> 1;
        pucRGBInterImg[blue] = bayer[iBayerStep];
        bayer++;
        pucRGBInterImg += 3;
      }

      if (blue > 0) {
        for (; bayer <= bayerEnd - 2; bayer += 2, pucRGBInterImg += 6) {
          pucRGBInterImg[-1] = bayer[0];
          pucRGBInterImg[0] = (bayer[1] + bayer[iBayerStep] + 1) >> 1;
          pucRGBInterImg[1] = bayer[iBayerStep + 1];

          pucRGBInterImg[2] = bayer[2];
          pucRGBInterImg[3] = (bayer[1] + bayer[iBayerStep + 2] + 1) >> 1;
          pucRGBInterImg[4] = bayer[iBayerStep + 1];
        }
      } else {
        for (; bayer <= bayerEnd - 2; bayer += 2, pucRGBInterImg += 6) {
          pucRGBInterImg[1] = bayer[0];
          pucRGBInterImg[0] = (bayer[1] + bayer[iBayerStep] + 1) >> 1;
          pucRGBInterImg[-1] = bayer[iBayerStep + 1];

          pucRGBInterImg[4] = bayer[2];
          pucRGBInterImg[3] = (bayer[1] + bayer[iBayerStep + 2] + 1) >> 1;
          pucRGBInterImg[2] = bayer[iBayerStep + 1];
        }
      }

      if (bayer < bayerEnd) {
        pucRGBInterImg[-blue] = bayer[0];
        pucRGBInterImg[0] = (bayer[1] + bayer[iBayerStep] + 1) >> 1;
        pucRGBInterImg[blue] = bayer[iBayerStep + 1];
        bayer++;
        pucRGBInterImg += 3;
      }

      bayer -= iWidth;
      pucRGBInterImg -= iWidth * 3;

      blue = -blue;
      start_with_green = !start_with_green;
    }
  }

// }}}
  
  void BayerConverter::clearBorders(icl8u *rgb, int iWidth, 
                                    int iHeight, int w) {
    // {{{ open

    //Variable initialization
    int i, j;
    
    // black edges are added with a width w:
    i = 3 * iWidth * w - 1;
    j = 3 * iWidth * iHeight - 1;
    while (i >= 0) {
      rgb[i--] = 0;
      rgb[j--] = 0;
    }
    i = iWidth * (iHeight - 1) * 3 - 1 + w * 3;
    while (i > iWidth) {
      j = 6 * w;
      while (j > 0) {
        rgb[i--] = 0;
        j--;
      }
      i -= (iWidth - 2 * w) * 3;
    }
  }

  std::string BayerConverter::translateBayerConverterMethod(BayerConverter::bayerConverterMethod ebcm) {
	switch(ebcm){
		case nearestNeighbor: return "nearestNeighbor";
		case simple: return "simple";
		case bilinear: return "bilinear";
		case hqLinear: return "hqLinear";
		case edgeSense: return "edgeSense";
		case vng: return "vng";
		default: ICL_INVALID_FORMAT; return "undefined format";        
    }
  }

  BayerConverter::bayerConverterMethod BayerConverter::translateBayerConverterMethod(std::string sbcm) {
	if(sbcm.length()<=0){
      ICL_INVALID_FORMAT;
      return nearestNeighbor;
    }
    switch(sbcm[0]){
      case 'n': return nearestNeighbor;
      case 's': return simple;
      case 'b': return bilinear;
      case 'h': return hqLinear;
      case 'e': return edgeSense;
      case 'v': return vng;
      default: ICL_INVALID_FORMAT; return nearestNeighbor;
    }
  }

  std::string BayerConverter::translateBayerPattern(BayerConverter::bayerPattern ebp) {
	switch(ebp){
		case bayerPattern_RGGB: return "RGGB";
		case bayerPattern_GBRG: return "GBRG";
		case bayerPattern_GRBG: return "GRBG";
		case bayerPattern_BGGR: return "BGGR";
		default: ICL_INVALID_FORMAT; return "undefined format";        
    }
  }

  BayerConverter::bayerPattern BayerConverter::translateBayerPattern(std::string sbp) {
	if(sbp.length()<=0){
      ICL_INVALID_FORMAT;
      return bayerPattern_RGGB;
    }
	if (sbp == "RGGB") {
		return bayerPattern_RGGB;
	}
	if (sbp == "GBRG") {
		return bayerPattern_GBRG;
	}
	if (sbp == "GRBG") {
		return bayerPattern_GRBG;
	}
	if (sbp == "BGGR") {
		return bayerPattern_BGGR;
	}
	ICL_INVALID_FORMAT; 
	return bayerPattern_RGGB;
  }

// }}}
} // namespace
