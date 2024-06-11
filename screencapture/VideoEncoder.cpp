#include "VideoEncoder.h"
#include "libavformat/avformat.h"
extern "C"
{
    #include <libavcodec/avcodec.h>
}

bool VideoEncoder::init()
{   
    AVFormatContext *avformatContext = avformat_alloc_context();
    AVOutputFormat *avoutputFormat = avformat_alloc_output_context2(&avformatContext, const AVOutputFormat *oformat, const char *format_name, const char *filename)
    AVStream *avVideoStream = avformat_new_stream(AVFormatContext *s, const AVCodec *c)


    return true;
}