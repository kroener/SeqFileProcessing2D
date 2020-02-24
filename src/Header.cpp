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
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <string.h>
#include <sstream>
#include <vector>
#include "Header.hpp"
#include "unicode/utypes.h"
#include "unicode/ucnv.h"
#include "unicode/ustring.h"
#include "unicode/uchar.h"
#include "unicode/uloc.h"
#include "unicode/unistr.h"

using icu::UnicodeString;

using namespace std;

Header::Header()
{
    ReadOnly=0;
    seq_has_meta=0;
    is_vid=0;
    is_img=0;
}

Header::Header(const Header& a)
{
    Magic=a.Magic;
    strcpy(Name,a.Name);
    Version=a.Version;
    HeaderSize=a.HeaderSize;
    DescriptionFormat=a.DescriptionFormat;
    strcpy(Description,a.Description);
    Width=a.Width;
    Height=a.Height;
    Depth=a.Depth;
    RealDepth=a.RealDepth;
    ImageBSize=a.ImageBSize;
    ImageFormat=a.ImageFormat;
    AllocatedFrames=a.AllocatedFrames;
    Origin=a.Origin;
    TrueImageSize=a.TrueImageSize;
    Framerate=a.Framerate;
    ReferenceFrame=a.ReferenceFrame;
    FixedSize=a.FixedSize;
    Flags=a.Flags;
    BayerPattern=a.BayerPattern;
    Time_offset_us=a.Time_offset_us;
    ExtendedHeaderSize=a.ExtendedHeaderSize;
    Compression_format=a.Compression_format;
    Reference_time_s=a.Reference_time_s;
    Reference_time_ms=a.Reference_time_ms;
    Reference_time_us=a.Reference_time_us;

    ReadOnly=a.ReadOnly;
    Streampix6=a.Streampix6;
    ImageOffset=a.ImageOffset;
    seq_has_meta=a.seq_has_meta;
    is_vid=a.is_vid;
    is_img=a.is_img;
}

int Header::ReadHeader(fstream *seqFile)
{
    if(seqFile)
    {
        seqFile->seekg (0, seqFile->beg);
        seqFile->read(reinterpret_cast<char*>(&Magic), sizeof(unsigned int));
        if(Magic!=65261)
         return 1;    
        seqFile->read(reinterpret_cast<char*>(&Name), 24*sizeof(char));
        seqFile->read(reinterpret_cast<char*>(&Version), sizeof(int));
        seqFile->read(reinterpret_cast<char*>(&HeaderSize), sizeof(int));
        seqFile->read(reinterpret_cast<char*>(&Description), sizeof(char)*512);
        seqFile->read(reinterpret_cast<char*>(&Width), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&Height), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&Depth), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&RealDepth), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&ImageBSize), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&ImageFormat), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&AllocatedFrames), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&Origin), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&TrueImageSize), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&Framerate), sizeof(double));
        seqFile->read(reinterpret_cast<char*>(&DescriptionFormat), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&ReferenceFrame), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&FixedSize), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&Flags), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&BayerPattern), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&Time_offset_us), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&ExtendedHeaderSize), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&Compression_format), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&Reference_time_s), sizeof(unsigned int));
        seqFile->read(reinterpret_cast<char*>(&Reference_time_ms), sizeof(unsigned short));
        seqFile->read(reinterpret_cast<char*>(&Reference_time_us), sizeof(unsigned short));
        ReadOnly=1;
        if(Version>=5)
        {
            Streampix6=1;
            ImageOffset=8192;
        }
        else
        {
            Streampix6=0;
            ImageOffset=1024;
        }
    }
    else
    {
        cout<< "Could not read from File"<<endl;
        return 1;
    }
    return 0;
}

unsigned long long Header::imageStartOffset(unsigned int n)
{
    if(n<AllocatedFrames)
    {
        return (unsigned long long)ImageOffset+(unsigned long long)n*TrueImageSize;
    }
    else
    {
        return 0;
    }
}

unsigned long long Header::TimeStartOffset(unsigned int n)
{
    if(n<AllocatedFrames)
    {
        return ImageOffset+(unsigned long long)n*TrueImageSize+ImageBSize;
    }
    else
    {
        return 0;
    }
}

int Header::seqHasMeta()
{
    return seq_has_meta; 
}

void Header::setSeqHasMeta(int i)
{
    seq_has_meta=i; 
}
