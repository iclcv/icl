/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLUtils/Lockable.h                            **
** Module : ICLUtils                                               **
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
*********************************************************************/

#ifndef ICL_LOCKABLE_H
#define ICL_LOCKABLE_H

#include <ICLUtils/Mutex.h>
#include <ICLUtils/UncopiedInstance.h>

namespace icl{

  /// Interface for objects, that can be locked using an internal mutex
  class Lockable{

    /// mutable and uncopied mutex instance (locking preserves constness)
    mutable UncopiedInstance<Mutex> m_mutex;
    public:
    
    /// lock object
    void lock() const { m_mutex.lock(); }
    
    /// unlock object
    void unlock() const { m_mutex.unlock(); }
    
    /// returns mutex of this object
    Mutex &getMutex() const { return m_mutex; }
  };
  
}

#endif
