#include "../common/ffmpeg_helper.h"
#include <X11/X.h>
#include <X11/Xlib.h>
extern "C"
{
    #include <libswscale/swscale.h>
}


class VideoEncoder
{
public:
    VideoEncoder() = default;
    bool init();
    bool reset();
    bool encoderXimage(XImage* inputImage);
    bool fflushEncoder();

    void setFrameSize(int width,int height) { m_frameW = width;m_frameH = height; }
    int getFrameWidth() {  return m_frameW; }
    int getFrameHeight() {  return m_frameW; }

  
private:
    int m_frameW = 0;
    int m_frameH = 0;
    int m_frameIndex = 0;
    OutputStream m_videoStream = {0};
    SwsContext *m_swsCtx;
};