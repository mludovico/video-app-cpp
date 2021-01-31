extern "C"{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <inttypes.h>
}

#include "helpers.hpp"

bool load_frame(const char* filename, int* width_out, int* height_out, unsigned char** data_out) {
    
    // Open file using libavformat
    AVFormatContext* av_format_context = avformat_alloc_context();
    if (!av_format_context) {
        exitWithError(false, "Could not create avformat context.\n");
    }

    int response = avformat_open_input(&av_format_context, filename, NULL, NULL);
    if(response != 0) {
        printf("Could not open video file\n");
        exitWithError(false, av_err2str(response));
    }

    //Find video stream index
    int video_stream_index = -1;
    AVCodecParameters* av_codec_params;
    AVCodec* av_codec;
    
    for (int i = 0; i < av_format_context->nb_streams; i++) {
        auto stream = av_format_context->streams[i];
        av_codec_params = av_format_context->streams[i]->codecpar;
        av_codec = avcodec_find_decoder(av_codec_params->codec_id);

        if (!av_codec) {
            continue;
        }
        if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index < 0) {
        exitWithError(false, "Could not find a valid stream inside file\n");
    }

    //Setup a codec context for the decoder

    AVCodecContext* av_codec_context = avcodec_alloc_context3(av_codec);
    if (!av_codec_context) {
        exitWithError(false, "Could not create avcodec context\n");
    }
    if (avcodec_parameters_to_context(av_codec_context, av_codec_params) < 0) {
        exitWithError(false, "Could not initialize avcodeccontext");
    }
    if (avcodec_open2(av_codec_context, av_codec, NULL) < 0) {
        exitWithError(false, "Could not open codec.");
    }

    // Allocate frame and packet
    AVFrame* av_frame = av_frame_alloc();
    if(!av_frame) {
        exitWithError(false, "Could not allocate frame");
    }
    AVPacket* av_packet = av_packet_alloc();
    if(!av_packet) {
        exitWithError(false, "Could not allocate packet");
    }

    while (av_read_frame(av_format_context, av_packet) >= 0) {
        if(av_packet->stream_index != video_stream_index) {
            continue;
        }
        response = avcodec_send_packet(av_codec_context, av_packet);
        if (response < 0) {
            printf("Failed to decode packet\n");
            exitWithError(false, av_err2str(response));
        }
        response = avcodec_receive_frame(av_codec_context, av_frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            continue;
        }else if (response < 0) {
            printf("Failed to decode packet\n");
            exitWithError(false, av_err2str(response));
        }
        av_packet_unref(av_packet);
        break;
    }

    SwsContext* sws_scaler_context = sws_getContext(
        av_frame->width,
        av_frame->height,
        av_codec_context->pix_fmt,
        av_frame->width,
        av_frame->height,
        AV_PIX_FMT_RGB0,
        SWS_BILINEAR,
        NULL, NULL, NULL
    );

    if (!sws_scaler_context) {
        exitWithError(false, "Could not initialize sw scaler\n");
    }
    uint8_t* data = new uint8_t[av_frame->width * av_frame->height * 4];
    uint8_t* dest[4] = { data, NULL, NULL, NULL };
    int dest_linesize[4] = { av_frame->width * 4, 0, 0, 0 };
    sws_scale(sws_scaler_context, av_frame->data, av_frame->linesize, 0, av_frame->height, dest, dest_linesize);
    sws_freeContext(sws_scaler_context);

    *width_out = av_frame->width;
    *height_out = av_frame->height;
    *data_out = data;

    avformat_close_input(&av_format_context);
    avformat_free_context(av_format_context);
    avcodec_free_context(&av_codec_context);
    av_packet_free(&av_packet);
    avcodec_free_context(&av_codec_context);

    return true;
}