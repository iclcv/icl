#include "ICLImg.h"
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

  Interleaved(ImgBase* poSrc, bool deepCopyData);
  ~Interleaved();

  bool m_bHaveData;
  ImgBase* m_poData;
  std::vector<T*> m_vecDataPtr;

  const std::vector<T*>& getDataPtr();
  unsigned int getDim() {return m_poData->getChannels();}

};

} // namespace icl

#endif
