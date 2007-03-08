#include <ICLRegionDetector.h>

namespace icl{
  namespace regiondetector{
    RegionDetector::RegionDetector(int iW, int iH, int iMinSize,int iMaxSize, int iMinValue, int iMaxValue):
      m_iW(iW),m_iH(iH),m_iDim(iW*iH),m_iMinSize(iMinSize),
      m_iMaxSize(iMaxSize),m_iMinValue(iMinValue),m_iMaxValue(iMaxValue){

      m_ppoLim = new RegionDetectorBlobPart*[m_iDim];
      memset(m_ppoLim,0,m_iDim*sizeof(RegionDetectorBlobPart*));
      
      m_poBlobPartMM = new BlobPartMemoryManager(1000);
      m_poScanLineMM = new ScanLineMemoryManager(10000);

      m_poBlobList = new BlobList(0);
    }
    
    RegionDetector::~RegionDetector(){
      delete [] m_ppoLim;
      delete m_poBlobPartMM;
      delete m_poScanLineMM;
      for(int i=0;i<m_poBlobList->size();i++){
        delete (*m_poBlobList)[i];
      }
      delete m_poBlobList;
    }


    void RegionDetector::setSize(const Size &size){
      if(m_iW != size.width || m_iH != size.height){
        m_iW = size.width;
        m_iH = size.height;
        m_iDim = m_iW*m_iH;
        
        delete [] m_ppoLim;
        m_ppoLim = new RegionDetectorBlobPart*[m_iDim];
        memset(m_ppoLim,0,m_iDim*sizeof(RegionDetectorBlobPart*));
      }
    }

    
    void RegionDetector::setMinSize(int iMinSize){
      this->m_iMinSize = iMinSize;
    }
    void RegionDetector::setMaxSize(int iMaxSize){
      this->m_iMaxSize = iMaxSize;
    }
    void RegionDetector::setMinValue(int iMinValue){
      this->m_iMinValue = iMinValue;
    }
    void RegionDetector::setMaxValue(int iMaxValue){
      this->m_iMaxValue = iMaxValue;
    }
    
    BlobList *RegionDetector::find_blobs(icl8u *pucData){
      //TODO remove
      icl8u *data = pucData;
      int i;
      int start_of_curr_pix_line = 0;
     

      // for the Compability of the *old* code
      RegionDetectorBlobPart **lim = m_ppoLim;
      int w = m_iW;
      int h = m_iH;
     
      // first pixel
      lim[0] = m_poBlobPartMM->next();
      lim[0]->clear();
      start_of_curr_pix_line = 0;

      // first row
      for(i=1;i<w;i++){
        if(data[i]==data[i-1]){
          lim[i]=lim[i-1];
        }else{
          RegionDetectorScanLine *pl = m_poScanLineMM->next();
          pl->update(0,start_of_curr_pix_line,i-1,w,data);
          lim[i-1]->add(pl);
          start_of_curr_pix_line = i;
          lim[i] = m_poBlobPartMM->next();
          lim[i]->clear();
        }
      }
      RegionDetectorScanLine *pl = m_poScanLineMM->next();
      pl->update(0,start_of_curr_pix_line,w-1,w,data);
      lim[w-1]->add(pl);
          
      //rest of the image [1..w-1]x[1..h-1]
      RegionDetectorBlobPart **r_curr_line;
      RegionDetectorBlobPart **r_last_line;
      RegionDetectorBlobPart *r_to_delete;
      RegionDetectorBlobPart *r_to_keep;
      icl8u *im_curr_line;
      icl8u *im_last_line;
      int x,y,j;

      for(y=1;y<h;y++){
        r_curr_line = lim+y*w;
        r_last_line = r_curr_line-w;
        im_curr_line = data+y*w;
        im_last_line = im_curr_line-w;

        //1st pix
        if(im_curr_line[0] == im_last_line[0]){
          r_curr_line[0] = r_last_line[0];
        }
        else{
          r_curr_line[0] = m_poBlobPartMM->next();
          r_curr_line[0]->clear();
        }
      
        //rest of pixels
        start_of_curr_pix_line = 0;
         
        for(x=1;x<w;x++){
          if( im_curr_line[x]==im_curr_line[x-1] ){
        
            if(im_curr_line[x]==im_last_line[x] && r_curr_line[x-1] != r_last_line[x] ){
              r_to_delete = r_last_line[x];
              r_to_keep = r_curr_line[x-1];
                  
              r_curr_line[x]=r_to_keep;
              r_to_keep->add(r_to_delete);
        
              for(j=0;j<x;j++){
                if(r_curr_line[j] == r_to_delete){
                  r_curr_line[j] = r_to_keep;
                }
              }
              for(j=x+1;j<w;j++){
                if(r_last_line[j] == r_to_delete){
                  r_last_line[j] = r_to_keep;
                }
              }
            }
            else{
              r_curr_line[x]=r_curr_line[x-1];
            }   
          }else{
            if(im_curr_line[x]==im_last_line[x]){
              r_curr_line[x]=r_last_line[x];
            }else{
              r_curr_line[x]=m_poBlobPartMM->next();
              r_curr_line[x]->clear();
            }
            RegionDetectorScanLine *pl = m_poScanLineMM->next();
            pl->update(y,start_of_curr_pix_line,x-1,w,data);
            r_curr_line[x-1]->add(pl);
            start_of_curr_pix_line = x;
          }
        }
        RegionDetectorScanLine *pl = m_poScanLineMM->next();
        pl->update(y,start_of_curr_pix_line,w-1,w,data);
        r_curr_line[w-1]->add(pl);
      }
      //Creating RegionDetectorBlobs recursivly
      //delete old blobs !!

      for(BlobList::iterator it = m_poBlobList->begin();it!= m_poBlobList->end();it++){
        delete *it;
      }
      m_poBlobList->clear();
      int iSize;
      icl8u ucVal;
      for(BlobPartMemoryManager::iterator it = m_poBlobPartMM->begin();it != m_poBlobPartMM->end();it++){
        if(!((*it)->is_inside_other_region())){
          RegionDetectorBlob *b = new RegionDetectorBlob(*it);
          iSize = b->getSize();
          ucVal = b->getVal();
          if((iSize >= m_iMinSize) && (iSize <= m_iMaxSize) && (ucVal >= m_iMinValue) && (ucVal <= m_iMaxValue)){
            m_poBlobList->push_back(b);
          }else{
            delete b;
          }
        }
      }
            
      m_poBlobPartMM->clear();
      m_poScanLineMM->clear();

      return m_poBlobList;
    }
  }
}
