#include <iclIO.h>
#include <iclException.h>
#include <iclImg.h>
#include <zlib.h>
#include <stdio.h>
#include <cctype>
#include <map>



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
#ifdef WITH_JPEG_SUPPORT
      else if (sType == "jpg" || sType == "jpeg") 
         return bGzipped ? ioFormatUnknown : ioFormatJPG;
#endif
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
#ifdef WITH_JPEG_SUPPORT
        case ioFormatJPG:
#endif
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
#ifdef WITH_JPEG_SUPPORT
  void icl_jpeg_error_exit (j_common_ptr cinfo) {
     /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
     struct icl_jpeg_error_mgr* err = (struct icl_jpeg_error_mgr*) cinfo->err;
     
     /* Always display the message. */
     /* We could postpone this until after returning, if we chose. */
     (*cinfo->err->output_message) (cinfo);
     
     /* Return control to the setjmp point */
     longjmp(err->setjmp_buffer, 1);
  }
#endif

  struct OffsPtr {
    OffsPtr(const Point &offs=Point::null, icl8u* img=0, int width=0):
      offs(offs), img(img), width(width){}
    Point offs;
    icl8u *img;
    int width;
  };
  
  template<class T>
  void labelImage(Img<T> *image,const char*txt, const std::map<char,OffsPtr> &m){
    static const Point OFS(5,5);

    Rect roi = Rect(OFS.x-1,OFS.y-1,strlen(txt)*(7+2),7+2);
    if(Rect(Point::null,image->getSize()).contains(roi)){
      Img<T> *boxImage = image->shallowCopy(roi);
      boxImage->clear();
      delete boxImage;
    }
    
    int letterIdx = 0;
    for(const char *p = txt; *p; p++){

      const std::map<char,OffsPtr>::const_iterator it = m.find(*p);
      if(it == m.end()) continue;
      const OffsPtr &op = it->second;
      //      printf("tying to draw \"%c\"  pointer offs = (%d,%d)\n",*p,(op.offs.x),op.offs.y);
      
      
      for(int c=0;c < image->getChannels();c++){
        int xStartLetter = op.offs.x;
        int xEndLetter = xStartLetter+7;
        int xStartImage = OFS.x + letterIdx*(7+2);
        for(int xL=xStartLetter,xI=xStartImage; xL < xEndLetter ; xI++,xL++){
          if(xI < image->getWidth() ){
            int yStartLetter = 0;
            int yEndLetter = 7;
            int yStartImage = OFS.y;
            for(int yL=yStartLetter, yI= yStartImage; yL < yEndLetter; yI++,yL++){
              if(yI < image->getHeight()){
                if(char(op.img[xL+yL*op.width]) == '#'){
                  (*image)(xI,yI,c) = 255;
                }else{
                  (*image)(xI,yI,c) = 0;
                }
              }
            }
          }
        }
      }
      letterIdx++;
    } 
  }
  
  void labelImage(ImgBase *image,const std::string &label){
    ICLASSERT_RETURN( image );
    ICLASSERT_RETURN( label.length() );

    static char ABC_AM[] = 
    " ##### ######  ############ ############## ##### #     ################     ##      #     #"
    "#     ##     ##      #     ##      #      #     ##     #   #         ##    # #      ##   ##"
    "#     ##     ##      #     ##      #      #      #     #   #         ##   #  #      # # # #"
    "#     ####### #      #     ################  ###########   #         #####   #      #  #  #"
    "########     ##      #     ##      #      #     ##     #   #         ##   #  #      #     #"
    "#     ##     ##      #     ##      #      #     ##     #   #         ##    # #      #     #"
    "#     #######  ############ ########       ##### #     ############## #     #########     #";

    static char ABC_NZ[] = 
    "#     # ##### ######  ##### ######  ##### ########     ##     ##     ##     ##     ########"
    "##    ##     ##     ##     ##     ##     #   #   #     ##     ##     # #   #  #   #      # "
    "# #   ##     ##     ##     ##     ##         #   #     # #   # #     #  # #    # #      #  "
    "#  #  ##     ####### #     #######  #####    #   #     # #   # #  #  #   #      #      #   "
    "#   # ##     ##      # ### ##   #        #   #   #     #  # #  # # # #  # #     #     #    "
    "#    ###     ##      #    ###    # #     #   #   #     #  # #  ##   ## #   #    #    #     " 
    "#     # ##### #       ##### #     # #####    #    #####    #   #     ##     #   #   #######";
                                                                                                
    static char ABC_09[] = 
    " #####      ## #####  ##### #   #  ####### ##### ####### #####  ##### "
    "#     #   ## ##     ##     ##   #  #      #     #      ##     ##     #"
    "#     # ##   #      #      ##   #  #      #           # #     ##     #"
    "#     ##     #  ####  ##### ############# ######  ###### #####  ######"
    "#     #      ###           #    #        ##     #   #   #     #      #"
    "#     #      ##      #     #    #  #     ##     #  #    #     ##     #"
    " #####       ######## #####     #   #####  #####  #      #####  ##### ";

    static char ABC_EX[] = 
    "          #    #   #  #   #  #####  #    # ###      #       #    #     # # #                                   #"
    "          #    #   # ########  #  ## #  # #   #     #      #      #     ###     #                             # "
    "          #           #   #    #  # #  #   ###             #      #    #####    #                            #  "
    "          #           #   #  #####    #    # #            #        #    ###   #####         #####           #   "
    "          #           #   # #  #     #  # #   ##           #      #    # # #    #       #                  #    "
    "                     ########  #  # #  # ##   ##           #      #             #      #             ##   #     "
    "          #           #   #  ##### #    #  #### #           #    #                    #              ##  #      ";
    static std::map<char,OffsPtr> m;
    static bool first = true;

    if(first){
      first = false;
      for(int c='a', xoffs=0; c<='m'; c++, xoffs+=7){
        m[c] = OffsPtr(Point(xoffs,0),reinterpret_cast<icl8u*>(const_cast<char*>(ABC_AM)),13*7);
        m[c-32] = OffsPtr(Point(xoffs,0),reinterpret_cast<icl8u*>(const_cast<char*>(ABC_AM)),13*7);
      }
      for(int c='n', xoffs=0; c<='z'; c++, xoffs+=7){
        m[c] = OffsPtr(Point(xoffs,0),reinterpret_cast<icl8u*>(const_cast<char*>(ABC_NZ)),13*7);
        m[c-32] = OffsPtr(Point(xoffs,0),reinterpret_cast<icl8u*>(const_cast<char*>(ABC_NZ)),13*7);
      }
      for(int c='0', xoffs=0; c<='9'; c++, xoffs+=7){
        m[c] = OffsPtr(Point(xoffs,0),reinterpret_cast<icl8u*>(const_cast<char*>(ABC_09)),10*7);
      }
      for(int c=' ', xoffs=0; c<='/'; c++, xoffs+=7){
        m[c] = OffsPtr(Point(xoffs,0),reinterpret_cast<icl8u*>(const_cast<char*>(ABC_EX)),16*7);
      }
    }
    
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: labelImage<icl##D>(image->asImg<icl##D>(),label.c_str(),m); break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
  }

} //namespace
