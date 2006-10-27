/*
  Skin.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "Skin.h"
#include "Img.h"
#include "Converter.h"
#include "Mathematics.h"

namespace icl {
/*
  Skin::Skin() {
    m_poChromaApply = 0;
    m_poChromaTrain = 0;
  }

  Skin::Skin(std::vector<float> skinParams) {
    m_poChromaApply = 0;
    m_poChromaTrain = 0;
    m_vecSkinParams = skinParams;
  }
*/
  void Skin::apply(ImgI *poSrc, ImgI **ppoDst) {
    // {{{ open 
    FUNCTION_LOG("");

    //Variable initialisation
    float p1, p2, rg_x, rg_y;
    
    //Ensure ImgI compatibility
    ensureCompatible(&m_poChromaApply, depth8u, 
                     poSrc->getSize(), formatChroma);
    ensureCompatible(ppoDst, depth8u, poSrc->getSize(), 
                     formatMatrix, poSrc->getROI());
    
    //Convert src image
    Converter().convert(m_poChromaApply, poSrc);
    (*ppoDst)->asImg<icl8u>()->clear();
    
    //Apply skin color detection
    Rect oROI = poSrc->getROI();
    
    int iW = oROI.x + oROI.width;
    int iH = oROI.y + oROI.height;
            
    for (int y=oROI.y;y<iH;y++) {
      for(int x=oROI.x;x<iW;x++) {
        rg_x = (*m_poChromaApply->asImg<icl8u>())(x,y,0);
        rg_y = (*m_poChromaApply->asImg<icl8u>())(x,y,1);

        //---- evaluate parabolas ----
        p1 = m_vecSkinParams[2] * 
          ((rg_x - m_vecSkinParams[0] ) * (rg_x - m_vecSkinParams[0] )) + 
          m_vecSkinParams[1];
        
        p2 = m_vecSkinParams[5] * 
          ((rg_x - m_vecSkinParams[3] ) * (rg_x - m_vecSkinParams[3] )) + 
          m_vecSkinParams[4];

        if ((p1 < rg_y) && (p2 > rg_y))
        {
          (*(*ppoDst)->asImg<icl8u>())(x,y,0) = 255;
        }
      }
    }
  }
  
// }}}

  void Skin::train(ImgI *poSrc) {
    // {{{ open 
    FUNCTION_LOG("");

    //Variable initialisation
    std::vector<float> vecTmpParams(6);

    //Ensure ImgI compatibility
    ensureCompatible(&m_poChromaTrain, 
                     depth8u, poSrc->getSize(), 
                     formatChroma);
    
    //Convert src image
    Converter().convert(m_poChromaTrain, poSrc);

    //Compute start parameter
    std::vector<float> vecChromaMean = icl::mean(m_poChromaTrain);
    std::vector<float> vecChromaDev = icl::stdDeviation(m_poChromaTrain);
    
    //---- Compute the parabola parameter for parabola 1 ----
    vecTmpParams[0] = vecChromaMean[0];
    vecTmpParams[1] = vecChromaMean[1] - vecChromaDev[1];
    vecTmpParams[2] = 0.5;
    
    SECTION_LOG("Parameter Parabola 1:");
    SECTION_LOG("---------------------");
    SECTION_LOG("xO: " << vecTmpParams[0]);
    SECTION_LOG("yO: " << vecTmpParams[1]);
    SECTION_LOG("a : " << vecTmpParams[2]);
    
    //---- Compute the parabola parameter for parabola 2 ----
    vecTmpParams[3] = vecChromaMean[0];
    vecTmpParams[4] = vecChromaMean[1] + vecChromaDev[1];
    vecTmpParams[5] = -0.5;
    
    SECTION_LOG("Parameter Parabola 2:");
    SECTION_LOG("---------------------");
    SECTION_LOG("x0: " << vecTmpParams[3]);
    SECTION_LOG("y0: " << vecTmpParams[4]);
    SECTION_LOG("a : " << vecTmpParams[5]);
    
    //Gibbs parameter optimization

    //Set new parameter as default
    m_vecSkinParams = vecTmpParams;
  }
  // }}}

} //namespace icl

