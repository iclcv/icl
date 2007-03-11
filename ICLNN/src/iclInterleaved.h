#include "iclImg.h"
#include <vector>
 /*
  Interleaved.h

  Written by: Michael Götting and Christof Elbrechter (2007)
              University of Bielefeld
              AG Neuroinformatik
              {mgoettin,celbrech}@techfak.uni-bielefeld.de
*/

#ifndef Interleaved_H
#define Interleaved_H

namespace icl {

template <typename T>
class Interleaved {
 public:

  Interleaved(const ImgBase* poSrc);
  Interleaved() {}
  ~Interleaved() {}

  const ImgBase* m_poData;
  std::vector<T*> m_vecDataPtr;

  void setData(const ImgBase *poSrc);
  
  const std::vector<T*>& getDataPtr();
  unsigned int getDim() {return m_poData->getChannels();}
  const ImgBase* getSrcImg() { return m_poData; }

};

} // namespace icl

#endif
