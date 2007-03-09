#include <iclIO.h>
#include <iclException.h>
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
      else if (sType == "csv")
         return bGzipped ? ioFormatUnknown : ioFormatCSV;
      else return ioFormatUnknown;
   }
   
   // }}}
  
   void analyseHashes (const std::string &sFileName, unsigned int& nHashes, 
                       string::size_type& iSuffixPos) {
      nHashes = 0; iSuffixPos = string::npos;

      // search for first '.'
      string::size_type iTmpPos = sFileName.rfind ('.');
      if (iTmpPos == string::npos) 
         throw ICLException ("cannot detect file type");

      // search for second '.' if the suffix is .gz so far
      const string& sType = sFileName.substr (iTmpPos);
      if (sType == ".gz" && iTmpPos > 0) { // search further for file type
         iSuffixPos = sFileName.rfind ('.', iTmpPos-1);
      }
      if (iSuffixPos == string::npos) iSuffixPos = iTmpPos;
      
      // count number of hashes directly before the suffix
      for (string::const_reverse_iterator start (sFileName.begin() + iSuffixPos),
              it = start, end = sFileName.rend(); it != end; ++it) {
         if (*it != '#') {
            // first pos without hash, count hashes
            nHashes = it - start;
            break;
         }
      }
   }

   void openFile (FileInfo& oInfo, const char *pcMode) throw (FileOpenException) {
      switch (oInfo.eFileFormat) {
        case ioFormatPNM: 
        case ioFormatICL:
           // these modes support gzipped format
           // enable gzipped for reading always
           if (strchr (pcMode, 'r')) oInfo.bGzipped = true;
           // for writing, we keep the setting from file name
           break;
        case ioFormatJPG:
        case ioFormatCSV:
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

} //namespace
