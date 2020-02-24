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
#ifndef Header_hpp
#define Header_hpp
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>

using namespace std;

/** \brief Header Class for the access to *.seq files
  *  Class to read and write header of *.seq files 
  *  compatible with StreamPix 5 and StreamPix 6
  */
class Header
{
public:
    Header();
    Header(int streampix6);
    Header(const Header& a);
    ~Header() {};
    void readOnly()
    {
        ReadOnly=1;
    };
    void readWrite()
    {
        ReadOnly=0;
    };
    void isVid()
    {
        is_vid=1;
    };
    void isImg()
    {
        is_img=1;
    };
    int ReadHeader(fstream *seqFile);
    //get Values
    unsigned int magic()
    {
        return Magic;
    };
    int version()
    {
        return Version;
    };
    int headerSize()
    {
        return HeaderSize;
    };
    int descriptionFormat()
    {
        return DescriptionFormat;
    };
    unsigned int width()
    {
        return Width;
    };
    unsigned int height()
    {
        return Height;
    };
    unsigned int depth()
    {
        return Depth;
    };
    unsigned int realDepth()
    {
        return RealDepth;
    };
    unsigned int imageBSize()
    {
        return ImageBSize;
    };
    unsigned int imageFormat()
    {
        return ImageFormat;
    };
    unsigned int allocatedFrames()
    {
        return AllocatedFrames;
    };
    unsigned int origin()
    {
        return Origin;
    };
    unsigned int trueImageSize()
    {
        return TrueImageSize;
    };
    double framerate()
    {
        return Framerate;
    };
    unsigned int referenceFrame()
    {
        return ReferenceFrame;
    };
    unsigned int fixedSize()
    {
        return FixedSize;
    };
    unsigned int flags()
    {
        return Flags;
    };
    unsigned int bayerPattern()
    {
        return BayerPattern;
    };
    unsigned int time_offset_us()
    {
        return Time_offset_us;
    };
    unsigned int extendedHeaderSize()
    {
        return ExtendedHeaderSize;
    };
    unsigned int compression_format()
    {
        return Compression_format;
    };
    unsigned int reference_time_s()
    {
        return Reference_time_s;
    };
    unsigned short reference_time_ms()
    {
        return Reference_time_ms;
    };
    unsigned short reference_time_us()
    {
        return Reference_time_us;
    };
    unsigned int streampix6()
    {
        return Streampix6;
    };
    //set Values
    void magic(unsigned int v)
    {
        if(!ReadOnly) Magic=v;
    };
    void name(char v[24])
    {
        if(!ReadOnly) strcpy(v,Name);
    };
    void version(int v)
    {
        if(!ReadOnly) Version=v;
    };
    void headerSize(int v)
    {
        if(!ReadOnly) HeaderSize=v;
    };
    void descriptionFormat(int v)
    {
        if(!ReadOnly) DescriptionFormat=v;
    };
    void utf16description(char v[512])
    {
        if(!ReadOnly) strcpy(v,Description);
    };
    void width(unsigned int v)
    {
        if(!ReadOnly) Width=v;
    };
    void height(unsigned int v)
    {
        if(!ReadOnly) Height=v;
    };
    void depth(unsigned int v)
    {
        if(!ReadOnly) Depth=v;
    };
    void realDepth(unsigned int v)
    {
        if(!ReadOnly) RealDepth=v;
    };
    void imageBSize(unsigned int v)
    {
        if(!ReadOnly) ImageBSize=v;
    };
    void imageFormat(unsigned int v)
    {
        if(!ReadOnly) ImageFormat=v;
    };
    void allocatedFrames(unsigned int v)
    {
        if(!ReadOnly) AllocatedFrames=v;
    };
    void addFrame()
    {
        if(!ReadOnly) AllocatedFrames++;
    };
    void origin(unsigned int v)
    {
        if(!ReadOnly) Origin=v;
    };
    void trueImageSize(unsigned int v)
    {
        if(!ReadOnly) TrueImageSize=v;
    };
    void framerate(double v)
    {
        if(!ReadOnly) Framerate=v;
    };
    void referenceFrame(unsigned int v)
    {
        if(!ReadOnly) ReferenceFrame=v;
    };
    void fixedSize(unsigned int v)
    {
        if(!ReadOnly) FixedSize=v;
    };
    void flags(unsigned int v)
    {
        if(!ReadOnly) Flags=v;
    };
    void bayerPattern(unsigned int v)
    {
        if(!ReadOnly) BayerPattern=v;
    };
    void time_offset_us(unsigned int v)
    {
        if(!ReadOnly) Time_offset_us=v;
    };
    void extendedHeaderSize(unsigned int v)
    {
        if(!ReadOnly) ExtendedHeaderSize=v;
    };
    void compression_format(unsigned int v)
    {
        if(!ReadOnly) Compression_format=v;
    };
    void reference_time_s(unsigned int v)
    {
        if(!ReadOnly) Reference_time_s=v;
    };
    void reference_time_ms(unsigned short v)
    {
        if(!ReadOnly) Reference_time_ms=v;
    };
    void reference_time_us(unsigned short v)
    {
        if(!ReadOnly) Reference_time_us=v;
    };
    unsigned long long imageStartOffset(unsigned int n);
    unsigned long long TimeStartOffset(unsigned int n);
    int seqHasMeta();
    void setSeqHasMeta(int i);
    int init(int streampix6);
    int WriteHeader(fstream& seqFile);
    int WriteHeader(fstream* seqFile);
    int update();
    unsigned long long getOffset();

private:
    unsigned int Magic;
    char Name[24];
    int Version;
    int HeaderSize;
    int DescriptionFormat;
    char Description[512];
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int RealDepth;
    unsigned int ImageBSize;
    unsigned int ImageFormat;
    unsigned int AllocatedFrames;
    unsigned int Origin;
    unsigned int TrueImageSize;
    double Framerate;
    unsigned int ReferenceFrame;
    unsigned int FixedSize;
    unsigned int Flags;
    unsigned int BayerPattern;
    unsigned int Time_offset_us;
    unsigned int ExtendedHeaderSize;
    unsigned int Compression_format;
    unsigned int Reference_time_s;
    unsigned short Reference_time_ms;
    unsigned short Reference_time_us;

    int ReadOnly;
    int Streampix6;
    int ImageOffset;
    int seq_has_meta;
    int is_vid;
    int is_img;
};
#endif
