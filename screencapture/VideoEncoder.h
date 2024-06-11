#include <X11/X.h>
#include <X11/Xlib.h>
class VideoEncoder
{
public:
    VideoEncoder() = default;
    bool init();
    bool reset();
    bool encoderXimage(XImage* inputImage);
  
private:
    int m_frameIndex = 0;
};