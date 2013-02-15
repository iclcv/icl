/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/VisualizationDescription.h            **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#pragma once

#include <ICLUtils/Any.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Rect32f.h>

namespace icl{
  namespace utils{
    

    /// Abstract class for visualization tasks
    /** The visualization class provides an interface for tools
        to provide visualization information. It's idea is
        to provide an easy interface -- it's implementation is 
        rather slow, but completely neglegible for most applications;
        
        Right now, this is a very simple interface which still needs
        to be extended
        
        \section PART Supported Parts
        Right now, the following parts are supported. Color and Fill parts remain
        active untill the next color/fill  part is put into the list
        - c draw color (serialized as VisualizationDescription::Color)
        - f fill color (analoguous to c) 
        - r rectangle (serialized as Rect32f)
        - e ellipse (serialized as Rect32f, cicles are also ellipses)
        - l line (serialized as Rect32f)#
        - t text (serialized as VisualizationDescription::Text)
        - y polygon (serialized as binary float vector)
        - p points (serialized as binary float vector)
        - P point size (serialized as float)
        - L line width (serialized as float)
        
        - +xo. for symbols (serialized as Point32f)
          - +: plus symbols
          - x: x-symbols
          - o: little circles
          - .: point
    */
    class VisualizationDescription{
      public:
      
      /// Single part of the the visualization pipeline
      struct Part{
        /// constructor
        inline Part(){}
        
        /// constructor with given parameters
        Part(char type, const Any &content):type(type),content(content){}

        /// type
        char type;
        
        /// strind-serialized content
        Any content;
      };

      /// Utility class for Text
      struct Text{
        Point32f pos; //!< text pos
        std::string text; //!< text content
        /// Empty constructor
        Text(){}
        
        /// Constructor with given pos and content
        Text(const Point32f &pos, const std::string &text):pos(pos),text(text){}
      };
      
      /// Utility Color class
      struct Color{
        /// color union
        union{
          icl8u comp[4];
          icl32s rgba; /// rgba color value
        };
        /// Empty default constructor
        Color(){}
        
        /// Constructor with given color values
        Color(icl8u r, icl8u g, icl8u b, icl8u a=255){
          this->r() = r;
          this->g() = g;
          this->b() = b;
          this->a() = a;
        }
        /// accesses red value
        inline icl8u &r() { return comp[0]; }

        /// accesses green value
        inline icl8u &g() { return comp[1]; }

        /// accesses blue value
        inline icl8u &b() { return comp[2]; }

        /// accesses alpha value
        inline icl8u &a() { return comp[3]; }

        /// accesses red value (const)
        inline const icl8u &r() const { return comp[0]; }

        /// accesses green value (const)
        inline const icl8u &g() const { return comp[1]; }

        /// accesses blue value (const)
        inline const icl8u &b() const { return comp[2]; }

        /// accesses alpha value (const)
        inline const icl8u &a() const { return comp[3]; }

      };
      
      protected:

      /// internal part list
      std::vector<Part> parts;
      public:
      
      /// returns the parts
      inline const std::vector<Part> &getParts() const { 
        return parts; 
      }
      
      /// clears the list
      inline void clear() { 
        parts.clear(); 
      }
      
      /// adds a given part
      inline void addPart(const Part &part){
        parts.push_back(part);
      }
      /// adds a given part
      inline void addPart(char c, const Any &content){
        addPart(Part(c,content));
      }
      
      /// adds another visualization description
      /** Internally the part-lists are just concatenated */
      VisualizationDescription &operator+=(const VisualizationDescription &other){
        size_t oldSize = parts.size();
        parts.resize(oldSize + other.parts.size());
        std::copy(other.parts.begin(),other.parts.end(),parts.begin()+oldSize);
        return *this;
      }
      
      /// adds two descriptions
      /** Internally the part-lists are just concatenated */
      VisualizationDescription operator+(const VisualizationDescription &other){
        VisualizationDescription sum;
        sum.parts.resize(parts.size()+other.parts.size());
        std::copy(parts.begin(),parts.end(),sum.parts.begin());
        std::copy(other.parts.begin(),other.parts.end(),sum.parts.begin()+parts.size());
        return sum;
      }
      
      /// sets the current draw color (no alpha)
      void color(icl8u r, icl8u g, icl8u b);

      /// sets the current draw color (with alpha)
      void color(icl8u r, icl8u g, icl8u b, icl8u a);

      /// sets the current fill color (no alpha)
      void fill(icl8u r, icl8u g, icl8u b);

      /// sets the current fill color (with alpha)
      void fill(icl8u r, icl8u g, icl8u b, icl8u a);

      /// add a rectangle
      inline void rect(icl32f x, icl32f y, icl32f width, icl32f height){
        addPart('r',Rect32f(x,y,width,height));
      }

      /// add a rectangle
      inline void rect(const Rect32f &r){
        addPart('r',r);
      }

      /// add an ellipse
      inline void ellipse(icl32f x, icl32f y, icl32f width, icl32f height){
        addPart('e',Rect32f(x,y,width,height));
      }

      /// add an ellipse
      inline void ellipse(const Rect32f &r){
        addPart('e',r);
      }

      /// adds a circle (intnerally handeled as ellipse)
      inline void circle(icl32f cx, icl32f cy, icl32f radius){
        addPart('e',Rect32f(cx-radius,cy-radius,2*radius,2*radius));
      }

      /// adds a line (intnerally represented by bounding rectangle)
      inline void line(icl32f x1, icl32f y1, icl32f x2, icl32f y2){
        addPart('l',Rect32f(x1,y1,x2-x1,y2-y1));
      }

      /// adds a line (intnerally represented by bounding rectangle)      
      inline void line(const Point32f &a, const Point32f &b){
        addPart('l',Rect32f(a.x,a.y,b.x-a.x,b.y-a.y));
      }

      /// adds a symbol (supported types are +*x and .)
      inline void sym(char type, icl32f x, icl32f y){
        addPart(type,Point32f(x,y));
      }
      
      /// adds a symbol (supported types are +*x and .)
      inline void sym(char type, const Point32f &p){
        addPart(type,p);
      }
      /// adds text
      /** note that the text must be single lined */
      void text(icl32f x, icl32f y, const std::string &text);
      
      /// adds text
      /** note that the text must be single lined */
      void text(const Point32f &pos, const std::string &text);
      
      /// adds a point (internally handles as a sym of type '.')
      void point(const Point32f &p){
        sym('.',p);
      }
      
      /// adds points (data is in order [x1,y1,x2,y2]
      template<class Iterator>
      void points(Iterator begin, Iterator end){
        addPart('p',std::vector<float>(begin,end));
      }
      
      /// adds points from given float vector
      void points(const std::vector<Point32f> &ps){
        points(&ps[0].x,&ps[0].x + ps.size() * 2);
      }

      /// adds points from given vector of Points
      void points(const std::vector<Point> &ps){
        points(&ps[0].x,&ps[0].x + ps.size() * 2);
      }

      /// template based creation of a polygon 
      template<class Iterator>
      void polygon(Iterator begin, Iterator end){
        addPart('y',std::vector<float>(begin,end));
      }
      
      /// adds polygon from given float vector
      void polygon(const std::vector<Point32f> &ps){
        polygon(&ps[0].x,&ps[0].x + ps.size() * 2);
      }

      /// adds polygon from given vector of Points
      void polygon(const std::vector<Point> &ps){
        polygon(&ps[0].x,&ps[0].x + ps.size() * 2);
      }
      
      /// defines the linewidth
      void linewidth(float w){
        addPart('L',w);
      }
      
      /// defines the point size
      void pointsize(float s){
        addPart('P',s);
      }

    };
    
    /// overloaded ostream operator for VisualizationDescription::Text
    /// syntax: (x,y)text
    inline std::ostream &operator<<(std::ostream &stream, const VisualizationDescription::Text &t){
      return (stream << t.pos << t.text);
    }

    /// overloaded istream operator for VisualizationDescription::Text
    /// syntax: (x,y)text (text must be single lined)
    inline std::istream &operator>>(std::istream &stream, VisualizationDescription::Text &t){
      stream >> t.pos;
      std::getline(stream,t.text);
      return stream;
    }

    /// overloaded ostream operator for VisualizationDescription::Color
    inline std::ostream &operator<<(std::ostream &stream, const VisualizationDescription::Color &c){
      return (stream << c.rgba);
    }

    /// overloaded istream operator for VisualizationDescription::Color
    inline std::istream &operator>>(std::istream &stream, VisualizationDescription::Color &c){
      stream >> c.rgba;
      return stream;
    }
    
    /// adds text
    inline void VisualizationDescription::text(icl32f x, icl32f y, const std::string &text){
      addPart('t',VisualizationDescription::Text(Point32f(x,y),text));
    }
    
      /// adds text
    inline void VisualizationDescription::text(const Point32f &pos, const std::string &text){
      addPart('t',VisualizationDescription::Text(pos,text));
    }
    
    /// sets the current draw color (no alpha)
    inline void VisualizationDescription::color(icl8u r, icl8u g, icl8u b){
      addPart('c',VisualizationDescription::Color(r,g,b));
    }
    
    /// sets the current draw color (with alpha)
    inline void VisualizationDescription::color(icl8u r, icl8u g, icl8u b, icl8u a){
      addPart('c',VisualizationDescription::Color(r,g,b,a));
    }
    
    /// sets the current fill color (no alpha)
    inline void VisualizationDescription::fill(icl8u r, icl8u g, icl8u b){
      addPart('f',VisualizationDescription::Color(r,g,b));
    }
    
    /// sets the current fill color (with alpha)
    inline void VisualizationDescription::fill(icl8u r, icl8u g, icl8u b, icl8u a){
      addPart('f',VisualizationDescription::Color(r,g,b,a));
    }
  }
}
