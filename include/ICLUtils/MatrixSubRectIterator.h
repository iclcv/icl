/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLUtils/MatrixSubRectIterator.h               **
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

#ifndef ICL_MATRIX_SUB_RECT_ITERATOR_H
#define ICL_MATRIX_SUB_RECT_ITERATOR_H

#include <iterator>

namespace icl{
  /// Iterator class used to iterate through a sub rect of 2D data
  /** 
      The MatrixSubRectIterator is a utility to iterate line by line through
      all elements of a sub-rectangle of a row-major-order aligned data block. 
     The following ASCII image 
      shows an images ROI.
  <pre>
    1st element
      |
  ....|.................... 
  ....+->Xoooooooo......... ---
  .......ooooooooo.........  |
  .......ooooooooo......... Rect-Height
  .......ooooooooo.........  |
  .......ooooooooo......... ---
  ......................... 
         |-RectW-|
  |---------MatrixW-------|
  
  </pre>
  

      \section CONST const-ness
      Please note that the const-ness of an MatrixSubRectIterator instance does
      not say anything about the sturcture itselft. Hence also const 
      MatrixSubRectIterators can be 'moved' using ++-operators or incRow()
      method.\n
      Instead, const-ness relates to the underlying data block
      is referenced by the iterator instance.

  */
  template <typename Type>
  class MatrixSubRectIterator : public std::iterator<std::forward_iterator_tag,Type>{
    protected:
    inline void init () {
       m_lineStep = m_matrixWidth - m_subRectWidth + 1;
       m_dataEnd = m_dataCurr;
       if (m_subRectWidth > 0){
          m_dataEnd += m_subRectWidth + (m_subRectHeight-1) * m_matrixWidth;
       }
       m_currLineEnd = m_dataCurr + m_subRectWidth - 1;
    }

    public:
    
    static inline const MatrixSubRectIterator<Type> create_end_iterator(const Type *dataOrigin,  int matrixWidth, int subRectX, 
                                                                        int subRectY, int subRectWidth, int subRectHeight){      
      MatrixSubRectIterator<Type> i(const_cast<Type*>(dataOrigin),matrixWidth,subRectX, subRectY, subRectWidth,subRectHeight);
      i.m_dataCurr = i.m_dataEnd - subRectWidth + matrixWidth;
      i.m_currLineEnd = i.m_dataCurr + subRectWidth;
      return i;
    }

    /** Creates an MatrixSubRectIterator object */
    /// Default Constructor
    inline MatrixSubRectIterator():
       m_matrixWidth(0),m_subRectWidth(0),m_subRectHeight(0),
       m_dataOrigin(0), m_dataCurr(0) {init();}
    
     /** 2nd Constructor creates an MatrixSubRectIterator object with Type "Type"
         @param ptData pointer to the corresponding channel data
         @param iImageWidth width of the corresponding image
         @param roROI ROI rect for the iterator
     */
    inline MatrixSubRectIterator(Type *ptData,int matrixWidth,int subRectX, int subRectY, int subRectWidth, int subRectHeight):
       m_matrixWidth(matrixWidth),m_subRectWidth(subRectWidth),m_subRectHeight(subRectHeight),
       m_dataOrigin(ptData),m_dataCurr(ptData+subRectX+subRectY*matrixWidth) {init();}
    
    inline MatrixSubRectIterator &operator=(const MatrixSubRectIterator &other){
      m_matrixWidth = other.m_matrixWidth;
      m_subRectWidth = other.m_subRectWidth;
      m_subRectHeight = other.m_subRectHeight;
      m_lineStep = other.m_lineStep;
      m_dataOrigin = other.m_dataOrigin;
      m_dataCurr = other.m_dataCurr;
      m_dataEnd = other.m_dataEnd;
      m_currLineEnd = other.m_currLineEnd;
      return *this;
    }

    inline const MatrixSubRectIterator& operator=(const MatrixSubRectIterator &other) const{
      return (*const_cast<MatrixSubRectIterator*>(this)) = other;
    }

    /// retuns a reference of the current pixel value (const)
    /** changes on *p (p is of type MatrixSubRectIterator) will effect
        the image data       
    */
    inline const Type &operator*() const { return *m_dataCurr;  }

    /// retuns a reference of the current pixel value
    /** changes on *p (p is of type MatrixSubRectIterator) will effect
        the image data       
    */
    inline Type &operator*(){  return *m_dataCurr;  }
    
    /// moves to the next iterator position (Prefix ++it)
    /** The image ROI will be scanned line by line
        beginning on the bottom left iterator.
       <pre>

           +-- begin here (index 0)
           |  
    .......|.................
    .......V.................
    .......012+-->+8<---------- first line wrap after 
    .......9++++++++.........   this pixel (index 8)
    .......+++++++++.........
    .......+++++++++.........
    .......++++++++X<---------- last valid pixel
    ....+->I.................
        |  
    'I' is the first invalid iterator
    (p.inRegion() will become false)
  

       </pre>
       
       In most cases The ++ operator will just increase the
       current x position and update the reference to the
       current pixel data. If the end of a line is reached, then
       the position is set to the beginning of the next line.
    */
    inline MatrixSubRectIterator& operator++(){
      if ( ICL_UNLIKELY(m_dataCurr == m_currLineEnd) ){
        m_dataCurr += m_lineStep;
        m_currLineEnd += m_matrixWidth;
      }else{
        m_dataCurr++;
      }
      return *this;
    }

    /// const version of pre increment operator
    inline const MatrixSubRectIterator& operator++() const{
      return ++(*const_cast<MatrixSubRectIterator*>(this));
    }

    /** postfix operator++ (used -O3 to avoid
        loss of performace when using the "it++"-operator
        In most cases the "++it"-operator will ensure
        best performace.
    **/
    inline MatrixSubRectIterator operator++(int){
      MatrixSubRectIterator current (*this);
      ++(*this); // call prefix operator
      return current; // return previous
    }

    /// const version of post increment operator
    inline const MatrixSubRectIterator operator++(int) const{
      return (*const_cast<MatrixSubRectIterator*>(this))++;
    }
    

    /// to check if iterator is still inside the ROI
    /** This function was replaced by STL-like begin(), end() logic
        Although in some cases it might be quite useful, so
        we renamed it rather than deleting it
        @see operator++ */
    inline bool inSubRect() const{
      return m_dataCurr < m_dataEnd;          
    }



    /// compare two iterators
    inline bool operator!=(const MatrixSubRectIterator<Type> &it) const{
      return m_dataCurr != it.m_dataCurr;
    }
    /// compare two iterators
    inline bool operator==(const MatrixSubRectIterator<Type> &it) const{
      return m_dataCurr == it.m_dataCurr;
    }
    /// compare two iterators
    inline bool operator<(const MatrixSubRectIterator<Type> &it) const{
      return m_dataCurr < it.m_dataCurr;
    }
    /// compare two iterators
    inline bool operator>(const MatrixSubRectIterator<Type> &it) const{
      return m_dataCurr > it.m_dataCurr;
    }
    /// compare two iterators
    inline bool operator<=(const MatrixSubRectIterator<Type> &it) const{
      return m_dataCurr <= it.m_dataCurr;
    }
    /// compare two iterators
    inline bool operator>=(const MatrixSubRectIterator<Type> &it) const{
      return m_dataCurr >= it.m_dataCurr;
    }


    /// returns the length of each row processed by this iterator
    /** @return row length 
     */
    inline int getSubRectWidth() const{
      return m_subRectWidth;
    }
    
    inline int getSubRectHeight() const{
      return m_subRectHeight;
    }
    
    /// move the pixel vertically forward
    /** current x value is hold, the current y-value is
        incremented by iLines
        @param iLines amount of lines to jump over
    */
    inline void incRow(int numLines=1) const {
      m_dataCurr += numLines * m_matrixWidth;
      m_currLineEnd += numLines * m_matrixWidth;
    }

    /// returns the current x position of the iterator (wrt matrix origin);
    /** @returna current x position*/
    inline int x(){
      return (m_dataCurr-m_dataOrigin) % m_matrixWidth;
    }

    /// returns the current y position of the iterator (wrt matrix origin)
    /** @returna current y position*/
    inline int y(){
      return (m_dataCurr-m_dataOrigin) / m_matrixWidth;
    }       
    
    protected:
    /// corresponding matrix width
    int m_matrixWidth;
    
    /// sub rect size of the iterator
    int m_subRectWidth, m_subRectHeight;

    /// result of m_matrixWidth - m_subRectWidth
    int m_lineStep;

    /// pointer to the upper matrix data origin  (upper left element)
    Type *m_dataOrigin;

    /// pointer to the current data element
    mutable Type *m_dataCurr;

    /// pointer to the first element behind the subrect
    Type *m_dataEnd;

    /// pointer to the first invalid element of the current line
    mutable Type *m_currLineEnd;
    
  };
}
#endif
