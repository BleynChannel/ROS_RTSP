#include "ffmpegDecode.h"
 
ffmpegDecode :: ~ffmpegDecode()
{
    pCvMat->release();
    //�ͷű��ζ�ȡ��֡�ڴ�
    av_free_packet(packet);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}
 
ffmpegDecode :: ffmpegDecode(char * file)
{
    pAvFrame = NULL/**pFrameRGB = NULL*/;
    pFormatCtx  = NULL;
    pCodecCtx   = NULL;
    pCodec      = NULL;
 
    pCvMat = new cv::Mat();
    i=0;
    videoindex=0;
 
    ret = 0;
    got_picture = 0;
    img_convert_ctx = NULL;
    y_size = 0;
    packet = NULL;

   
 
    if (NULL == file)
    {
        filepath =  "opencv.h264";
    }
    else
    {
        filepath = file;
    }

    optionsDict = NULL;
    av_dict_set(&optionsDict, "rtsp_transport", "tcp", 0);                //����tcp����	,,��������������Щrtsp���ͻῨ��
    av_dict_set(&optionsDict, "stimeout", "2000000", 0);                  //���û������stimeout
 
    init();
    openDecode();
   
 
    return;
}
 
void ffmpegDecode :: init()
{
    
    av_register_all();
    avformat_network_init(); 


    if(avformat_open_input(&pFormatCtx,filepath,0,&optionsDict)!=0)
    {
        printf("�޷����ļ�\n");
        return;
    }
 
    //�����ļ�������Ϣ,avformat_open_input����ֻ�Ǽ�����ļ���ͷ��������Ҫ������ļ��е�������Ϣ
    //if(av_find_stream_info(pFormatCtx)<0)
    //{
    //    printf("Couldn't find stream information.\n");
    //    return;
    //}

    if(avformat_find_stream_info(pFormatCtx,NULL)<0)
    {
       printf("Couldn't find stream information.\n");
        return;
    }



    return;
}
 
void ffmpegDecode :: openDecode()
{
    //�����ļ��ĸ��������ҵ���һ����Ƶ��������¼�����ı�����Ϣ
    videoindex = -1;
    for(i=0; i<pFormatCtx->nb_streams; i++) 
    {
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoindex=i;
            break;
        }
    }
    if(videoindex==-1)
    {
        printf("Didn't find a video stream.\n");
        return;
    }
    pCodecCtx=pFormatCtx->streams[videoindex]->codec;
 
    //�ڿ��������֧�ָø�ʽ�Ľ�����
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL)
    {
        printf("Codec not found.\n");
        return;
    }
 
    //�򿪽�����
    if(avcodec_open2(pCodecCtx, pCodec,NULL) < 0)
    {
        printf("Could not open codec.\n");
        return;
    }


    //����һ��ָ֡�룬ָ�������ԭʼ֡
    pAvFrame=av_frame_alloc();
    y_size = pCodecCtx->width * pCodecCtx->height;
    //����֡�ڴ�
    packet=(AVPacket *)av_malloc(sizeof(AVPacket));
    av_new_packet(packet, y_size);
 
    //���һ����Ϣ-----------------------------
    printf("stream info-----------------------------------------\n");
    av_dump_format(pFormatCtx,0,filepath,0);
    //av_dump_formatֻ�Ǹ����Ժ���������ļ���������Ƶ���Ļ�����Ϣ�ˣ�֡�ʡ��ֱ��ʡ���Ƶ�����ȵ�
    printf("-------------------------------------------------\n");
}  



AVPixelFormat ffmpegDecode :: ConvertDeprecatedFormat(enum AVPixelFormat format)
{
    switch (format)
    {
    case AV_PIX_FMT_YUVJ420P:
        return AV_PIX_FMT_YUV420P;
        break;
    case AV_PIX_FMT_YUVJ422P:
        return AV_PIX_FMT_YUV422P;
        break;
    case AV_PIX_FMT_YUVJ444P:
        return AV_PIX_FMT_YUV444P;
        break;
    case AV_PIX_FMT_YUVJ440P:
        return AV_PIX_FMT_YUV440P;
        break;
    default:
        return format;
        break;
    }
}

 

 
int ffmpegDecode :: readOneFrame()
{
    int result = 0;
    result = av_read_frame(pFormatCtx, packet);
    return result;
}
 
cv::Mat ffmpegDecode :: getDecodedFrame()
{
    if(packet->stream_index==videoindex)
    {
        //����һ��֡
        ret = avcodec_decode_video2(pCodecCtx, pAvFrame, &got_picture, packet);
        if(ret < 0)
        {
            printf("�������\n");
            return cv::Mat();
        }
        if(got_picture)
        {
            //���ݱ�����Ϣ������Ⱦ��ʽ
            if(img_convert_ctx == NULL){
                img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                    ConvertDeprecatedFormat(pCodecCtx->pix_fmt), pCodecCtx->width, pCodecCtx->height,
                    AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL); 
            }    
            //----------------------opencv
            if (pCvMat->empty())
            {
                pCvMat->create(cv::Size(pCodecCtx->width, pCodecCtx->height),CV_8UC3);
            }
 
            if(img_convert_ctx != NULL)  
            {  
                get(pCodecCtx, img_convert_ctx, pAvFrame);
            }
        }
    }
    av_free_packet(packet);
 
    return *pCvMat;
}


 
void ffmpegDecode :: get(AVCodecContext    * pCodecCtx, SwsContext * img_convert_ctx, AVFrame * pFrame)
{
    if (pCvMat->empty())
    {
        pCvMat->create(cv::Size(pCodecCtx->width, pCodecCtx->height),CV_8UC3);
    }
 
    AVFrame    *pFrameRGB = NULL;
    uint8_t  *out_bufferRGB = NULL;
    pFrameRGB = av_frame_alloc();
 
    //��pFrameRGB֡���Ϸ�����ڴ�;
    int size = avpicture_get_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);
    out_bufferRGB = new uint8_t[size];
    avpicture_fill((AVPicture *)pFrameRGB, out_bufferRGB, AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);
 
    //YUV to RGB
    sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
 
    memcpy(pCvMat->data,out_bufferRGB,size);
 
    delete[] out_bufferRGB;
    av_free(pFrameRGB);
}

/*
int main()
{
	ffmpegDecode rtsp_read("rtsp://admin:wanji123@192.168.2.101:554/cam/realmonitor?channel=1&subtype=0");
       //ffmpegDecode rtsp_read("rtmp://58.200.131.2:1935/livetv/hunantv");
       while(1){ 
        rtsp_read.readOneFrame();
	cv::Mat Img = rtsp_read.getDecodedFrame();
          if(!Img.empty()){
        imshow("window", Img);
    }

	//imshow("2333",Img);
	cvWaitKey(1);
         
        }
	
	return 0;
}*/
