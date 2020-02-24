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
#ifndef MyVideoWriter_hpp
#define MyVideoWriter_hpp
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
#include "Metadata.hpp"
#if CV_MAJOR_VERSION < 3
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <ctype.h>
#else
#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <ctype.h>
#endif
extern "C" {
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libavresample/avresample.h>
#include <libswscale/swscale.h>
}
#undef main

using namespace std;

class MyVideoWriter
{
public:
    struct Settings{
        int bit_rate=17000000;
        int w;
        int h;
        int timebasenum=1;
        int timebaseden=25;
        int gop_size=60;
        AVCodecID video_codec=AV_CODEC_ID_H265;
        string codec_by_name="h264_nvenc";
        int by_name=1;
        string preset="slow";
        int crf=0;
        bool useCrf=false;
    };
    MyVideoWriter(string filen,Settings is);
    ~MyVideoWriter();
    int add(Image<unsigned char>* img);
    int add(cv::Mat& img);
    int isOpened();
    int close();
    int metaData(unsigned long i,unsigned int n);
    int metaDataToCurrentFrame(unsigned int n);
    int metaData(unsigned long i,unsigned int n, Metadata::timeValue tv);
    int metaDataToCurrentFrame(unsigned int n, Metadata::timeValue tv);
    int hasMeta();

private:
    string metaName();
    int openMeta();
    Settings settings;
    AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);
    int getLateFrames();
    AVStream *st;
    AVCodecContext *enc;
    int64_t next_pts;
    AVFrame *frame;
    AVFrame *tmp_frame;
    float t, tincr, tincr2;
    struct SwsContext *sws_ctx;
    AVCodec *codec;
    const char *filename;
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    int haveTmp;
    int isReady;
    Metadata meta;
    int have_meta;
    string vidfilename;
    unsigned int addedImages;
};
#endif
