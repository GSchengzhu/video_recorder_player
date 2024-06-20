#include "ScreenCapture.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <cstdio>
#include <sys/ipc.h>
#include <sys/shm.h>

bool XImageScreenCapture::init()
{
    m_display = XOpenDisplay(NULL);
    if(!m_display)
    {
        return false;
    }
    printf("xopen display................\n");
    m_window = RootWindow(m_display, 0);
    if(!m_window)
    {
        return false;
    }
    printf("RootWindow................\n");
    int height = DisplayHeight(m_display, 0);
    int width = DisplayWidth(m_display, 0);
    
    XImage *windowImage = XGetImage(m_display, m_window,0,0, width,height, AllPlanes, ZPixmap);
    if(!windowImage)
    {
        return false;
    }
    printf("XGetImage................\n");
    m_shm_segment_info = new XShmSegmentInfo;
    m_image = XShmCreateImage(m_display, DefaultVisual(m_display, 0), windowImage->depth, ZPixmap, NULL, m_shm_segment_info, width, height);
    if(!m_image)
    {
        return false;
    }

    XDestroyImage(windowImage);
    printf("shmget\n");
    // int screenSize = height*width*windowImage->bits_per_pixel/8;
    m_shm_segment_info->shmid = -1;
    m_shm_segment_info->shmid = shmget(IPC_PRIVATE, m_image->bytes_per_line*m_image->height, IPC_CREAT | 0777);
    if(m_shm_segment_info->shmid == -1)
    {
        return false;
    }
    printf("shmat\n");
    m_shm_segment_info->shmaddr = m_image->data = (char*)shmat(m_shm_segment_info->shmid,
                                                                NULL, 0);
    if(!XShmAttach(m_display, m_shm_segment_info))
    {
        return false;
    }
    printf("XShmAttach\n");
    
    return true;
}

XImageScreenCapture::~XImageScreenCapture()
{
    if(m_shm_segment_info && m_shm_segment_info->shmid != -1 && m_display)
    {
        XShmDetach(m_display, m_shm_segment_info);
        shmdt(m_shm_segment_info->shmaddr);
        shmctl(m_shm_segment_info->shmid, IPC_RMID, 0);

        delete m_shm_segment_info;
        m_shm_segment_info = NULL;
    }

    if(m_image)
    {
         XDestroyImage(m_image);
         m_image = NULL;
    }
}

XImage* XImageScreenCapture::captureXImage()
{
    if(!XShmGetImage(m_display, m_window,  m_image, 0, 0, AllPlanes))
    {
        printf("capture image failed\n");
        return NULL;
    }

    return m_image;
}