#include "VideoEncoder.h"
#include "../common/ffmpeg_helper.h"
#include "../common/utils.h"
#include "libavcodec/codec.h"
#include "libavcodec/codec_id.h"
#include "libavcodec/packet.h"
#include "libavformat/avformat.h"
#include "libavutil/dict.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libavutil/pixfmt.h"
#include "libavutil/rational.h"
#include "libavutil/samplefmt.h"
#include "libswscale/swscale.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <vector>
extern "C"
{
    #include <libavcodec/avcodec.h>
}

bool VideoEncoder::init()
{   
    if(m_frameH == 0 || m_frameW == 0)
    {
        logging("frame size not set");
        return false;
    }

    // const AVOutputFormat *avoutputFormat = NULL;

    AVFormatContext *avformatContext = NULL;
    avformat_alloc_output_context2(&avformatContext, NULL, "flv", NULL);
    // avoutputFormat = avformatContext->oformat;

    //init videostream
    const AVCodec* videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    m_videoStream.pkt = av_packet_alloc();
    // videoStream.st = avformat_new_stream(avformatContext,NULL);
    m_videoStream.enc = avcodec_alloc_context3(videoCodec);

    // videoStream.enc->codec_id = avoutputFormat->video_codec;
    m_videoStream.enc->codec_id = AV_CODEC_ID_H264;
    m_videoStream.enc->bit_rate = 4000000;
    m_videoStream.enc->width = m_frameW;
    m_videoStream.enc->height = m_frameH;

    m_videoStream.enc->time_base = m_videoStream.st->time_base = AVRational{1,25};
    m_videoStream.enc->gop_size = 12;
    m_videoStream.enc->pix_fmt = AV_PIX_FMT_YUV420P;
     
    AVDictionary *param = NULL;
    av_dict_set(&param, "preset", "slow", 0);
    av_dict_set(&param, "tune", "zerolatency", 0);
    avcodec_open2(m_videoStream.enc,videoCodec,&param);
    av_dict_free(&param);
    m_videoStream.frame = av_frame_alloc();
    m_videoStream.frame->format = AV_PIX_FMT_YUV420P;
    m_videoStream.frame->width = m_frameW;
    m_videoStream.frame->height = m_frameH;
    av_frame_get_buffer(m_videoStream.frame, 0);
    avcodec_parameters_from_context(m_videoStream.st->codecpar, m_videoStream.enc);


    // const AVCodec* audioCodec = avcodec_find_encoder(avoutputFormat->audio_codec);
    // audioStream.enc = avcodec_alloc_context3(audioCodec);
    // audioStream.st = avformat_new_stream(avformatContext, NULL);
    // audioStream.pkt = av_packet_alloc();
    // audioStream.enc->sample_fmt = AV_SAMPLE_FMT_FLTP;
    // audioStream.enc->bit_rate = 64000;
    // audioStream.enc->sample_rate = 44100;

    av_dump_format(avformatContext, 0, "dump.output", 1);

    m_swsCtx = sws_getCachedContext(m_swsCtx, 
                                    m_frameW, 
                                    m_frameW, 
                                    AV_PIX_FMT_RGB24, 
                                    m_frameW, m_frameH, 
                                    AV_PIX_FMT_YUV420P, 
                                    SWS_BICUBIC, 
                                    NULL, 
                                    NULL, 
                                    NULL);
    
    return true;
}

bool VideoEncoder::encoderXimage(XImage* inputImage)
{

    int height = inputImage->height;
    int width = inputImage->width;
    
    std::vector<unsigned long> rgbs; 
    for(int i = 0; i < width; i++)
    {
        for(int j = 0; j < height; j++)
        {
            unsigned long rgb = XGetPixel(inputImage, i, j);
            rgbs.push_back(rgb);
        }
    }

    AVFrame *rgbFrame = av_frame_alloc();
    av_image_fill_arrays(rgbFrame->data,
                         rgbFrame->linesize, 
                         (unsigned char*)rgbs.data(), 
                         AV_PIX_FMT_RGB24, 
                         m_frameW,m_frameH, 
                         1);
    sws_scale(m_swsCtx, rgbFrame->data, rgbFrame->linesize, 0, m_frameH, 
              m_videoStream.frame->data, m_videoStream.frame->linesize);
    
}

