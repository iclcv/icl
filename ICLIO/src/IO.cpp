#include "IO.h"
#include "Exception.h"
#include <zlib.h>
#include <stdio.h>

using namespace std;

static string& toLower (string& s) {
   for (string::iterator it=s.begin(), end=s.end(); it != end; ++it)
      *it = std::tolower (*it);
   return s;
}

namespace icl {

   ioformat getFileType (const std::string &sFileName, bool& bGzipped)
      // {{{ open 
   {
      bGzipped = false;
      string::size_type iTmpPos = sFileName.rfind ('.');
      if (iTmpPos == string::npos) return ioFormatUnknown;

      // extract suffix after separating '.' and convert to lower case
      string sType (sFileName.substr (iTmpPos+1)); toLower (sType);
      if (sType == "gz") { // search further for file type
         bGzipped = true;
         if (iTmpPos == 0) return ioFormatUnknown; // ".gz" only
         string::size_type iSuffixPos = sFileName.rfind ('.', iTmpPos-1);
         if (iSuffixPos == string::npos) sType = "";
         else sType = sFileName.substr (iSuffixPos+1, iTmpPos-iSuffixPos-1);
         toLower (sType);
      }

      if (sType == "ppm" || sType == "pgm" || sType == "pnm") 
         return ioFormatPNM;
      else if (sType == "icl") 
         return ioFormatICL;
      else if (sType == "jpg" || sType == "jpeg") 
         return bGzipped ? ioFormatUnknown : ioFormatJPG;
      else if (sType == "seq") 
         return bGzipped ? ioFormatUnknown : ioFormatSEQ;
      else return ioFormatUnknown;
   }
   
   // }}}
  
   void openFile (FileInfo& oInfo, const char *pcMode) throw (FileOpenException) {
      switch (oInfo.eFileFormat) {
        case ioFormatPNM: case ioFormatICL:
           // these modes support gzipped format
           // enable gzipped for reading always
           if (strchr (pcMode, 'r')) oInfo.bGzipped = true;
           // for writing, we keep the setting from file name
           break;
        case ioFormatJPG:
           // these modes suppose plain files only
           oInfo.bGzipped = false;
           break;
        default:
           // unknown file format: can't open it ;-)
           throw FileOpenException (oInfo.sFileName);
           break;
      }

      if (oInfo.bGzipped)
         oInfo.fp = gzopen (oInfo.sFileName.c_str(), pcMode);
      else
         oInfo.fp = fopen (oInfo.sFileName.c_str(), pcMode);

      // if file pointer fp is still NULL, the file can't be opened
      if (!oInfo.fp) throw FileOpenException (oInfo.sFileName);
   }

   void closeFile (FileInfo& oInfo) {
      if (!oInfo.fp) return;
      if (oInfo.bGzipped) gzclose (oInfo.fp); else fclose ((FILE*) oInfo.fp);
      oInfo.fp = 0;
   }

  // Routine to replace the standard jpeg error_exit routine.
  // The caller has to activate it by calling setjmp (err->setjmp_buffer)
  void icl_jpeg_error_exit (j_common_ptr cinfo) {
     /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
     struct icl_jpeg_error_mgr* err = (struct icl_jpeg_error_mgr*) cinfo->err;
     
     /* Always display the message. */
     /* We could postpone this until after returning, if we chose. */
     (*cinfo->err->output_message) (cinfo);
     
     /* Return control to the setjmp point */
     longjmp(err->setjmp_buffer, 1);
  }


#if 0 // not needed anymore?
   //--------------------------------------------------------------------------
   void splitString(const string& line, 
                    const string& separators,
                    vector<string> &words) 
   {
      // {{{ open
      FUNCTION_LOG("(string, char*, vector<string>)");

      //---- Initialisation ----
      std::string::size_type a = 0, e;
      words.clear();

      //---- Split into substrings ----
      while ((a = line.find_first_not_of(separators, a)) != std::string::npos) 
      {
         e = line.find_first_of(separators, a);
         if (e != std::string::npos) 
         {
            if (line.substr(a, e-a)[0] != '#')
            {
               words.push_back(line.substr(a, e-a));
            }
            a = e + 1;
         }
         else 
         {
            words.push_back(line.substr(a));
            break;
         }
      }  
   }
  
   // }}}
  
  
   //--------------------------------------------------------------------------
   string number2String(int i)
   {
      // {{{ open
      FUNCTION_LOG("(int)");
      //---- Initialization ----
      ostringstream oss;
    
      //---- conversion ----
      oss << i;
    
      //---- return ----
      return oss.str();
   }
  
   // }}}
#endif

} //namespace
