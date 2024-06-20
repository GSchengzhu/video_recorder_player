#ifndef FFMPEG_HELPER_H
#define FFMPEG_HELPER_H
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavcodec/packet.h"
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
}

typedef struct 
{
    AVStream *st;
    AVCodecContext *enc;

    AVFrame *frame;
    AVFrame *tmpFrame;
    AVPacket *pkt;

} OutputStream;

#endif