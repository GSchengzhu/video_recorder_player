#include "./screencapture/VideoRecorder.h"
#include <unistd.h>

int main()
{
    VideoRecorder video;
    video.init("/home/sos/test.h264");
    video.Start();
    while (true) {
        pause();
    }

    return 0;
}