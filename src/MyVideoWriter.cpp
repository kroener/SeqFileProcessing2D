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
#include "MyVideoWriter.hpp"
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
using namespace cv;

MyVideoWriter::MyVideoWriter(string filen,Settings is)
{
 vidfilename=filen;
 have_meta=0;
 addedImages=0;
 settings=is;
 isReady=1;
 next_pts=0;
 haveTmp=0;
 av_register_all();
 fmt=av_guess_format(NULL, filen.c_str(), NULL);
 if(!fmt)
 {
  printf("Could not deduce output format from file extension: using MP4.\n");
  fmt=av_guess_format("mp4", NULL, NULL);
 }
 if(!fmt) 
 {
  fprintf(stderr, "Could not find suitable output format\n");
  isReady=0;
 }
 if(isReady)
 {
  oc=avformat_alloc_context();
  if(!oc)
  {
   fprintf(stderr, "Memory error\n");
   isReady=0;
  }
 }
 if(isReady)
 {
  oc->oformat=fmt;
  snprintf(oc->filename, sizeof(oc->filename), "%s", filen.c_str());
  if(fmt->video_codec!=AV_CODEC_ID_NONE)
  {
   if(settings.by_name)
   {
    codec=avcodec_find_encoder_by_name(settings.codec_by_name.c_str());
    if (!codec) 
    {
     fprintf(stderr, "%s codec not found\n", settings.codec_by_name);
     isReady=0; 
    }
   }
   else
   {
    codec=avcodec_find_encoder(settings.video_codec);
    if(!codec)
    {
     fprintf(stderr, "%d codec not found\n",settings.video_codec);
     isReady=0;
    }
   }    
   if(isReady)
   {
    st=avformat_new_stream(oc, NULL);
    if(!st)
    {
     fprintf(stderr, "Could not alloc stream\n");
     isReady=0;
    }
   }
   if(isReady)
   {
    enc=avcodec_alloc_context3(codec);
    if (!enc)
    {
     fprintf(stderr, "Could not alloc an encoding context\n");
     isReady=0;
    }
   }
   if(isReady)
   {
    if(!settings.useCrf)
     enc->bit_rate=settings.bit_rate;
    else
     enc->bit_rate=0;
    enc->width=settings.w;
    enc->height=settings.h;
    st->time_base=(AVRational){ settings.timebasenum, settings.timebaseden };
    enc->time_base=st->time_base;
    enc->gop_size=settings.gop_size;
    enc->pix_fmt=AV_PIX_FMT_YUV420P;
    enc->max_b_frames=0;
    enc->qmin=0;
    enc->qmax=51;
    if(oc->oformat->flags&AVFMT_GLOBALHEADER)
     enc->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;
    if(codec->id==AV_CODEC_ID_H264)
    {
     if(settings.useCrf)
     {
      if(settings.by_name)
      {
       av_opt_set(enc->priv_data, "preset", "hq", 0);
       av_opt_set(enc->priv_data, "profil", "high", 0);
       av_opt_set(enc->priv_data, "cq",  std::to_string(settings.crf).c_str(), 0);
      }
      else
      {
       av_opt_set(enc->priv_data, "crf",  std::to_string(settings.crf).c_str(), 0);
       av_opt_set(enc->priv_data, "preset", settings.preset.c_str(), 0);
      }
     }
    }
    else if(codec->id==AV_CODEC_ID_HEVC)
    {
     if(settings.useCrf)
     {
      if(settings.by_name)
      {
       av_opt_set(enc->priv_data, "preset", "hq", 0);
       av_opt_set(enc->priv_data, "cq",  std::to_string(settings.crf).c_str(), 0);
      }
      else
      {
       av_opt_set(enc->priv_data, "preset", settings.preset.c_str(), 0);
       av_opt_set(enc->priv_data, "tune", "zerolatency", 0);
       av_opt_set(enc->priv_data, "crf",  std::to_string(settings.crf).c_str(), 0);
      }
     }
    }
    else
    {
     if(settings.useCrf)
      fprintf(stderr, "Error: I don't know the codec id and can't set crf\n");    
    }
   }
  }
 }
 if(isReady)
 {
  if(avcodec_open2(enc, NULL, NULL) < 0) 
  {
   fprintf(stderr, "could not open codec\n");
   isReady=0;
  }
 }
 if(isReady)
 {
  frame=alloc_picture(enc->pix_fmt, enc->width, enc->height);
  if(!frame)
  {
   fprintf(stderr, "Could not allocate picture\n");
   isReady=0;
  }
 }
 if(isReady)
 {
  tmp_frame=NULL;
  if(enc->pix_fmt!=AV_PIX_FMT_YUV420P)
  {
   tmp_frame=alloc_picture(AV_PIX_FMT_YUV420P, enc->width, enc->height);
   if (!tmp_frame)
   {
    fprintf(stderr, "Could not allocate temporary picture\n");
    isReady=0;
   }
   haveTmp=1;
  }
 }
 if(isReady)
 {
  int ret=avcodec_parameters_from_context(st->codecpar, enc);
  if(ret < 0)
  {
   fprintf(stderr, "Could not copy the stream parameters\n");
   isReady=0;
  }
 }
 if(isReady)
 {
  av_dump_format(oc, 0, filen.c_str(), 1);
  if(!(fmt->flags & AVFMT_NOFILE))
  {
   if(avio_open(&oc->pb, filen.c_str(), AVIO_FLAG_WRITE) < 0)
   {
    fprintf(stderr, "Could not open '%s'\n", filen.c_str());
    isReady=0;
   }
  }
 }
 if(isReady)
 {
  avformat_write_header(oc, NULL);
 }
}

MyVideoWriter::~MyVideoWriter()
{
 if(isReady) getLateFrames();    
 if(isReady) av_write_trailer(oc);
 if(isReady) avcodec_free_context(&enc);
 if(isReady) av_frame_free(&frame);
 if(haveTmp)
 {
  av_frame_free(&tmp_frame);
  sws_freeContext(sws_ctx);
 }
 if(isReady && !(fmt->flags & AVFMT_NOFILE))
  avio_close(oc->pb);
 if(isReady) avformat_free_context(oc);
}

AVFrame *MyVideoWriter::alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
 AVFrame *picture;
 int ret;
 picture=av_frame_alloc();
 if(!picture)
  return NULL;
 picture->format=pix_fmt;
 picture->width =width;
 picture->height=height;
 ret=av_frame_get_buffer(picture, 32);
 if(ret < 0)
 {
  fprintf(stderr, "Could not allocate frame data.\n");
 }
 return picture;
}

int MyVideoWriter::add(Image<unsigned char>* img)
{
 int ret;
 if(isReady)
 {
  if(enc->pix_fmt !=AV_PIX_FMT_YUV420P)
  {
   if(!sws_ctx)
   {
    sws_ctx=sws_getContext(enc->width, enc->height, AV_PIX_FMT_YUV420P,
                             enc->width, enc->height, enc->pix_fmt,
                             SWS_BICUBIC, NULL, NULL, NULL);
    if(!sws_ctx)
     fprintf(stderr,"Cannot initialize the conversion context\n");
   }
   ret=av_frame_make_writable(tmp_frame);
   for(int y=0; y<enc->height; y++)
    for(int x=0; x<enc->width; x++)
     tmp_frame->data[0][y*tmp_frame->linesize[0]+x]=img->getValue(x,y);
   for(int y=0; y<enc->height/2; y++)
    for(int x=0; x<enc->width/2; x++)
   {
    tmp_frame->data[1][y*tmp_frame->linesize[1]+x]=128;
    tmp_frame->data[2][y*tmp_frame->linesize[2]+x]=128;
   }
   sws_scale(sws_ctx, tmp_frame->data, tmp_frame->linesize,
             0, enc->height, frame->data, frame->linesize);
  }
  else
  {
   ret=av_frame_make_writable(frame);
   for(int y=0; y<enc->height; y++)
    for(int x=0; x<enc->width; x++)
     frame->data[0][y*frame->linesize[0]+x]=img->getValue(x,y);
   for(int y=0; y<enc->height/2; y++)
   {
    for(int x=0; x<enc->width/2; x++)
    {
     frame->data[1][y*frame->linesize[1]+x]=128;
     frame->data[2][y*frame->linesize[2]+x]=128;
    }
   }
  }
  frame->pts=++next_pts;
  ret=avcodec_send_frame(enc, frame);
  if(ret<0)
  {
   fprintf(stderr, "Error submitting a frame for encoding\n");
  }
  while(ret>=0)
  {
   AVPacket pkt={0};
   av_init_packet(&pkt);
   ret=avcodec_receive_packet(enc,&pkt);
   if(ret<0 && ret!=AVERROR(EAGAIN) && ret!=AVERROR_EOF)
   {
    fprintf(stderr, "Error encoding a video frame\n");
   } 
   else if(ret>=0)
   {
    av_packet_rescale_ts(&pkt, enc->time_base, st->time_base);
    pkt.stream_index=st->index;
    ret=av_interleaved_write_frame(oc, &pkt);
    if(ret<0)
     fprintf(stderr, "Error while writing video frame\n");
   }
  }
  addedImages++;
  return ret>=0;
 }
 else
  return -1;
}

int MyVideoWriter::add(Mat& img)
{
 int channels=img.channels();
 int ret;
 if(isReady)
 {
  if(enc->pix_fmt!=AV_PIX_FMT_YUV420P)
  {
   if(!sws_ctx)
   {
    sws_ctx=sws_getContext(enc->width, enc->height, AV_PIX_FMT_YUV420P,
                           enc->width, enc->height, enc->pix_fmt,
                           SWS_BICUBIC, NULL, NULL, NULL);
    if(!sws_ctx)
     fprintf(stderr,"Cannot initialize the conversion context\n");
   }
   ret=av_frame_make_writable(tmp_frame);
   if(channels==1)
   {
    for(int y=0; y<enc->height; y++)
     for(int x=0; x<enc->width; x++)
      tmp_frame->data[0][y*tmp_frame->linesize[0]+x]=img.at<unsigned char>(y,x);
    for (int y=0; y<enc->height/2; y++)
     for (int x=0; x<enc->width/2; x++)
    {
     tmp_frame->data[1][y*tmp_frame->linesize[1]+x]=128;
     tmp_frame->data[2][y*tmp_frame->linesize[2]+x]=128;
    }
   }
   else
   {
    for(int y=0; y<enc->height; y++)
     for (int x=0; x<enc->width; x++)
    {
     Vec3b colour=img.at<Vec3b>(y,x);
     tmp_frame->data[0][y*tmp_frame->linesize[0]+x]=((66*colour.val[2]+129*colour.val[1]+25*colour.val[0])>>8)+16;
    }    
    for(int y=0; y<enc->height/2; y++)
     for(int x=0; x<enc->width/2; x++)
    {
     Vec3b colour=img.at<Vec3b>(2*y,2*x);
     tmp_frame->data[1][y*tmp_frame->linesize[1]+x]=((-38*colour.val[2]-74*colour.val[1]+112*colour.val[0])>>8)+128;
     tmp_frame->data[2][y*tmp_frame->linesize[2]+x]=((112*colour.val[2]-94*colour.val[1]-18*colour.val[0])>>8)+128;
    }
   }
   sws_scale(sws_ctx, tmp_frame->data, tmp_frame->linesize,
             0, enc->height, frame->data, frame->linesize);
  }
  else
  {
   ret=av_frame_make_writable(frame);
   if(channels==1)
   {
    for(int y=0; y<enc->height; y++)
     for(int x=0; x<enc->width; x++)
      frame->data[0][y*frame->linesize[0]+x]=img.at<unsigned char>(y,x);
    for(int y=0; y<enc->height/2; y++)
     for(int x=0; x<enc->width/2; x++)
    {
     frame->data[1][y*frame->linesize[1]+x]=128;
     frame->data[2][y*frame->linesize[2]+x]=128;
    }
   }
   else
   {
    for(int y=0; y<enc->height; y++)
     for (int x=0; x < enc->width; x++)
    {
     Vec3b colour=img.at<Vec3b>(y,x);
     frame->data[0][y*frame->linesize[0]+x]=((66*colour.val[2]+129*colour.val[1]+25*colour.val[0])>>8)+16;
    }
    for(int y=0; y<enc->height/2; y++)
     for (int x=0; x < enc->width / 2; x++)
    {
     Vec3b colour=img.at<Vec3b>(2*y,2*x);
     frame->data[1][y*frame->linesize[1]+x]=((-38*colour.val[2]-74*colour.val[1]+112*colour.val[0])>>8)+128;
     frame->data[2][y*frame->linesize[2]+x]=((112*colour.val[2]-94*colour.val[1]-18*colour.val[0])>>8)+128;
    }
   }
  }
  frame->pts=++next_pts;
  ret=avcodec_send_frame(enc, frame);
  if(ret<0)
   fprintf(stderr, "Error submitting a frame for encoding\n");
  while(ret>=0)
  {
   AVPacket pkt={0};
   av_init_packet(&pkt);
   ret=avcodec_receive_packet(enc, &pkt);
   if(ret<0 && ret!=AVERROR(EAGAIN) && ret!=AVERROR_EOF)
   {
    fprintf(stderr, "Error encoding a video frame\n");
   }
   else if(ret>=0)
   {
    av_packet_rescale_ts(&pkt, enc->time_base, st->time_base);
    pkt.stream_index=st->index;
    ret=av_interleaved_write_frame(oc, &pkt);
    if(ret<0)
     fprintf(stderr, "Error while writing video frame\n");
   }
  }
  addedImages++;
  return ret==AVERROR_EOF;
 }
 else
  return -1;
}

int MyVideoWriter::getLateFrames()
{
 int ret;
 frame=NULL;
 ret=avcodec_send_frame(enc, frame);
 if(ret<0)
 {
  fprintf(stderr, "Error submitting a frame for encoding\n");
 }
 while(ret>=0)
 {
  AVPacket pkt={0};
  av_init_packet(&pkt);
  ret=avcodec_receive_packet(enc,&pkt);
  if(ret==AVERROR_EOF) cerr<<"got all buffer...\n";
  if(ret<0 && ret!=AVERROR(EAGAIN) && ret!=AVERROR_EOF)
  {
   fprintf(stderr, "Error encoding a video frame\n");
  }
  else if(ret>=0)
  {
   av_packet_rescale_ts(&pkt, enc->time_base, st->time_base);
   pkt.stream_index=st->index;
   ret=av_interleaved_write_frame(oc, &pkt);
   if(ret<0)
    fprintf(stderr, "Error while writing video frame\n");
  }
 }
 return ret==AVERROR_EOF;
}

int MyVideoWriter::isOpened()
{
 return isReady;
}

int MyVideoWriter::close()
{
 if(isReady) getLateFrames();    
 if(isReady) av_write_trailer(oc);
 if(isReady) avcodec_free_context(&enc);
 if(isReady) av_frame_free(&frame);
 if(haveTmp)
 {
  av_frame_free(&tmp_frame);
  sws_freeContext(sws_ctx);
 }
 if(isReady && !(fmt->flags&AVFMT_NOFILE))
  avio_close(oc->pb);
 if(isReady) avformat_free_context(oc);
 return 0;
}

string MyVideoWriter::metaName()
{
 string mname=vidfilename+".metadata";
 return mname;
}

int MyVideoWriter::openMeta()
{
 int ret=meta.create(metaName(),1);
 if(ret==0)
 {
  have_meta=1;
 }    
 else
 {
  cerr<<"Error creating Metadata file\n";
  have_meta=0;
 }
 return !have_meta;
}

int MyVideoWriter::hasMeta()
{
 return have_meta;
}

int MyVideoWriter::metaData(unsigned long i, unsigned int n)
{
 int ret=0;
 if(!have_meta)
 {
  ret=openMeta();
 }
 if(ret!=0){
  cerr<< "Error: Can't open Metadata file\n";
  return 1;
 } 
 if(have_meta)
 {
   ret=meta.write((unsigned int)i, n);
   if(ret!=0)
   {
     cerr<< "Error: Can't write Metadata to file\n";
     return 1;
   }  
 }
 return 0;
}

int MyVideoWriter::metaData(unsigned long i, unsigned int n,Metadata::timeValue tv)
{
 int ret=0;
 if(!have_meta)
 {
  ret=openMeta();
 }
 if(ret!=0){
  cerr<< "Error: Can't open Metadata file\n";
  return 1;
 } 
 if(have_meta)
 {
  ret=meta.write((unsigned int)i, n, tv);
  if(ret!=0)
  {
   cerr<< "Error: Can't write Metadata to file\n";
   return 1;
  } 
 }
 return 0;
}

int MyVideoWriter::metaDataToCurrentFrame(unsigned int n)
{
 unsigned int i=addedImages;
 if(i>0)
 {
  i--;  
  int ret=0;
  if(!have_meta)
   ret=openMeta();
  if(ret!=0)
  {
   cerr<< "Error: Can't open Metadata file\n";
   return 1;
  } 
  if(have_meta)
  {
   ret=meta.write(i, n);
   if(ret!=0)
   {
    cerr<< "Error: Can't write Metadata to file\n";
    return 1;
   }
  }
  return 0;
 }
 else
 {
  cerr<< "Error: No frame added yet!\n";
  return 1; 
 } 
}

int MyVideoWriter::metaDataToCurrentFrame(unsigned int n,Metadata::timeValue tv)
{
 unsigned int i=addedImages;
 if(i>0)
 {
  i--;  
  int ret=0;
  if(!have_meta)
   ret=openMeta();
  if(ret!=0){
   cerr<< "Error: Can't open Metadata file\n";
   return 1;
  } 
  if(have_meta)
  {
   ret=meta.write(i,n,tv);
   if(ret!=0)
   {
    cerr<< "Error: Can't write Metadata to file\n";
    return 1;
   }
  }
  return 0;
 }
 else
 {
  cerr<< "Error: No frame added yet!\n";
  return 1;
 } 
}
