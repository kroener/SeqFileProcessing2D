/* 
 * Copyright (C) 2020 Christian Kröner, University of Warwick 
 *
 * This file is part of SeqFileProcessing2D.
 *
 * SeqFileProcessing2D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SeqFileProcessing2D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with SeqFileProcessing2D.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "MyVideoReader.hpp"
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include "Image.hpp"
//libav:
extern "C" {
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libavresample/avresample.h>
#include <libswscale/swscale.h>
//decoding
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}
#undef main

using namespace std;

MyVideoReader::MyVideoReader()
{
    isReady=0;
    eps_zero = 0.000025;
    first_frame_number = -1;
    av_register_all();
    video_frame_count=0;
    video_stream_idx=0;
    picture_pts = AV_NOPTS_VALUE;
    frame_number=0;
    refcount=0;
    fmt_ctx = NULL;
    video_dec_ctx = NULL;
    video_stream = NULL;
    audio_stream = NULL;

}

MyVideoReader::~MyVideoReader()
{
    if(video_dec_ctx)
        avcodec_free_context(&video_dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_frame_free(&frame);
    av_free(video_dst_data[0]);
}

int MyVideoReader::getNext(Image<unsigned char>& img)
{
    if(av_read_frame(fmt_ctx, &pkt) >= 0) {
        AVPacket orig_pkt = pkt;
        do {
            ret = decode_packet(&got_frame, 0);
            if (ret < 0)
                break;
            pkt.data += ret;
            pkt.size -= ret;
        } while (pkt.size > 0);
        if(!img.hasBuffer())
        {
           img.create(width, height);
        }
        else if(!(img.getX()==width && img.getY()==height))
        {
           img.create(width, height);
        }
        picture_pts=orig_pkt.pts;        
        memcpy(img.buffer,video_dst_data[0],width*height);
        frame_number = dts_to_frame_number(picture_pts) - first_frame_number;
        double timestamp=frame_number/get_fps();
        img.setT(timestamp);
        av_packet_unref(&orig_pkt);
        return 0;
    }
    return 1; 
}

int MyVideoReader::open_codec_context(int *stream_idx,
                              AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), src_filename);
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            return ret;
        }
        av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
        if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }
    return 0;
}

int MyVideoReader::decode_packet(int *got_frame, int cached)
{
    ret = 0;
    int decoded = pkt.size;
    *got_frame = 0;
    if (pkt.stream_index == video_stream_idx) {
        ret = avcodec_decode_video2(video_dec_ctx, frame, got_frame, &pkt);
        if (ret < 0) {
            fprintf(stderr, "Error decoding video frame (%d)\n", ret);
            return ret;
        }

        if (*got_frame) {

            if (frame->width != width || frame->height != height ||
                frame->format != pix_fmt) {
                fprintf(stderr, "Error: Width, height and pixel format have to be "
                        "constant in a rawvideo file, but the width, height or "
                        "pixel format of the input video changed:\n"
                        "old: width = %d, height = %d, format = %s\n"
                        "new: width = %d, height = %d, format = %s\n",
                        width, height, av_get_pix_fmt_name(pix_fmt),
                        frame->width, frame->height,
                        av_get_pix_fmt_name((AVPixelFormat)frame->format));
                return -1;
            }
            if(cached==0)
               av_image_copy(video_dst_data, video_dst_linesize,
                          (const uint8_t **)(frame->data), frame->linesize,
                          pix_fmt, width, height);
            
        }
        else{
            cout << "Not got Frame..."<<endl;        
        }
    } 

    if (*got_frame && refcount)
        av_frame_unref(frame);
    return decoded;
}

int64_t MyVideoReader::get_total_frames()
{
    int64_t nbf = video_stream->nb_frames;

    if (nbf == 0)
    {
        nbf = (int64_t)floor(get_duration_sec() * get_fps() + 0.5);
    }
    return nbf;
}

double MyVideoReader::get_duration_sec()
{
    double sec = (double)fmt_ctx->duration / (double)AV_TIME_BASE;

    if (sec < eps_zero)
    {
        sec = (double)video_stream->duration * r2d(video_stream->time_base);
    }

    return sec;
}

double MyVideoReader::get_fps()
{
 double fps=video_stream->r_frame_rate.num / (double)video_stream->r_frame_rate.den;
 if (fps < eps_zero)
    {
        fps = 1.0 / r2d(video_stream->codec->time_base);
    }
  return fps;
}

void MyVideoReader::seek(int64_t _frame_number)
{
    _frame_number = std::min(_frame_number, get_total_frames());
    int delta = 16;
    for(;;)
    {
        int64_t _frame_number_temp = std::max(_frame_number-delta, (int64_t)0);
        double sec = (double)_frame_number_temp / get_fps();
        int64_t time_stamp = video_stream->start_time;
        double  time_base  = r2d(video_stream->time_base);
        time_stamp += (int64_t)(sec / time_base + 0.5);
        if (get_total_frames() > 1) av_seek_frame(fmt_ctx, video_stream_idx, time_stamp, AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(video_dec_ctx);
        if( _frame_number > 0 )
        {
            grabFrame();

            if( _frame_number > 1 )
            {
                frame_number = dts_to_frame_number(picture_pts) - first_frame_number;
                printf("_frame_number = %d, frame_number = %d, delta = %d\n",
                       (int)_frame_number, (int)frame_number, delta);

                if( frame_number < 0 || frame_number > _frame_number-1 )
                {
                    if( _frame_number_temp == 0 || delta >= INT_MAX/4 )
                        break;
                    delta = delta < 16 ? delta*2 : delta*3/2;
                    continue;
                }
                while( frame_number < _frame_number-1 )
                {
                    if(!grabFrame())
                        break;
                }
                frame_number++;
                break;
            }
            else
            {
                frame_number = 1;
                break;
            }
        }
        else
        {
            frame_number = 0;
            break;
        }
    }
}

int64_t MyVideoReader::dts_to_frame_number(int64_t dts)
{
    double sec = dts_to_sec(dts);
    return (int64_t)(get_fps() * sec + 0.5);
}

double MyVideoReader::dts_to_sec(int64_t dts)
{
    if(video_stream->start_time == AV_NOPTS_VALUE)
    return (double)(dts) *
        r2d(video_stream->time_base);
    else
    return (double)(dts - video_stream->start_time) *
        r2d(video_stream->time_base);
    
}

double MyVideoReader::r2d(AVRational r)
{
    return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
}

bool MyVideoReader::grabFrame()
{
    bool valid=false;
    picture_pts = AV_NOPTS_VALUE;

    if(av_read_frame(fmt_ctx, &pkt) >= 0) {
        AVPacket orig_pkt = pkt;
        do {
            ret = decode_packet(&got_frame, 1);
            if (ret < 0)
                break;
            pkt.data += ret;
            pkt.size -= ret;
            } while (pkt.size > 0);
        if( picture_pts == AV_NOPTS_VALUE )
            picture_pts = pkt.pts != AV_NOPTS_VALUE && pkt.pts != 0 ? pkt.pts : pkt.dts;
        frame_number++;
        if( picture_pts != AV_NOPTS_VALUE )
            valid = true;
        else
            valid = false;    
        av_packet_unref(&orig_pkt);
    }
    if( valid && first_frame_number < 0 )
    {
        first_frame_number = dts_to_frame_number(picture_pts);
    }
    return valid;
}

int MyVideoReader::getW()
{
    return width;
}

int MyVideoReader::getH()
{
    return height;
}

int MyVideoReader::isOpened()
{
    return isReady;
}

int MyVideoReader::open(string filen)
{
    src_filename=filen.c_str();
    int ret;
    isReady=1;
    if (avformat_open_input(&fmt_ctx, filen.c_str(), NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        isReady=0;
    }
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        isReady=0;
    }
    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        video_stream = fmt_ctx->streams[video_stream_idx];
        width = video_dec_ctx->width;
        height = video_dec_ctx->height;
        pix_fmt = video_dec_ctx->pix_fmt;
        ret = av_image_alloc(video_dst_data, video_dst_linesize,
                             width, height, pix_fmt, 1);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate raw video buffer\n");
            isReady=0;
        }
        video_dst_bufsize = ret;
    }
    av_dump_format(fmt_ctx, 0, src_filename, 0);
    if (!audio_stream && !video_stream) {
        fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
        isReady=0;
    }
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        isReady=0;
    }
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
    grabFrame();
    grabFrame();
    return isReady;
}

int MyVideoReader::release()
{
    if(video_dec_ctx)
        avcodec_free_context(&video_dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_frame_free(&frame);
    av_free(video_dst_data[0]);
    isReady=0;
    return 0;
}
