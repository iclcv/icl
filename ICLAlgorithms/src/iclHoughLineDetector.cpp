#include <iclHoughLineDetector.h>
#include <iclException.h>



namespace icl{
  HoughLineDetector::HoughLineDetector(int maxLines,
                                       const Size &sizeHint,
                                       float deltaAngular,
                                       float deltaRadial,
                                       int minPointsPerLine,
                                       SmartPtr<UnaryOp> preprocessor):
    m_maxLines(maxLines),m_preprocessor(preprocessor),
    m_roiDimForBuffer(sizeHint),m_deltaAngular(deltaAngular),
    m_deltaRadial(deltaRadial),m_minPointsPerLine(minPointsPerLine),
    m_detectionLineBuffer(0),m_preprocessingBuffer(0)
  {
    updateBufferSize();
    if(maxLines <= 0){
      throw ICLException("max line count must be positive integer");
    }
  }

  HoughLineDetector::~HoughLineDetector(){
    ICL_DELETE_ARRAY((IppPointPolar*&)m_detectionLineBuffer);
    ICL_DELETE(m_preprocessingBuffer);
  }

  void HoughLineDetector::updateBufferSize(){
    int size = 0;
    IppPointPolar delta = {m_deltaRadial,m_deltaAngular};
    IppStatus status = ippiHoughLineGetSize_8u_C1R(m_roiDimForBuffer,delta,m_maxLines,&size);
    if(status != ippStsNoErr){
      ERROR_LOG("error calculating hough buffer size: " << ippGetStatusString(status) << std::endl
                << "buffer size was not updated!"); 
    }else{
      m_buffer.resize(size);
    }
    
    ICL_DELETE_ARRAY((IppPointPolar*&)m_detectionLineBuffer);
    m_detectionLineBuffer = new IppPointPolar[m_maxLines];
  }
    
  const std::vector<HoughLine> HoughLineDetector::detectLines(const ImgBase *image){
    if(!image){
      ERROR_LOG("input image is NULL");
      m_lines.clear();
      return m_lines;
    }
    if(m_preprocessor){
      m_preprocessor->apply(image,&m_preprocessingBuffer);
      if(!m_preprocessingBuffer){
        ERROR_LOG("preprocessor produced no image");
        m_lines.clear();
        return m_lines;
      }else{
        image = m_preprocessingBuffer;
      }
    }
    if(image->getChannels() != 1){
      ERROR_LOG("input image (or preprocessed image) has more than on channels" << std::endl
                << "hough detection works only on one channel images -> using channel 0");
    }
    if(image->getDepth() != depth8u){
      image->convert(&m_inputBuffer8u);
      image = &m_inputBuffer8u;
    }
    if(image->getROISize().getDim() > m_roiDimForBuffer.getDim()){
      m_roiDimForBuffer = image->getROISize();
      updateBufferSize();
    }
    int linesFound = 0;
    IppPointPolar *linesDetected = (IppPointPolar*)m_detectionLineBuffer;
    IppPointPolar delta = {m_deltaRadial, m_deltaAngular};
    IppStatus status = ippiHoughLine_8u32f_C1R(image->asImg<icl8u>()->getROIData(0),
                                               image->getLineStep(),image->getROISize(),delta,
                                               m_minPointsPerLine,linesDetected,
                                               m_maxLines,&linesFound,m_buffer.data());
    
    if(status != ippStsNoErr){
      ERROR_LOG("error applying ippiHoughLine8u32f_C1R(..): " << ippGetStatusString(status) << std::endl
                << "no lines detected!");
      m_lines.clear();
      return m_lines;
    }
    
    m_lines.resize(linesFound);
    for(int i=0;i<linesFound;++i){
      m_lines[i] = HoughLine(linesDetected[i].rho,linesDetected[i].theta);
    }
    return m_lines;
  }
}
