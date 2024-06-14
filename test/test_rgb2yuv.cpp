#include "../screencapture/ScreenCapture.h"
#include "libavutil/pixfmt.h"
#include <cstdio>
#include <cstdlib>
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include "libavutil/frame.h"
}

#include <X11/Xutil.h>
#include <vector>
int main(int argc,char* argv[])
{
    XImageScreenCapture capture;
    capture.init();
    XImage* image = capture.captureXImage();
    
    std::vector<unsigned char> rgbs; 
    for(int i = 0; i < image->height; i++)
    {
        for(int j = 0; j < image->width; j++)
        {
            unsigned long rgb = XGetPixel(image, j, i);
            rgbs.push_back((rgb & image->red_mask) >> 16);
            rgbs.push_back((rgb & image->green_mask) >> 8);
            rgbs.push_back((rgb & image->blue_mask));
        }
    }

    AVFrame *rgbFrame = av_frame_alloc();
    int ret = av_image_fill_arrays(rgbFrame->data,
                         rgbFrame->linesize, 
                         rgbs.data(), 
                         AV_PIX_FMT_RGB24, 
                         image->width,
                         image->height, 
                         1);
    printf("ret: %d\n",ret);
    AVFrame *yuvFrame = av_frame_alloc();
    yuvFrame->format = AV_PIX_FMT_YUV420P;
    yuvFrame->width = (image->width/2)*2;
	yuvFrame->height = (image->height/2)*2;
    av_frame_get_buffer(yuvFrame, 0);
    SwsContext *swsCtx = NULL;
    swsCtx = sws_getContext(image->width,
                        image->height, 
                        AV_PIX_FMT_RGB24,
                         yuvFrame->width,
                          yuvFrame->height, 
                          AV_PIX_FMT_YUV420P, 
                          SWS_BICUBIC,
                            NULL, NULL, NULL);
    printf(" before yuvframe linesize: %d\n",yuvFrame->linesize[0]);
    printf(" before  yuvframe linesize: %d\n",yuvFrame->linesize[1]);
    ret = sws_scale(swsCtx, 
    rgbFrame->data, 
    rgbFrame->linesize, 
    0,
     image->height, 
              yuvFrame->data, 
              yuvFrame->linesize);
    printf("second ret: %d\n",ret);
    char path[1024] = {0};
    sprintf(path, "/home/sos/test_%d_%d.yuv", yuvFrame->width,yuvFrame->height);
    FILE *fp = fopen(path, "wb+");
    if(!fp)
    {
        perror("open:");
        printf("open failed\n");
    }

    // int y_size = yuvFrame->width * yuvFrame->height;
    // printf("ysize: %d\n",y_size);
    // printf("yuvframe linesize: %d\n",yuvFrame->linesize[0]);
    //  printf("yuvframe linesize: %d\n",yuvFrame->linesize[1]);
    // fwrite(yuvFrame->data[0], 1, y_size, fp);
    // fwrite(yuvFrame->data[1], 1, y_size/4, fp);
    // fwrite(yuvFrame->data[2], 1, y_size/4, fp);
    


    for(int i=0;i<yuvFrame->height;i++)
    {
        fwrite(yuvFrame->data[0],1,yuvFrame->width,fp);    //Y 
        yuvFrame->data[0]+=yuvFrame->linesize[0];
    }

    for(int i=0;i<yuvFrame->height/2;i++){
        fwrite(yuvFrame->data[1],1,yuvFrame->width/2,fp);   //U
        yuvFrame->data[1]+=yuvFrame->linesize[1];
    }			

    for(int i=0;i<yuvFrame->height/2;i++){
        fwrite(yuvFrame->data[2],1,yuvFrame->width/2,fp);   //V
        yuvFrame->data[2]+=yuvFrame->linesize[2];
    }

    fclose(fp);
   

    // AVFrame *rgbFrame_2 = av_frame_alloc();
    // rgbFrame_2->format = AV_PIX_FMT_RGB24;
    // rgbFrame_2->width = yuvFrame->width;
	// rgbFrame_2->height = yuvFrame->height; 
    // av_frame_get_buffer(rgbFrame_2, 0);

    // SwsContext *swsCtx_2 = sws_getContext(yuvFrame->width,
    //                     yuvFrame->height, 
    //                     AV_PIX_FMT_YUV420P,
    //                     rgbFrame_2->width,
    //                     rgbFrame_2->height, 
    //                     AV_PIX_FMT_RGB24, 
    //                     SWS_BICUBIC,
    //                     NULL, NULL, NULL);
    
    // ret = sws_scale(swsCtx_2, 
    //   yuvFrame->data, 
    // yuvFrame->linesize, 
    //  0,
    //  yuvFrame->height, 
    //           rgbFrame_2->data, 
    //           rgbFrame_2->linesize);
    // printf("%d-%d\n", rgbFrame_2->width, rgbFrame_2->height);
    // FILE *fp_2 = fopen("/home/sos/test_2.rgb", "wb+");
    // if(!fp_2)
    // {
    //     perror("open:");
    //     printf("open failed\n");
    // }

    // fwrite(rgbFrame_2->data[0], 1, yuvFrame->width * yuvFrame->height*3, fp_2);
    // fclose(fp_2);
    
    return 0;
}