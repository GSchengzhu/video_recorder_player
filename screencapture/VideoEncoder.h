#include "../common/ffmpeg_helper.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <string>
extern "C"
{
    #include <libswscale/swscale.h>
}


class VideoEncoder
{
public:
    VideoEncoder() = default;
    ~VideoEncoder();
    bool initEncode();
    bool setPath(std::string path){  m_path = path;  }
    std::string path() {  return m_path;  }
    bool reset();
    bool encoderXimage(XImage* inputImage);
    bool fflushEncoder();

    void setFrameSize(int width,int height) { m_frameW = width;m_frameH = height; }
    int getFrameWidth() {  return m_frameW; }
    int getFrameHeight() {  return m_frameW; }

  
private:
    std::string m_path;
    int m_frameW = 0;
    int m_frameH = 0;
    int m_frameIndex = 0;
    int m_imageBeforeW = 0;
    int m_imageBeforeH = 0;
    OutputStream m_videoStream = {0};
    SwsContext *m_swsCtx;
    AVFormatContext *m_avformatContext = NULL;
};