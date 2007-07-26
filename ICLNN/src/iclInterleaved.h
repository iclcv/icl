#ifndef Interleaved_H
#define Interleaved_H

#include "iclImg.h"
#include <vector>
 /*
  Interleaved.h

  Written by: Michael Götting and Christof Elbrechter (2007)
              University of Bielefeld
              AG Neuroinformatik
              {mgoettin,celbrech}@techfak.uni-bielefeld.de
*/

namespace icl {

template <typename T>
class Interleaved {
 public:

  Interleaved();
  Interleaved(const ImgBase* poSrc);
  ~Interleaved() {}

  const ImgBase* m_poData;
  std::vector<const T*> m_vecDataPtr;

  void setData(const ImgBase *poSrc);
  
  std::vector<const T*>& getDataPtr();
  unsigned int getDim() {return m_poData->getChannels();}
  const ImgBase* getSrcImg() { return m_poData; }

};

} // namespace icl

#endif
