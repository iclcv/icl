/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/FiducialDetectorPluginART.cpp           **
** Module : ICLMarkers                                             **
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

#include <ICLIO/FileGrabber.h>
#include <ICLIO/FileList.h>
#include <ICLIO/File.h>
#include <ICLCV/Quick.h>

#include <ICLMarkers/FiducialDetectorPluginART.h>

namespace icl{
  namespace markers{
    static Img8u rotate(const Img8u &in){
      const Size s = in.getSize();
      Img8u out(Size(s.height,s.width),1);
      Channel8u o = out[0];
      const Channel8u i = in[0];
      for(int y=0;y<s.height;++y){
        for(int x=0;x<s.width;++x){
          //o(y,x) = i(x,s.height-1-y);
          o(y,x) = i(s.width-1-x,y);
        }
      }
      return out;
    }
    
    static void inplace_thresh(icl8u *p, int n, int thresh){
      for(int i=0;i<n;++i){
        p[i] = 255 * (p[i]>thresh);
      }
    }
  
    
    namespace{
      struct NamedImage : public Img8u{
        std::string name;
        Size size;
        int id;
        NamedImage(const Img8u &image, const std::string &name, const Size &size, int id):
          Img8u(image),name(name),size(size),id(id){}
      };
      
      struct Matching{
        virtual void prepare(const std::vector<SmartPtr<NamedImage> > &loaded, const Size &matchDim)=0;
        virtual NamedImage *match(const Img8u &image, int *rot, float *err) = 0;
        virtual ~Matching(){}
      };
      
      struct Img4{
        Img8u im[4];
        NamedImage *ni;
        Img8u &operator[](int i) { return im[i]; }
      };
  
      struct MatchingBinaryHamming : public Matching{
        std::vector<Img4> is;
        Img8u dimBuf;
        virtual void prepare(const std::vector<SmartPtr<NamedImage> > &loaded, const Size &dim){
          dimBuf.setSize(dim);
          dimBuf.setChannels(1);
          for(unsigned int i=0;i<loaded.size();++i){
            Img8u image(dim,1);
            loaded[i]->scaledCopy(&image);
            inplace_thresh(image.begin(0), image.getDim(), 128);
            is.push_back(Img4());
            Img4 &i4 = is.back();
            i4[0] = image;
            i4[1] = rotate(i4[0]);
            i4[2] = rotate(i4[1]);
            i4[3] = rotate(i4[2]);
            i4.ni = const_cast<NamedImage*>(loaded[i].get());
          }
        }
        
        int get_error(const icl8u *a, const icl8u *b, int n){
          int err = 0;
          for(int i=0;i<n;++i){
            err += ( (a[i]>127) ^ (b[i]&1) );
          }
          return err;
        }
        
        virtual NamedImage *match(const Img8u &image, int *rot, float *err){
          if(!is.size()){
            ERROR_LOG("FiducialDetectorPluginART: no patterns loaded!");
            return 0;
          }
          int minIdx = -1;
          int minErr = 1<<22;
          int minRot = 0;
          const icl8u *data = 0;
          if(image.getSize() == dimBuf.getSize()){
            data = image.begin(0);
          }else{
            image.scaledCopy(&dimBuf);
            data = dimBuf.begin(0);
          }
          const int dim = dimBuf.getDim();
          for(unsigned int i=0;i<is.size();++i){
            const int es[4] = {
              get_error(data,is[i][0].begin(0),dim),
              get_error(data,is[i][1].begin(0),dim),
              get_error(data,is[i][2].begin(0),dim),
              get_error(data,is[i][3].begin(0),dim)
            };
            int m = (int)(std::min_element(es,es+4)-es);
            if(es[m] < minErr){
              minIdx = i;
              minErr = es[m];
              minRot = m;
            }
          }
          *err = float(minErr)/dim;
          *rot = minRot;
          return is[minIdx].ni;
        }
      };
  
  
      struct MatchingGray : public Matching{
        enum DistMode{
          sqrDist,
          normalizedCrossCorr
        } mode;
        
        MatchingGray(DistMode mode):mode(mode){}
        
        std::vector<Img4> is;
        Img8u dimBuf;
        virtual void prepare(const std::vector<SmartPtr<NamedImage> > &loaded, const Size &dim){
          dimBuf.setSize(dim);
          dimBuf.setChannels(1);
          for(unsigned int i=0;i<loaded.size();++i){
            Img8u image(dim,1);
            loaded[i]->scaledCopy(&image);
            is.push_back(Img4());
            Img4 &i4 = is.back();
            i4[0] = image;
            i4[1] = rotate(i4[0]);
            i4[2] = rotate(i4[1]);
            i4[3] = rotate(i4[2]);
            i4.ni = const_cast<NamedImage*>(loaded[i].get());
          }
        }
        
        float get_error(const Channel8u &a, const Channel8u &b){
          float err = 0;
          const int dim = a.getDim();          
          if(dim<2) return 0;
          
          if(mode == sqrDist){
            for(int i=0;i<dim;++i){
              err += sqr(a[i]-b[i]);
            }
            err = ::sqrt(err);
            err /= (255*dim);
          }else{
            /// normalized cross corellation
            float meanA=0, meanB=0;
            for(int i=0;i<dim;++i){
              meanA += a[i];
              meanB += b[i];
            }
            meanA /= dim;
            meanB /= dim;
            float varA=0, varB=0;
            for(int i=0;i<dim;++i){
              int A = a[i]-meanA;
              int B = b[i]-meanB;
              varA += sqr(A);
              varB += sqr(B);
              err += A*B;
            }
            // we still need a better heuristics for this
            err = (::sqrt(varA)*::sqrt(varB)*(dim-1) / err);
            err = ::sqrt(err)/dim;
          }
          return err;
        }
        
        virtual NamedImage *match(const Img8u &image, int *rot, float *err){
          if(!is.size()){
            ERROR_LOG("FiducialDetectorPluginART: no patterns loaded!");
            return 0;
          }
          int minIdx = -1;
          float minErr = 1<<22;
          int minRot = 0;
          const Img8u *input = 0;
          if(image.getSize() == dimBuf.getSize()){
            input = &image;
          }else{
            image.scaledCopy(&dimBuf);
            input = &dimBuf;
          }
          const Channel8u in = (*input)[0];
          for(unsigned int i=0;i<is.size();++i){
            const float es[4] = {
              get_error(in,is[i][0][0]),
              get_error(in,is[i][1][0]),
              get_error(in,is[i][2][0]),
              get_error(in,is[i][3][0])
            };
            int m = (int)(std::min_element(es,es+4)-es);
            if(es[m] < minErr){
              minIdx = i;
              minErr = es[m];
              minRot = m;
            }
          }
          *err = minErr;
          *rot = minRot;
          return is[minIdx].ni;
        }
        
  
      };
      
    }
  
    
    struct FiducialDetectorPluginART::Data{
      std::vector<SmartPtr<NamedImage> > loaded;
      std::string lastMatching;
      Size lastMatchingSize;
      SmartPtr<Matching> matching;
      
      void updateMatching(const std::string &a,const Size &matchingSize){
        lastMatching = a;
        lastMatchingSize = matchingSize;
        if( a == "binary hamming" ){
          matching = new MatchingBinaryHamming;
        }else if( a == "gray sqrdist" ){
          matching = new MatchingGray(MatchingGray::sqrDist);
        }else if ( a== "gray ncc"){
          matching = new MatchingGray(MatchingGray::normalizedCrossCorr);
        }else{
          throw ICLException("currently unsupported matching type: " + a);
        }
  
        matching->prepare(loaded,matchingSize);
      }
    };
    
    
    FiducialDetectorPluginART::FiducialDetectorPluginART():data(new Data){
      addProperty("matching algorithm","menu","binary hamming,gray sqrdist,gray ncc","binary hamming",0,
                  "Algorithm for comparing the rectified marker center with\n"
                  "the internal representation:\n"
                  "binary hamming: binary hamming distance\n"
                  "gray sqrdist: square distance of the gray images\n"
                  "gray ncc: normalized cross correlation coefficient\n");
      addProperty("matching max error","range","[0,1]",0.1,0,
                  "Matching accuracy value:\n"
                  "0: no matches\n"
                  "1: everything matches\n");
      addProperty("matching dim","range","[4,256]:1",32,0,
                  "Marker patch rectification size.");
      addProperty("border ratio","range","[0,1]",0.4,0,
                  "Ratio of marker border pixels and marker dimension.");
    }
    
    FiducialDetectorPluginART::~FiducialDetectorPluginART(){
      delete data;
    }
    
    void FiducialDetectorPluginART::addOrRemoveMarkers(bool add, const Any &which, const ParamList &params){
      FileList l(which);
  
      Size size = params["size"];
  
  
      if(add){
        for(int i=0;i<l.size();++i){
          FileGrabber g(l[i]);
          g.useDesired(formatGray);
          g.useDesired(depth8u);
          data->loaded.push_back(new NamedImage( *g.grab()->asImg<icl8u>(), 
                                                 File(l[i]).getBaseName(), 
                                                 size, data->loaded.size()));
        }
      }else{
        if(which == "*"){
          data->loaded.clear();
        }else{
          for(int i=0;i<l.size();++i){
            std::string id = File(l[i]).getBaseName();
            for(unsigned int j=0;j<data->loaded.size();++j){
              if(data->loaded[j]->name == id){
                data->loaded.erase(data->loaded.begin()+j);
                break;
              }
            }
          }
          // update the id's / indices
          for(unsigned int i=0;i<data->loaded.size();++i){
            data->loaded[i]->id = i;
          }
        }
        
      }
      
      std::string a = getPropertyValue("matching algorithm");
      int matchDim = getPropertyValue("matching dim");
      Size matchSize(matchDim,matchDim);
      data->updateMatching(a,matchSize);
    }
    
    
    
    FiducialImpl *FiducialDetectorPluginART::classifyPatch(const Img8u &image, int *rot, 
                                                           bool returnRejectedQuads, ImageRegion r){
      std::string a = getPropertyValue("matching algorithm");
      int matchDim = getPropertyValue("matching dim");
      Size matchSize(matchDim,matchDim);
      if(data->lastMatching != a || data->lastMatchingSize != matchSize){
        data->updateMatching(a,matchSize);
      }
      const float e = getPropertyValue("matching max error");
  
      float err;
      NamedImage *n = data->matching->match(image, rot, &err) ;
      static Fiducial::FeatureSet supported = Fiducial::AllFeatures;
      static Fiducial::FeatureSet computed = ( Fiducial::Center2D | 
                                               Fiducial::Rotation2D |
                                               Fiducial::Corners2D );
      if(n && err < e){
        FiducialImpl *impl = new FiducialImpl(this,supported,computed,
                                              n->id, -1, n->size);
        impl->imageRegion = r;
        return impl;
      }else if (returnRejectedQuads){
        *rot = 0;
        FiducialImpl *impl = new FiducialImpl(this,supported,computed, 999999, -1, Size(1,1)); // dummy ID
        impl->imageRegion = r;
        return impl;
      }else{
        return 0;
      }
    }
  
    std::string FiducialDetectorPluginART::getName(const FiducialImpl *impl){
      return data->loaded[impl->id]->name;
    }
  
    void FiducialDetectorPluginART::getQuadRectificationParameters(Size &markerSizeWithBorder,
                                                                   Size &markerSizeWithoutBorder){
      float r = getPropertyValue("border ratio");
      int d = getPropertyValue("matching dim");
      markerSizeWithBorder = Size((1+r)*d, (1+r)*d);
      markerSizeWithoutBorder = Size(d,d);
    }  
  
    Img8u FiducialDetectorPluginART::createMarker(const Any &whichOne,const Size &size, const ParamList &params){  
      FileGrabber g(whichOne);
      g.useDesired(formatGray);
      g.useDesired(depth8u);
      const Img8u &l = *g.grab()->asImg<icl8u>();
      float b = params["border ratio"];
      Size s(l.getWidth()*(1+b), l.getHeight()*(1+b));
      Img8u r(s,1);
      r.setROI(Rect(l.getWidth()*b*.5, l.getHeight()*b*.5,l.getWidth(),l.getHeight()));
      l.deepCopyROI(&r);
      r.scale(size);
      return r;
    }
  } // namespace markers
}
