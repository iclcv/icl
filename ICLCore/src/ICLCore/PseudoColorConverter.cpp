/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/PseudoColorConverter.cpp           **
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

#include <ICLCore/PseudoColorConverter.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/ConfigFile.h>

using namespace icl::utils;

namespace icl{
  namespace core{
    
    static const icl8u defLut[3][256]={{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,6,9,12,15,19,22,25,28,31,35,38,41,44,
                                        48,51,54,57,60,64,67,70,73,77,80,83,86,90,93,96,99,102,106,109,112,115,
                                        119,122,125,128,131,135,138,141,144,148,151,154,157,160,164,167,170,173,
                                        177,180,183,186,190,193,196,199,202,206,209,212,215,219,222,225,228,231,
                                        235,238,241,244,248,251,254,255,255,255,255,255,255,255,255,255,255,255,
                                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                        255,255,255,255,255,255,255,255,255,255,255,255,250,246,241,237,232,228,
                                        223,218,214,209,205,200,196,191,187,182,178,173,168,164,159,155,150,146,
                                        141,137,132,128,128},
                                       {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8,12,
                                        17,21,25,29,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,
                                        108,112,116,120,124,128,133,136,141,144,149,152,157,160,165,168,173,176,
                                        181,184,189,192,197,200,205,208,213,216,221,224,229,232,237,240,245,248,
                                        253,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,252,248,245,
                                        241,237,234,230,226,222,219,215,211,208,204,200,196,193,189,185,182,178,
                                        174,171,167,163,159,156,152,148,145,141,137,134,130,126,122,119,115,111,
                                        108,104,100,96,93,89,85,82,78,74,71,67,63,59,56,52,48,45,41,37,34,30,26,
                                        22,19,15,11,8,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                       {132,137,141,146,150,155,159,164,168,173,177,182,187,191,196,200,205,209,
                                        214,218,223,227,232,237,241,246,250,255,255,255,255,255,255,255,255,255,
                                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,251,248,244,
                                        241,238,235,231,228,225,222,219,215,212,209,206,202,199,196,193,190,186,
                                        183,180,177,173,170,167,164,160,157,154,151,148,144,141,138,135,131,128,
                                        125,122,119,115,112,109,106,102,99,96,93,90,86,83,80,77,73,70,67,64,60,57,
                                        54,51,48,44,41,38,35,31,28,25,22,19,15,12,9,6,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0}};
    
    bool cmp_stop(const PseudoColorConverter::Stop &a, const PseudoColorConverter::Stop &b){
      return a.relPos < b.relPos;
    }
    
    struct PseudoColorConverter::Data{
      std::vector<icl8u> luts[3]; // one Lookup-Table per channel RGB
      std::vector<Stop> stops;
      PseudoColorConverter::ColorTable mode;
      Img8u outputBuffer;
      Img8u sourceBuffer;
      int maxVal;

      template<class T>
      void lut(const Img<T> &src, Img8u &dst){
        const T *s = src.begin(0);
        icl8u *r = dst.begin(0), *g = dst.begin(1), *b=dst.begin(2);
        const icl8u *rlut = luts[0].data();
        const icl8u *glut = luts[1].data();
        const icl8u *blut = luts[2].data();
        
        const int dim = src.getDim();
        for(int i=0;i<dim;++i){
          const int si = s[i];
          r[i] = rlut[si];
          g[i] = glut[si];
          b[i] = blut[si];
        }
      }
      
      void def(int maxValue){
        maxVal = maxValue;
        std::vector<Stop> s(256);
        for(int i=0;i<256;++i){
          s[i] = Stop(i/255.0f,Color(defLut[0][i],defLut[1][i],defLut[2][i]));
        }
        custom(s,maxValue);
        mode = PseudoColorConverter::Default;
      }
      
      void fill_lin(icl8u *p, int a, int b, float c1, float c2){
        for(int i=a;i<b;++i){
          float f = float(i-a)/(b-a);
          p[i] = f*c2 + (1.0f-f)*c1;
        }
      }
      
      void custom(const std::vector<Stop> &stopsIn, int maxValue) throw (ICLException){
        maxVal = maxValue;
        if(!stopsIn.size()) throw ICLException("PseudoColorConverter: no stops found");
        mode = PseudoColorConverter::Custom;
        stops = stopsIn;
        if(stops.size() == 1){
          std::fill(luts[0].begin(),luts[0].end(),stops[0].color[0]);
          std::fill(luts[1].begin(),luts[1].end(),stops[0].color[1]);
          std::fill(luts[2].begin(),luts[2].end(),stops[0].color[1]);
          return;
        }
        std::sort(this->stops.begin(),this->stops.end(),cmp_stop);
        ICLASSERT_THROW(stops.front().relPos >= 0, ICLException("PseudoColorConverter: stop with relative position < 0.0 found"));
        ICLASSERT_THROW(stops.back().relPos <= 1, ICLException("PseudoColorConverter: stop with relative position > 1.0 found"));
        if(stops[0].relPos != 0) stops.insert(stops.begin(),Stop(0,Color(0,0,0)));
        if(stops.back().relPos != 1) stops.push_back(Stop(1,Color(255,255,255)));
           
        for(unsigned int i=0;i<3;++i){
          luts[i].resize(maxVal+1);
          for(unsigned int s=0;s<stops.size()-1;++s){
            fill_lin(luts[i].data(),(maxVal+1)*stops[s].relPos,(maxVal+1)*stops[s+1].relPos,stops[s].color[i],stops[s+1].color[i]);
          }
        }
      }
    };
  
      /// creates instance with default color table
    PseudoColorConverter::PseudoColorConverter(int maxValue):m_data(new Data){
      m_data->def(maxValue);
    }
  
      /// creates instance with custom mode
    PseudoColorConverter::PseudoColorConverter(const std::vector<Stop> &stops, int maxValue) throw (ICLException):m_data(new Data){
      m_data->custom(stops,maxValue);
    }
      
      /// sets the mode
    void PseudoColorConverter::setColorTable(ColorTable t, const std::vector<Stop> &stops, int maxValue) throw (ICLException){
      if(t == Custom){
        m_data->custom(stops,maxValue);
      }else{
        m_data->def(maxValue);
      }
    }
      
      /// create a speudo color image from given source image
    void PseudoColorConverter::apply(const ImgBase *src, ImgBase **dst) throw (ICLException){
      ICLASSERT_THROW(src,ICLException(str(__FUNCTION__)+": src image was NULL"));
      ICLASSERT_THROW(dst,ICLException(str(__FUNCTION__)+": destination image was NULL"));
      ICLASSERT_THROW(src->getChannels() == 1, ICLException(str(__FUNCTION__)+": source image has more than one channel"));
      ensureCompatible(dst,depth8u,src->getSize(),formatRGB);
      if(src->getDepth() == depth8u){
        apply(*src->asImg<icl8u>(),*(*dst)->asImg<icl8u>());        
      }else{
        switch(src->getDepth()){
          case depth8u: m_data->lut(*src->as8u(), *(*dst)->as8u()); break;
          case depth16s: m_data->lut(*src->as16s(), *(*dst)->as8u()); break;
          case depth32s: m_data->lut(*src->as32s(), *(*dst)->as8u()); break;
          case depth32f: m_data->lut(*src->as32f(), *(*dst)->as8u()); break;
          case depth64f: m_data->lut(*src->as64f(), *(*dst)->as8u()); break;
          default:
            ICL_INVALID_DEPTH;
        }
      }
    }
      
    /// create a speudo color image from given source image
    void PseudoColorConverter::apply(const Img8u &src, Img8u &dst){
      dst.setSize(src.getSize());
      dst.setFormat(formatRGB);
      dst.setROI(src.getROI());
      dst.setTime(src.getTime());
      Img8u ci;
      for(int i=0;i<3;++i){
        dst.selectChannel(i,&ci);
        src.lut(m_data->luts[i].data(),&ci,8);
      }
    }
  
      /// create a speudo color image from given source image (using an internal buffer)
    const Img8u &PseudoColorConverter::apply(const Img8u &src){
      apply(src,m_data->outputBuffer);
      return m_data->outputBuffer;
    }
  
  
    void PseudoColorConverter::save(const std::string &filename){
      ConfigFile f;
      f.setPrefix("config.pseudo-color-converter.");
      f["maxVal"] = m_data->maxVal;
      f["mode"] = std::string(m_data->mode == Default ? "default" : "custom");
      if(m_data->mode == Custom){
        f["stop-count"] = (int) m_data->stops.size();
        for(unsigned int i=0;i<m_data->stops.size();++i){
          f["stop-"+str(i)+".color"] = str(m_data->stops[i].color[0]) +  " " 
                                     + str(m_data->stops[i].color[1]) +  " " 
                                     +   str(m_data->stops[i].color[2]); 
          f["stop-"+str(i)+".relative-position"] = m_data->stops[i].relPos;
        }
      }
      f.save(filename);
    }
    
    void PseudoColorConverter::load(const std::string &filename){
      ConfigFile f(filename);
      f.setPrefix("config.pseudo-color-converter.");
      if(f["mode"].as<std::string>() == "default"){
        m_data->def(f["maxVal"]);
      }else{
        int n = f["stop-count"];
        std::vector<Stop> stops(n);
        for(int i=0;i<n;++i){
          stops[i] = Stop(f["stop-"+str(i)+".relative-position"],parse<Color>(f["stop-"+str(i)+".color"]));
        }
        m_data->custom(stops,f["maxVal"]);
      }
    }
  
  } // namespace core
}
