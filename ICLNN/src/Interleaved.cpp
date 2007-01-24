/*
  Interleaved.cpp

  Written by: Michael Götting (2007)
              University of Bielefeld
              AG Neuroinformatik
              {mgoettin,celbrech}@techfak.uni-bielefeld.de
*/

#include "Interleaved.h"
#include "Img.h"

using namespace icl;
using namespace std;

namespace icl {

  template <class T>
  Interleaved<T>::Interleaved(ImgBase *poSrc, bool deepCopyData) :
    m_bHaveData(deepCopyData) {
    
    // deep copy data
    if (deepCopyData) {
      m_poData = poSrc->deepCopy();
    } else {
      m_poData = poSrc;
    }

    // create Ptr Vector
    for (int i=0;i<m_poData->getChannels();i++) {
      m_vecDataPtr.push_back((T*) m_poData->asImg<T>()->getDataPtr(i));
    }
  }
  
  template <class T>
  Interleaved<T>::~Interleaved()
  {
    if (m_bHaveData) { delete m_poData; }
    
  }

  template <class T>
  inline const vector<T*>& Interleaved<T>::getDataPtr() { 
    return m_vecDataPtr;
  }

  template class Interleaved<icl8u>;
  template class Interleaved<icl16s>;
  template class Interleaved<icl32f>;
  
}
