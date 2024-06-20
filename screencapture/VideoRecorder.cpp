#include "VideoRecorder.h"
#include <X11/Xlib.h>
#include <csignal>
#include <cstdio>
#include <string>
#include <thread>

bool VideoRecorder::m_running = false;

void SignalIntHandle(int sig)
{
    VideoRecorder::m_running = false;
}

VideoRecorder::~VideoRecorder()
{
    if(m_videoEncoder)
    {
        delete m_videoEncoder;
    }

    if(m_screenCapture)
    {
        delete m_screenCapture;
    }

    if(m_audioEncoder)
    {
        delete m_audioEncoder;
    }

    if(m_recorderFile)
    {
        fclose(m_recorderFile);
    }
}

bool VideoRecorder::init(std::string path)
{
    signal(SIGINT, SignalIntHandle);
    m_videoEncoder = new VideoEncoder;
    m_videoEncoder->setPath(path);
    if(!m_videoEncoder->initEncode())
    {
        printf("video encoder init failed\n");
        return false;
    }

    m_screenCapture = new XImageScreenCapture;
    if(!m_screenCapture->init())
    {
        printf("screen capture init failed\n");
        return false;
    }
    setRecorderPath(path);
}

bool VideoRecorder::Start()
{
    //  if(m_recorderPath.empty())
    //  {
    //     printf("recorder path empty\n");
    //     return false;
    //  }

    //  if(m_needAudio)
    //  {
    //     m_audioEncoder = new AudioEncoder;
    //     if(m_audioEncoder->init())
    //     {
    //         printf("audio encoder failed\n");
    //         return false;
    //     }
    //  }

    //  m_recorderFile = fopen(m_recorderPath.c_str(), "wb+");
    //  if(!m_recorderFile)
    //  {
    //     return false;
    //  }
     
     m_running = true;
     m_workThread = std::thread(&VideoRecorder::startRecorder,this);
     m_workThread.detach();

     printf("start recording\n");
}

bool VideoRecorder::Stop()
{
    m_running = false;
}

bool VideoRecorder::getRecorderStatus()
{
    return m_running;
}

void VideoRecorder::startRecorder()
{
    while (m_running) 
    {
        XImage* image = m_screenCapture->captureXImage();
        m_videoEncoder->encoderXimage(image);    
    }
    m_videoEncoder->fflushEncoder();
}
