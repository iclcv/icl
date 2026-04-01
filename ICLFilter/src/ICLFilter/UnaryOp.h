// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Configurable.h>
#include <ICLCore/Image.h>
#include <ICLFilter/OpROIHandler.h>

namespace icl{
  namespace filter{


    /// Abstract Base class for Unary Operators \ingroup UNARY
    /** A list of unary operators can be found here:\n
        \ref UNARY
    **/
    class ICLFilter_API UnaryOp : public utils::Configurable{
      void initConfigurable();

      public:

      /// Explicit empty constructor
      UnaryOp();

      /// Explicit copy constructor
      UnaryOp(const UnaryOp &other);

      /// Explicit declaration of the assignment operator
      UnaryOp &operator=(const UnaryOp &other);


      /// Destructor
      virtual ~UnaryOp();

      /// Pure virtual apply — all subclasses implement this
      virtual void apply(const core::Image &src, core::Image &dst) = 0;

      /// Legacy ImgBase** wrapper — final, delegates to Image-based apply
      virtual void apply(const core::ImgBase *src, core::ImgBase **dst) final;

      /// Single-arg apply: uses internal buffer, returns reference to it
      const core::Image& apply(const core::Image &src);

      /// function operator
      inline const core::Image& operator()(const core::Image &src){
        return apply(src);
      }


      /// sets if the image should be clip to ROI or not
      /**
        @param bClipToROI true=yes, false=no
      */
      void setClipToROI (bool bClipToROI) {
        m_oROIHandler.setClipToROI(bClipToROI);
        prop("UnaryOp.clip to ROI").value = bClipToROI ? "on" : "off";
        call_callbacks("UnaryOp.clip to ROI",this);
      }

      /// sets if the destination image should be adapted to the source, or if it is only checked if it can be adapted.
      /**
        @param bCheckOnly true = destination image is only checked, false = destination image will be checked and adapted.
      */
      void setCheckOnly (bool bCheckOnly) {
        m_oROIHandler.setCheckOnly(bCheckOnly);
        prop("UnaryOp.check only").value = bCheckOnly ? "on" : "off";
        call_callbacks("UnaryOp.check only",this);
      }

      /// returns the ClipToROI status
      /**
        @return true=ClipToROI is enable, false=ClipToROI is disabled
      */
      bool getClipToROI() const { return m_oROIHandler.getClipToROI(); }

      /// returns the CheckOnly status
      /**
        @return true=CheckOnly is enable, false=CheckOnly is disabled
      */
      bool getCheckOnly() const { return m_oROIHandler.getCheckOnly(); }


      /// sets value of a property (always call call_callbacks(propertyName) or Configurable::setPropertyValue)
      virtual void setPropertyValue(const std::string &propertyName, const utils::Any &value);

      /// Creates a UnaryOp instance from given string definition
      /** Supported definitions have the followin syntax:
          OP_SPEC<(PARAM_LIST)>

          examples are:
          - sobelX3x3
          - median(5x3)
          - closeBorder(3x3)

          A complete list of OP_SPECS can be obtained by the static listFromStringOps function.
          Each specific parameter list's syntax is accessible using the static getFromStringSyntax function.

      */
      static UnaryOp *fromString(const std::string &definition);

      /// gives a string syntax description for given opSpecifier
      /** opSpecifier must be a member of the list returned by the static function listFromStringOps */
      static std::string getFromStringSyntax(const std::string &opSpecifier);

      /// returns a list of all supported OP_SPEC values for the fromString function
      static std::vector<std::string> listFromStringOps();

      /// creates, applies and releases a UnaryOp defined by given definition string
      static void applyFromString(const std::string &definition,
                                  const core::ImgBase *src,
                                  core::ImgBase **dst);

      protected:

      /// Image-based prepare: ensures dst matches the given parameters
      bool prepare(core::Image &dst, core::depth d, const utils::Size &s,
                   core::format fmt, int channels, const utils::Rect &roi,
                   utils::Time t = utils::Time::null);

      /// Image-based prepare: ensures dst matches src's parameters
      bool prepare(core::Image &dst, const core::Image &src);

      /// Image-based prepare: matches src but with explicit depth
      bool prepare(core::Image &dst, const core::Image &src, core::depth d);

      /// Legacy prepare (for subclasses not yet migrated)
      bool prepare (core::ImgBase **ppoDst, core::depth eDepth, const utils::Size &imgSize,
                    core::format eFormat, int nChannels, const utils::Rect& roi,
                    utils::Time timestamp=utils::Time::null){
        return m_oROIHandler.prepare(ppoDst, eDepth,imgSize,eFormat, nChannels, roi, timestamp);
      }

      /// Legacy prepare
      virtual bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc) {
        return m_oROIHandler.prepare(ppoDst, poSrc);
      }

      /// Legacy prepare
      virtual bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc, core::depth eDepth) {
        return m_oROIHandler.prepare(ppoDst, poSrc, eDepth);
      }

      private:

      OpROIHandler m_oROIHandler;

      core::Image m_buf;
    };


  #define DYNAMIC_UNARY_OP_CREATION_FUNCTION(NAME)       \
    extern "C" {                                         \
      UnaryOp *create_##NAME(const std::string &s){      \
        return NAME(s);                                  \
      }                                                  \
    }

  } // namespace filter
}
