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

#include <ICLQuick/Common.h>
#include <ICLUtils/Rect32f.h>

#define INST_NUM_TYPES                           \
  INST_TYPE(char)                                \
  INST_TYPE(unsigned char)                       \
  INST_TYPE(short)                               \
  INST_TYPE(unsigned short)                      \
  INST_TYPE(int)                                 \
  INST_TYPE(unsigned int)                        \
  INST_TYPE(long)                                \
  INST_TYPE(unsigned long)                       \
  INST_TYPE(float)                               \
  INST_TYPE(double)                              


#define INST_OTHER_TYPES                         \
  INST_TYPE(Rect)                                \
  INST_TYPE(Rect32f)                             \
  INST_TYPE(Size)                                \
  INST_TYPE(Point)                               \
  INST_TYPE(Point32f)                            \
  INST_TYPE(Range32s)                            \
  INST_TYPE(Range32f)                            \
  INST_TYPE(std::string)                         \
  INST_TYPE(Img8u)                               \
  INST_TYPE(Img16s)                              \
  INST_TYPE(Img32s)                              \
  INST_TYPE(Img32f)                              \
  INST_TYPE(Img64f)                              



int main(){
  DataStore d;
  
#define INST_TYPE(T) d.allocValue(#T,*new T());
  INST_NUM_TYPES
  INST_OTHER_TYPES
#undef INST_TYPE

  d.listContents();

#define INST_TYPE(T)                                                    \
  d[#T] = 0;                                                            \
  std::cout << "entry of type " << #T << " value is: " << d[#T].as<T>() << std::endl;
  
  INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T)                                                    \
  d[#T] = icl8u(7);                                                     \
  std::cout << "entry of type " << #T << " value is: " << d[#T].as<T>() << std::endl;
  
  INST_NUM_TYPES
#undef INST_TYPE



  

}



