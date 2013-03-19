/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/TextTable.h                      **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Size.h>

namespace icl{
  namespace utils{
  
    /// Utility class for pretty console output
    /** The text table implements a simple tabular structure
        (rows x colums cells that have string content).
        The TextTable structure can always be serialized as
        a pretty printed. The table's size is automatically
        expanded if new data is set. Optionally, the table
        can be created with a given row- and column count.
        The table data can be set using the (x,y)-function-
        call operator.
        \section EX Example

        \code
        #include <ICLUtils/TextTable.h>
        #inclue <iostream>
        
        using namespace icl::utils;
        
        int main(int n, char **ppc){
          TextTable t;
        
          t(0,0) = "name";
          t(1,0) = "forename";
          t(2,0) = "age";
          t(3,0) = "address";
        
          t[1] = tok("elbrechter,christof,34,Some Street in Oerlinghausen (close to Bielefeld)",",");
          t[2] = tok("gotting,michael,??,Somewhere else",",");
        
          std::cout << t << std::endl;
        } 
        
        
        //  output
        //  +------------+----------+-----+----------------------+
        //  |    name    | forename | age |       address        | 
        //  +------------+----------+-----+----------------------+
        //  | elbrechter | christof | 34  | Some Street in Oerli | 
        //  |            |          |     | nghausen (close to B | 
        //  |            |          |     |      ielefeld)       | 
        //  +------------+----------+-----+----------------------+
        //  |  gotting   | michael  | ??  |    Somewhere else    | 
        //  +------------+----------+-----+----------------------+

      \endcode
    */
    class TextTable{
      std::vector<std::string> m_texts; //!< internal text data
      Size m_size;  //!< current size
      int m_maxCellWidth; //!< current maximum cell width for serialization

      public:
    
      /// creates a new table with optionally given dimensions
      /** The table size is automatically expanded whenever a non-
          existing row- or column-index is passed to the (x,y)-
          index operator to (read or set data) */
      inline TextTable(int width=0, int height=0, int maxCellWidth=20):
      m_texts(width*height),m_size(width,height),m_maxCellWidth(maxCellWidth){}

      /// returns a reference to the entry at given cell coordinates
      /** Please note, that there is no const-version since this method
          does expand the table dimensions if the selected cell-coordinates
          are larger than the cell size */
      inline std::string &operator()(int xCell, int yCell){
        ensureSize(xCell+1, yCell+1);
        return m_texts[xCell + m_size.width*yCell];
      }
    
      /// allocates at least as much memory for given amount of rows and columns
      void ensureSize(int width, int height);
    
    
      /// returns the maximum cell width
      /** This option is just for serialization. Cells will never become wider than
          this size. If a cell's content is larger than the maxCellWidth
          the row-height is increased automatically
          **/
      inline int getMaxCellWidth() const { return m_maxCellWidth; }
    
      /// returns the current maxCellWidth value
      inline void setMaxCellWidth(int maxCellWidth) { m_maxCellWidth = maxCellWidth; }
    
      /// returns the current table size
      inline const Size &getSize() const { return m_size; }
    
      /// returns the internal data vector 
      inline const std::vector<std::string> &getData() const { return m_texts; }

      /// Utility class that is used, to assign a table row at once
      class RowAssigner{
        TextTable &t; //!< parent TextTable 
        int row;      //!< parent
        inline RowAssigner(TextTable &t, int row):t(t),row(row){}

        /// grant instantiation access to the parent TextTable class
        friend class TextTable;

        public:
        /// assigns a standard vector of strings (each element is put into a single column)
        inline void operator=(const std::vector<std::string> &rowValues){
          for(unsigned int i=0;i<rowValues.size();++i){
            this->t(i,this->row) = rowValues[i];
          }
        }
        /// assigns the row of another TextTable
        inline void operator=(RowAssigner otherRow){
          for(int i=0;i<otherRow.t.getSize().width;++i){
            this->t(i,row) = otherRow.t(i,otherRow.row);
          }
        }
      };
    
      /// gives access to the table row (this can be assigned directly if needed)
      inline RowAssigner operator[](int row){
        return RowAssigner(*this,row);
      }
    
      /// serializes the table to a std::string
      std::string toString() const;
    
      /// clears all current existing table cells
      /** Please note: the table size is not changed here. */
      void clear();
    };
  
    /// overloaded ostream-operator that uses the TextTable's toString method for serialization
    inline std::ostream &operator<<(std::ostream &stream, const TextTable &t){
      return stream << t.toString();
    }
  } // namespace utils
}

