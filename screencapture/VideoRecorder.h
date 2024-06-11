#include <string>
#include <thread>
#include "VideoEncoder.h"
#include "ScreenCapture.h"
#include "AudioEncoder.h"
class VideoRecorder
{
public:
    VideoRecorder() = default;
    ~VideoRecorder();
    bool init(std::string path);

    bool setAudioStatus(bool need){  m_needAudio = need;  }
    bool audioStatus() {  return m_needAudio;  }
    
    void setRecorderPath(std::string recorderPath){  m_recorderPath = recorderPath; }
    std::string recorderPath(){  return m_recorderPath; }

    bool Start();
    bool Stop();
    bool getRecorderStatus();

private:
    void startRecorder();

private:
    std::string m_recorderPath;
    FILE* m_recorderFile;
    bool m_needAudio = false;
    bool m_running = false;

    VideoEncoder* m_videoEncoder;
    XImageScreenCapture* m_screenCapture;
    AudioEncoder * m_audioEncoder;
    std::thread m_workThread;
};