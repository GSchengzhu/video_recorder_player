#include "libavcodec/avcodec.h"
#include "libavcodec/packet.h"
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
typedef struct _OutputStream
{
    AVStream *st;
    AVCodecContext *enc;

    AVFrame *frame;
    AVFrame *tmpFrame;
    AVPacket *pkt;

} OutputStream;