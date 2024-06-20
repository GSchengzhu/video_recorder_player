#include "VideoEncoder.h"
#include "../common/ffmpeg_helper.h"
#include "../common/utils.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include "libavcodec/codec.h"
#include "libavcodec/codec_id.h"
#include "libavcodec/packet.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/dict.h"
#include "libavutil/error.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libavutil/mem.h"
#include "libavutil/pixfmt.h"
#include "libavutil/rational.h"
#include "libavutil/samplefmt.h"
#include "libswscale/swscale.h"
}

#define AIMW 640
#define AIMH 360

VideoEncoder::~VideoEncoder()
{
    if(m_videoStream.enc)
    {
        avcodec_close(m_videoStream.enc);
    }
    if(m_videoStream.pkt)
    {
        av_packet_free(&m_videoStream.pkt);
    }

    if(m_avformatContext)
    {
        avformat_free_context(m_avformatContext);
    }

}

bool VideoEncoder::initEncode()
{
    if(m_path.empty())
    {
        return false;
    }
    const AVOutputFormat *avoutputFormat = NULL;
    avoutputFormat = av_guess_format(NULL, m_path.c_str(), NULL);

    m_avformatContext = avformat_alloc_context();
    m_avformatContext->oformat = avoutputFormat;

    //获取编码器
    const AVCodec* videoCodec = avcodec_find_encoder(avoutputFormat->video_codec);
    
    //根据编码器生成编码器上下文  并设置通用初始参数
    m_videoStream.enc = avcodec_alloc_context3(videoCodec);
    m_videoStream.enc->codec_id = avoutputFormat->video_codec;
    m_videoStream.enc->bit_rate = 4000000;
    m_videoStream.enc->width = AIMW;
    m_videoStream.enc->height = AIMH;

    m_videoStream.enc->time_base = AVRational{1,25};
    m_videoStream.enc->gop_size = 12;
    m_videoStream.enc->pix_fmt = AV_PIX_FMT_YUV420P;

    m_videoStream.pkt = av_packet_alloc();
     
    //设置额外参数 用于适配其他编码器
    AVDictionary *param = NULL;
    av_dict_set(&param, "preset", "slow", 0);
    av_dict_set(&param, "tune", "zerolatency", 0);
    
    //打开编码器  参数依次为 编码器上下文(初始化参数) 、编码器本身、额外参数
    avcodec_open2(m_videoStream.enc,videoCodec,&param);
    av_dict_free(&param);

    //新建 stream 使用编码器上下文初始化stream  应该只有解码时才需要  编码时不需要 
    m_videoStream.st = avformat_new_stream(m_avformatContext,NULL);
    avcodec_parameters_from_context(m_videoStream.st->codecpar, m_videoStream.enc);
    
    //将avformatcontext与输出文件绑定
    avio_open(&m_avformatContext->pb, m_path.c_str(), AVIO_FLAG_WRITE);
    //写入头
    avformat_write_header(m_avformatContext,NULL);

    //将输出错误信息输出到dump.output文件中
    av_dump_format(m_avformatContext, 0, "dump.output", 1);
    
    //用于后续接收编码器中的数据
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, AIMW, AIMH, 1);
    m_videoStream.pkt = (AVPacket*)av_malloc(buffer_size);
    
    return true;
}

bool VideoEncoder::encoderXimage(XImage* inputImage)
{
    int width = inputImage->width;
    int height = inputImage->height;

    if(m_imageBeforeW != width || m_imageBeforeH != height)
    {   
        //获取图片转换器上下文 用于将ximage中的rgb文件转换为
        m_swsCtx = sws_getCachedContext(m_swsCtx, width, height, 
                             AV_PIX_FMT_RGB24, AIMW, 
                             AIMH,
                            AV_PIX_FMT_YUV420P, 
                            SWS_BICUBIC, NULL, NULL, NULL);
        m_imageBeforeW = width;
        m_imageBeforeH = height;
    }
    
    std::vector<unsigned char> rgbs;
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            unsigned long rgb = XGetPixel(inputImage, x, y);
            rgbs.push_back((rgb & inputImage->red_mask) >> 16);
            rgbs.push_back((rgb & inputImage->green_mask) >> 8);
            rgbs.push_back((rgb & inputImage->blue_mask));
        }
    }

    AVFrame *rgbFrame = av_frame_alloc();
    av_image_fill_arrays(rgbFrame->data,
                         rgbFrame->linesize, 
                         rgbs.data(), 
                         AV_PIX_FMT_RGB24, 
                         width,height, 
                         1);
    
    AVFrame *yuvFrame = av_frame_alloc();
    yuvFrame->format = AV_PIX_FMT_YUV420P;
    yuvFrame->width = AIMW;
    yuvFrame->height = AIMH;
    av_frame_get_buffer(yuvFrame, 0);
    sws_scale(m_swsCtx, rgbFrame->data, rgbFrame->linesize, 0, height, 
              yuvFrame->data, yuvFrame->linesize);

    int result = avcodec_send_frame(m_videoStream.enc, yuvFrame);
    while (result >= 0) 
    {
        result = avcodec_receive_packet(m_videoStream.enc, m_videoStream.pkt);
        if(result == AVERROR(EAGAIN) || result == AVERROR_EOF)
        {
            break;
        }else if(result < 0)
        {
            fprintf(stderr, "encode error\n");
            exit(-1);
        }
         //编码不需要 直接指定index
        m_videoStream.pkt->stream_index = m_videoStream.st->index;
        // m_videoStream.pkt->stream_index = 1;

        //将编码的pkt中的输出写入avformatcontext中
        //avio_open已经将avformatcontext输出重定向到path中
        av_write_frame(m_avformatContext,m_videoStream.pkt);
        av_packet_unref(m_videoStream.pkt);
    }
    av_free(rgbFrame); 
    av_free(yuvFrame);   
}

bool VideoEncoder::fflushEncoder()
{
    if(m_videoStream.enc)
    {
        int result = avcodec_send_frame(m_videoStream.enc, NULL);
        while (result >= 0) 
        {
            result = avcodec_receive_packet(m_videoStream.enc, m_videoStream.pkt);
            if(result == AVERROR(EAGAIN) || result == AVERROR_EOF)
            {
                break;
            }else if(result < 0)
            {
                fprintf(stderr, "fflush encode error\n");
                exit(-1);
            }

            av_write_frame(m_avformatContext,m_videoStream.pkt);
            av_packet_unref(m_videoStream.pkt);
        }
    
        avio_close(m_avformatContext->pb);
    }

    return true;
}


