/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/IOFunctions.cpp                              **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Götting, Robert Haschke   **
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

#include <ICLIO/IOFunctions.h>
#include <ICLCore/Img.h>
#include <stdio.h>
#include <map>
#include <cstring>


using namespace std;


namespace icl {
  struct OffsPtr {
    OffsPtr(const Point &offs=Point::null, icl8u* img=0, int width=0):
      offs(offs), img(img), width(width){}
    Point offs;
    icl8u *img;
    int width;
  };
  
  template<class T>
  void labelImage(Img<T> *image,const char*txt, const std::map<char,OffsPtr> &m){
    static const Point OFS(5,5);

    Rect roi = Rect(OFS.x-1,OFS.y-1,strlen(txt)*(7+2),7+2);
    if(Rect(Point::null,image->getSize()).contains(roi)){
      Img<T> *boxImage = image->shallowCopy(roi);
      boxImage->clear();
      delete boxImage;
    }
    
    int letterIdx = 0;
    for(const char *p = txt; *p; p++){

      const std::map<char,OffsPtr>::const_iterator it = m.find(*p);
      if(it == m.end()) continue;
      const OffsPtr &op = it->second;
      //      printf("tying to draw \"%c\"  pointer offs = (%d,%d)\n",*p,(op.offs.x),op.offs.y);
      
      
      for(int c=0;c < image->getChannels();c++){
        int xStartLetter = op.offs.x;
        int xEndLetter = xStartLetter+7;
        int xStartImage = OFS.x + letterIdx*(7+2);
        for(int xL=xStartLetter,xI=xStartImage; xL < xEndLetter ; xI++,xL++){
          if(xI < image->getWidth() ){
            int yStartLetter = 0;
            int yEndLetter = 7;
            int yStartImage = OFS.y;
            for(int yL=yStartLetter, yI= yStartImage; yL < yEndLetter; yI++,yL++){
              if(yI < image->getHeight()){
                if(char(op.img[xL+yL*op.width]) == '#'){
                  (*image)(xI,yI,c) = 255;
                }else{
                  (*image)(xI,yI,c) = 0;
                }
              }
            }
          }
        }
      }
      letterIdx++;
    } 
  }
  
  void labelImage(ImgBase *image,const std::string &label){
    ICLASSERT_RETURN( image );
    ICLASSERT_RETURN( label.length() );

    static char ABC_AM[] = 
    " ##### ######  ############ ############## ##### #     ################     ##      #     #"
    "#     ##     ##      #     ##      #      #     ##     #   #         ##    # #      ##   ##"
    "#     ##     ##      #     ##      #      #      #     #   #         ##   #  #      # # # #"
    "#     ####### #      #     ################  ###########   #         #####   #      #  #  #"
    "########     ##      #     ##      #      #     ##     #   #         ##   #  #      #     #"
    "#     ##     ##      #     ##      #      #     ##     #   #         ##    # #      #     #"
    "#     #######  ############ ########       ##### #     ############## #     #########     #";

    static char ABC_NZ[] = 
    "#     # ##### ######  ##### ######  ##### ########     ##     ##     ##     ##     ########"
    "##    ##     ##     ##     ##     ##     #   #   #     ##     ##     # #   #  #   #      # "
    "# #   ##     ##     ##     ##     ##         #   #     # #   # #     #  # #    # #      #  "
    "#  #  ##     ####### #     #######  #####    #   #     # #   # #  #  #   #      #      #   "
    "#   # ##     ##      # ### ##   #        #   #   #     #  # #  # # # #  # #     #     #    "
    "#    ###     ##      #    ###    # #     #   #   #     #  # #  ##   ## #   #    #    #     " 
    "#     # ##### #       ##### #     # #####    #    #####    #   #     ##     #   #   #######";
                                                                                                
    static char ABC_09[] = 
    " #####      ## #####  ##### #   #  ####### ##### ####### #####  ##### "
    "#     #   ## ##     ##     ##   #  #      #     #      ##     ##     #"
    "#     # ##   #      #      ##   #  #      #           # #     ##     #"
    "#     ##     #  ####  ##### ############# ######  ###### #####  ######"
    "#     #      ###           #    #        ##     #   #   #     #      #"
    "#     #      ##      #     #    #  #     ##     #  #    #     ##     #"
    " #####       ######## #####     #   #####  #####  #      #####  ##### ";

    static char ABC_EX[] = 
    "          #    #   #  #   #  #####  #    # ###      #       #    #     # # #                                   #"
    "          #    #   # ########  #  ## #  # #   #     #      #      #     ###     #                             # "
    "          #           #   #    #  # #  #   ###             #      #    #####    #                            #  "
    "          #           #   #  #####    #    # #            #        #    ###   #####         #####           #   "
    "          #           #   # #  #     #  # #   ##           #      #    # # #    #       #                  #    "
    "                     ########  #  # #  # ##   ##           #      #             #      #             ##   #     "
    "          #           #   #  ##### #    #  #### #           #    #                    #              ##  #      ";
    static std::map<char,OffsPtr> m;
    static bool first = true;

    if(first){
      first = false;
      for(int c='a', xoffs=0; c<='m'; c++, xoffs+=7){
        m[c] = OffsPtr(Point(xoffs,0),reinterpret_cast<icl8u*>(const_cast<char*>(ABC_AM)),13*7);
        m[c-32] = OffsPtr(Point(xoffs,0),reinterpret_cast<icl8u*>(const_cast<char*>(ABC_AM)),13*7);
      }
      for(int c='n', xoffs=0; c<='z'; c++, xoffs+=7){
        m[c] = OffsPtr(Point(xoffs,0),reinterpret_cast<icl8u*>(const_cast<char*>(ABC_NZ)),13*7);
        m[c-32] = OffsPtr(Point(xoffs,0),reinterpret_cast<icl8u*>(const_cast<char*>(ABC_NZ)),13*7);
      }
      for(int c='0', xoffs=0; c<='9'; c++, xoffs+=7){
        m[c] = OffsPtr(Point(xoffs,0),reinterpret_cast<icl8u*>(const_cast<char*>(ABC_09)),10*7);
      }
      for(int c=' ', xoffs=0; c<='/'; c++, xoffs+=7){
        m[c] = OffsPtr(Point(xoffs,0),reinterpret_cast<icl8u*>(const_cast<char*>(ABC_EX)),16*7);
      }
    }
    
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: labelImage<icl##D>(image->asImg<icl##D>(),label.c_str(),m); break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
  }

} //namespace
