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
#ifndef MyVideoReader_hpp
#define MyVideoReader_hpp
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

template < typename T > class Image;

/**
  * \brief Class to write h264 bit stream to file
  */
class MyVideoReader
{
public:
    MyVideoReader();
    ~MyVideoReader();
    int getNext(Image<unsigned char> &img);
    int seekAndGet(Image<unsigned char> *img);
    int seekAndGet(Image<unsigned char> &img);
    int64_t get_total_frames();
    double get_duration_sec();
    double get_fps();
    void seek(int64_t _frame_number);
    int64_t dts_to_frame_number(int64_t dts);
    double dts_to_sec(int64_t dts);
    double r2d(AVRational r);
    bool grabFrame();
    int getW();
    int getH();
    int open(string filen);
    int release();
    int isOpened();

private:
    int open_codec_context(int *stream_idx,
        AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    int decode_packet(int *got_frame, int cached);
    AVFormatContext *fmt_ctx;
    AVCodecContext *video_dec_ctx, *audio_dec_ctx;
    int width, height;
    enum AVPixelFormat pix_fmt;
    AVStream *video_stream, *audio_stream;
    const char *src_filename;
    const char *video_dst_filename;
    const char *audio_dst_filename;
    FILE *video_dst_file;
    FILE *audio_dst_file;
    uint8_t *video_dst_data[4];
    int video_dst_linesize[4];
    int video_dst_bufsize;
    int video_stream_idx, audio_stream_idx;
    AVFrame *frame;
    AVPacket pkt;
    int video_frame_count;
    int audio_frame_count;
    int refcount;
    int64_t first_frame_number,frame_number,picture_pts;
    int ret, got_frame;
    double eps_zero;
    int isReady;
};
#endif
