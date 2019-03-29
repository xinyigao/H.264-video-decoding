/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <stdio.h>  
  
#define __STDC_CONSTANT_MACROS  
  
#ifdef __cplusplus  
extern "C"  
{  
#endif  
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#ifdef __cplusplus  
};  
#endif 
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 100
#define AVCODEC_MAX_VIDEO_FRAME_SIZE 4096
/*******************************************************************************
 * Prototypes
 *****************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static void h264_video_decode(const char *filename, const char *outfilename)
{
	
printf("Decode file '%s' to '%s'\n", filename, outfilename);

	FILE *file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Could not open '%s'\n", filename);
		exit(1);
	}
	
	FILE *outfile = fopen(outfilename, "wb");
	if (!outfile) {
		fprintf(stderr, "Could not open '%s'\n", outfilename);
		exit(1);
	}
    int error;
    int  len;
    AVCodec *pInCodec=NULL;
    AVFormatContext *pInFmtCtx=NULL;//文件格式
    AVCodecContext *pInCodecCtx=NULL;//编码格式

	av_register_all();    
    avcodec_register_all(); 
    pInFmtCtx = avformat_alloc_context();//初始化一个AVFormatContext 
	error =avformat_open_input(&pInFmtCtx, filename, NULL, NULL);
	if(error !=0)
	{
          printf("Couldn't open file: error :%d\n",error); 
	}
	error = avformat_find_stream_info(pInFmtCtx,NULL);
	if( error <0)
	{
		printf("Couldn't find stream information error :%d\n",error);
	} 
      unsigned int j;
      int    videoStream = -1;

    for(j=0; j<pInFmtCtx->nb_streams; j++)
    {//找到视频对应的stream
       if(pInFmtCtx->streams[j]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
       {
           videoStream = j;
       }
    }      
      
    printf("video stream num: %d\n",videoStream);

    pInCodecCtx = pInFmtCtx->streams[videoStream]->codec;//音频的编码上下文
    pInCodec = avcodec_find_decoder(pInCodecCtx->codec_id);
    if(avcodec_open2(pInCodecCtx, pInCodec,NULL)<0)
    {
        printf("error avcodec_open failed.\n");
   
    }
 
       AVPacket packet;
       uint8_t *pktdata;
       int pkt;
       int out_size = AVCODEC_MAX_VIDEO_FRAME_SIZE*100;
       AVFrame *frame = av_frame_alloc();
       while(av_read_frame(pInFmtCtx, &packet)>=0)
       {
           if(packet.stream_index==videoStream)
           {
           
               pktdata = packet.data;
                pkt = packet.size;
                while(pkt>0)
                {
                    out_size = AVCODEC_MAX_VIDEO_FRAME_SIZE*100;
                    len =avcodec_decode_video2(pInCodecCtx,frame,&out_size,&packet);
                    if (len<0)
                     {
                        printf("Error while decoding.\n");
                        break;
                     }
                  

                    if(out_size>0)
                    {
	                 int i;	
                     int y_size = frame->width * frame->height;
                     //y 亮度信息写完了
                     fwrite(frame->data[0], 1, y_size, outfile);
                     fwrite(frame->data[1], 1, y_size / 4, outfile);
                     fwrite(frame->data[2], 1, y_size / 4, outfile);

                    }

                    pkt -= len;

                    pktdata += len;

                }
           }

           //av_free_packet(&packet);
           av_packet_unref(&packet);

       }

    fclose(file);
    fclose(outfile);

    if (pInCodecCtx!=NULL)
    {
        avcodec_close(pInCodecCtx);
    }
    av_frame_free(&frame); 
      
    avformat_close_input(&pInFmtCtx);
    avformat_free_context(pInFmtCtx);

   printf("Done\n");
}



/*!
 * @brief Main function
 */


int main(void)
{
	char *filepath_in="test.h264";
	char *filepath_out="test-out.yuv";
	
	h264_video_decode(filepath_in, filepath_out);
	return 0;		
}


