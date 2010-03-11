/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQt module of ICL                           **
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

#include <ICLQt/ChromaClassifierIO.h>

namespace icl{
  
  namespace{
    float &auxget(Parable &p, int idx){
      switch(idx){
        case 0: return p.a;
        case 1: return p.b;
        default: return p.c;
      }
    }
    const float &auxget(const Parable &p, int idx){
      return auxget(const_cast<Parable&>(p),idx);
    }
  }
  
  void ChromaClassifierIO::save(const ChromaClassifier &cc,const  std::string &filename, const std::string &name){
    ConfigFile f;
    const char cs[] = "abc";
    for(int p=0;p<2;++p){
      for(int c=0;c<3;++c){
        f.set(std::string("config.")+name+".parable-"+str(p)+"."+cs[c],auxget(cc.parables[p],c));
      }
    }
    f.save(filename);
  }
  void ChromaClassifierIO::save(const ChromaAndRGBClassifier &carc,const std::string &filename){
    save(carc.c,filename,std::string("chroma-and-rgb-classifier"));
    ConfigFile f;
    f.set("config.chroma-and-rgb-classifier.ref-color.red",carc.ref[0]);
    f.set("config.chroma-and-rgb-classifier.ref-color.green",carc.ref[1]);
    f.set("config.chroma-and-rgb-classifier.ref-color.blue",carc.ref[2]);
    f.set("config.chroma-and-rgb-classifier.threshold.red",carc.thresh[0]);
    f.set("config.chroma-and-rgb-classifier.threshold.green",carc.thresh[1]);
    f.set("config.chroma-and-rgb-classifier.threshold.blue",carc.thresh[2]);
    f.save(filename);
  }
  ChromaClassifier ChromaClassifierIO::load(const std::string &filename, const std::string &name){
    ConfigFile f(filename);
    ChromaClassifier cc;
    const char cs[] = "abc";
    for(int p=0;p<2;++p){
      for(int c=0;c<3;++c){
        auxget(cc.parables[p],c) = f.get<float>(std::string("config.")+name+".parable-"+str(p)+"."+cs[c]);
      }
    }
    return cc;
  }
  
  ChromaAndRGBClassifier ChromaClassifierIO::loadRGB(const std::string &filename){
    ChromaClassifier cc = load(filename,"chroma-and-rgb-classifier");
    ConfigFile f(filename);
    ChromaAndRGBClassifier carc;
    carc.c = cc;
    carc.ref[0] = f.get<icl8u>("config.chroma-and-rgb-classifier.ref-color.red");
    carc.ref[1] = f.get<icl8u>("config.chroma-and-rgb-classifier.ref-color.green");
    carc.ref[2] = f.get<icl8u>("config.chroma-and-rgb-classifier.ref-color.blue");
    carc.thresh[0] = f.get<icl8u>("config.chroma-and-rgb-classifier.threshold.red");
    carc.thresh[1] = f.get<icl8u>("config.chroma-and-rgb-classifier.threshold.green");
    carc.thresh[2] = f.get<icl8u>("config.chroma-and-rgb-classifier.threshold.blue");
    return carc;
  }
}


