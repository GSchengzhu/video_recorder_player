#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
class XImageScreenCapture
{
public:
    XImageScreenCapture() = default;
    ~XImageScreenCapture();

    bool init();
    XImage* captureXImage();

private:
    XImage *m_image;
    XShmSegmentInfo  *m_shm_segment_info;
    Display *m_display;
    Window m_window;
};