#include <iclSkinOp.h>
#include <iclImg.h>
#include <iclCC.h>
#include <iclMathematics.h>

/*
  Skin.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/


using namespace std;

namespace icl {

  void SkinOp::apply(const ImgBase *poSrc, ImgBase **ppoDst) {
    // {{{ open 
    FUNCTION_LOG("");

    //Variable initialisation
    float p1, p2;
    //Ensure ImgBase compatibility
    ensureCompatible(&m_poChromaApply, depth32f, 
                     poSrc->getSize(), formatChroma);
    ensureCompatible(ppoDst, depth8u, poSrc->getSize(), 
                     formatMatrix, poSrc->getROI());

    //Convert src image
    cc(poSrc,m_poChromaApply);
    (*ppoDst)->asImg<icl8u>()->clear();
    

    Img32f::iterator itChromaR=m_poChromaApply->asImg<icl32f>()->getIterator(0);
    Img32f::iterator itChromaG=m_poChromaApply->asImg<icl32f>()->getIterator(1);
    Img8u::iterator  itOutMask= (*ppoDst)->asImg<icl8u>()->getIterator(0);


    
    for (; itChromaR.inRegion(); ++itChromaR, ++itChromaG, ++itOutMask) {
      //---- evaluate parabolas ----
      p1 = m_vecSkinParams[2] * 
        ((*itChromaR - m_vecSkinParams[0])*(*itChromaR - m_vecSkinParams[0])) +
        m_vecSkinParams[1];
      
      p2 = m_vecSkinParams[5] * 
        ((*itChromaR - m_vecSkinParams[3])*(*itChromaR - m_vecSkinParams[3])) + 
        m_vecSkinParams[4];
      
      LOOP_LOG("Parabola value p1: " << p1);
      LOOP_LOG("Parabola value p2: " << p2);
      LOOP_LOG("p1 < chromaG     : " << p1 << " < " << *itChromaG);
      LOOP_LOG("p2 > chromaG     : " << p2 << " > " << *itChromaG);
      
      if ((*itChromaG > p1) && (*itChromaG < p2))
      {
        *itOutMask = 255;
      }
    }
  }

// }}}

  void SkinOp::apply(const ImgBase *poSrc, ImgBase **ppoDst,
                   ImgBase **ppoPara1, ImgBase **ppoPara2,
                   ImgBase **ppoChromaVal, 
                   ImgBase **ppoChromaSkinMask) {
    // {{{ open 
    FUNCTION_LOG("");
    
    //Variable initialisation
    float p1, p2, rg_x, rg_y;
    
    //Ensure ImgBase compatibility
    ensureCompatible(&m_poChromaApply, depth32f, 
                     poSrc->getSize(), formatChroma);
    ensureCompatible(ppoDst, depth8u, poSrc->getSize(), 
                     formatMatrix, poSrc->getROI());
    ensureCompatible(ppoPara1, depth32f, 
                     poSrc->getSize(), 2, formatMatrix);
    ensureCompatible(ppoPara2, depth32f, 
                     poSrc->getSize(), 2, formatMatrix);
    ensureCompatible(ppoChromaVal, depth32f, 
                     poSrc->getSize(), 2, formatMatrix);
    ensureCompatible(ppoChromaSkinMask, depth32f, 
                     poSrc->getSize(), 2, formatMatrix);

    //Convert src image
    cc(poSrc,m_poChromaApply);
    (*ppoDst)->asImg<icl8u>()->clear();
    
    //Apply skin color detection
    Rect oROI = poSrc->getROI();
    
    int iW = oROI.x + oROI.width;
    int iH = oROI.y + oROI.height;
    
    for (int y=oROI.y;y<iH;y++) {
      for(int x=oROI.x;x<iW;x++) {
        LOOP_LOG("@Position ("<<x<<","<<y<<")");
        rg_x = (*m_poChromaApply->asImg<icl32f>())(x,y,0);
        rg_y = (*m_poChromaApply->asImg<icl32f>())(x,y,1);
        (*(*ppoChromaVal)->asImg<icl32f>())(x,y,0) = rg_x;
        (*(*ppoChromaVal)->asImg<icl32f>())(x,y,1) = rg_y;
        
        //---- evaluate parabolas ----
        p1 = m_vecSkinParams[2] * 
          ((rg_x - m_vecSkinParams[0] ) * (rg_x - m_vecSkinParams[0] )) + 
          m_vecSkinParams[1];
        (*(*ppoPara1)->asImg<icl32f>())(x,y,0) = rg_x;
        (*(*ppoPara1)->asImg<icl32f>())(x,y,1) = p1;

        p2 = m_vecSkinParams[5] * 
          ((rg_x - m_vecSkinParams[3] ) * (rg_x - m_vecSkinParams[3] )) + 
          m_vecSkinParams[4];
        (*(*ppoPara2)->asImg<icl32f>())(x,y,0) = rg_x;
        (*(*ppoPara2)->asImg<icl32f>())(x,y,1) = p2;
        
        LOOP_LOG("Parabola value p1: " << p1);
        LOOP_LOG("Parabola value p2: " << p2);
        LOOP_LOG("p1 < rg_y        : " << p1 << " < " << rg_y);
        LOOP_LOG("p2 > rg_y        : " << p2 << " > " << rg_y);
        
        if ((rg_y > p1) && (rg_y < p2))
        {
          SUBSECTION_LOG("Skin color at pos("<<x<<","<<y<<")");
          (*(*ppoChromaSkinMask)->asImg<icl32f>())(x,y,0) = rg_x;
          (*(*ppoChromaSkinMask)->asImg<icl32f>())(x,y,1) = rg_y;
          (*(*ppoDst)->asImg<icl8u>())(x,y,0) = 255;
        }
      }
    }
  }
  
// }}}

  void SkinOp::train(const ImgBase *poSrc, const ImgBase *poMask) {
    // {{{ open 
    
    FUNCTION_LOG("");
    
    // Variable initialisation
    std::vector<float> vecTmpParams(6);
    
    //Convert src image
    ensureCompatible(&m_poChromaTrain, 
                     depth32f, poSrc->getSize(), 
                     formatChroma);
    cc(poSrc, m_poChromaTrain);
    
    //Compute start parameter
    std::vector<float> vecChromaMean = icl::mean(m_poChromaTrain);
    std::vector<float> vecChromaDev = icl::deviation(m_poChromaTrain);
    
    //---- Compute the parabola parameter for parabola 1 ----
    vecTmpParams[0] = vecChromaMean[0];
    vecTmpParams[1] = vecChromaMean[1] - ( 2.3 * vecChromaDev[1] );
    vecTmpParams[2] = 0.03;
    
    SECTION_LOG("Parameter Parabola 1:");
    SECTION_LOG("---------------------");
    SECTION_LOG("xO: " << vecTmpParams[0]);
    SECTION_LOG("yO: " << vecTmpParams[1]);
    SECTION_LOG("a0: " << vecTmpParams[2]);
    
    //---- Compute the parabola parameter for parabola 2 ----
    vecTmpParams[3] = vecChromaMean[0];
    vecTmpParams[4] = vecChromaMean[1] + ( 2.3 * vecChromaDev[1] );
    vecTmpParams[5] = -0.03;
    
    SECTION_LOG("Parameter Parabola 2:");
    SECTION_LOG("---------------------");
    SECTION_LOG("x1: " << vecTmpParams[3]);
    SECTION_LOG("y1: " << vecTmpParams[4]);
    SECTION_LOG("a1: " << vecTmpParams[5]);
    
    //Set new parameter as default
    m_vecSkinParams = vecTmpParams;

  }

  // }}}

} //namespace icl

