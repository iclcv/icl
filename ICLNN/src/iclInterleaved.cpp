#include "iclInterleaved.h"
#include "iclImg.h"
/*
  Interleaved.cpp

  Written by: Michael Götting (2007)
              University of Bielefeld
              AG Neuroinformatik
              {mgoettin,celbrech}@techfak.uni-bielefeld.de
*/


using namespace icl;
using namespace std;

namespace icl {

  template <class T>
  Interleaved<T>::Interleaved(const ImgBase *poSrc) :
    m_poData(poSrc) {
    // create Data Vector
    for (int i=0;i<m_poData->getChannels();i++) {
      m_vecDataPtr.push_back((T*) m_poData->asImg<T>()->getDataPtr(i));
    }
  }
  
  template <class T>
  void Interleaved<T>::setData(const ImgBase *poSrc)
  {
    // Variable initialisation
    m_poData = poSrc;
    m_vecDataPtr.clear();
    
    // create Data Vector
    for (int i=0;i<m_poData->getChannels();i++) {
      m_vecDataPtr.push_back((T*) m_poData->asImg<T>()->getDataPtr(i));
    }
  }
  
  template <class T>
  inline const vector<T*>& Interleaved<T>::getDataPtr() { 
    return m_vecDataPtr;
  }
  
  template class Interleaved<icl8u>;
  template class Interleaved<icl16s>;
  template class Interleaved<icl32f>;
  
}
