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

int Header::init(int streampix6)
{
    ReadOnly=0;
    Streampix6=streampix6;
    Magic=65261;
    for(int i=0; i<24; i++)Name[i]=0;
    UnicodeString str("Norpix");
    const UChar* strbuff= str.getBuffer();
    for(int i=0; i<str.countChar32(); i++)
        if(i<12)
        {
            Name[i*2]=strbuff[i] & 0xff;
            Name[i*2+1]=(strbuff[i] >> 8) & 0xff;
        }
    str.releaseBuffer();
    if(streampix6)
        Version=5;
    else
        Version=4;
    HeaderSize=1024;
    DescriptionFormat=0;
    for(int i=0; i<512; i++)Description[i]=0;
    UnicodeString str2("No Description");
    strbuff= str2.getBuffer();
    for(int i=0; i<str2.countChar32(); i++)
        if(i<256)
        {
            Description[i*2]=strbuff[i] & 0xff;
            Description[i*2+1]=(strbuff[i] >> 8) & 0xff;
        }
    str2.releaseBuffer();

    Width=0;
    Height=0;
    Depth=8;
    RealDepth=8;
    ImageBSize=0;
    ImageFormat=100;
    AllocatedFrames=0;
    Origin=0;
    TrueImageSize=0;
    Framerate=0;
    ReferenceFrame=50.;
    FixedSize=0;
    Flags=0;
    BayerPattern=0;
    Time_offset_us=0;
    ExtendedHeaderSize=0;
    Compression_format=0;
    Reference_time_s=0;
    Reference_time_ms=0;
    Reference_time_us=0;
    if(streampix6)ImageOffset=8192;
    else ImageOffset=1024;
    return 0;
}

int Header::WriteHeader(fstream& seqFile)
{
    if(seqFile)
    {
        seqFile.seekp (0, seqFile.beg);
        seqFile.write(reinterpret_cast<char*>(&Magic), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&Name), 24*sizeof(char));
        seqFile.write(reinterpret_cast<char*>(&Version), sizeof(int));
        seqFile.write(reinterpret_cast<char*>(&HeaderSize), sizeof(int));
        seqFile.write(reinterpret_cast<char*>(&Description), sizeof(char)*512);
        seqFile.write(reinterpret_cast<char*>(&Width), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&Height), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&Depth), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&RealDepth), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&ImageBSize), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&ImageFormat), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&AllocatedFrames), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&Origin), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&TrueImageSize), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&Framerate), sizeof(double));
        seqFile.write(reinterpret_cast<char*>(&DescriptionFormat), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&ReferenceFrame), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&FixedSize), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&Flags), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&BayerPattern), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&Time_offset_us), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&ExtendedHeaderSize), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&Compression_format), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&Reference_time_s), sizeof(unsigned int));
        seqFile.write(reinterpret_cast<char*>(&Reference_time_ms), sizeof(unsigned short));
        seqFile.write(reinterpret_cast<char*>(&Reference_time_us), sizeof(unsigned short));
    }
    else
    {
        cout<< "Could not write to File"<<endl;
        return 1;
    }
    return 0;
}

int Header::WriteHeader(fstream* seqFile)
{
    if(seqFile->is_open())
    {
        seqFile->seekp (0, seqFile->beg);
        seqFile->write(reinterpret_cast<char*>(&Magic), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&Name), 24*sizeof(char));
        seqFile->write(reinterpret_cast<char*>(&Version), sizeof(int));
        seqFile->write(reinterpret_cast<char*>(&HeaderSize), sizeof(int));
        seqFile->write(reinterpret_cast<char*>(&Description), sizeof(char)*512);
        seqFile->write(reinterpret_cast<char*>(&Width), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&Height), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&Depth), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&RealDepth), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&ImageBSize), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&ImageFormat), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&AllocatedFrames), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&Origin), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&TrueImageSize), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&Framerate), sizeof(double));
        seqFile->write(reinterpret_cast<char*>(&DescriptionFormat), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&ReferenceFrame), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&FixedSize), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&Flags), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&BayerPattern), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&Time_offset_us), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&ExtendedHeaderSize), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&Compression_format), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&Reference_time_s), sizeof(unsigned int));
        seqFile->write(reinterpret_cast<char*>(&Reference_time_ms), sizeof(unsigned short));
        seqFile->write(reinterpret_cast<char*>(&Reference_time_us), sizeof(unsigned short));
    }
    else
    {
        cout<< "Could not write to File"<<endl;
        return 1;
    }
    return 0;

}

int Header::update()
{
    ImageBSize=Width*Height;
    if(Streampix6)
        TrueImageSize=int((Width*Height+4+8192)/8192)*8192;
    else
        TrueImageSize=int((Width*Height+4+512)/512)*512;
    return 0;
}

unsigned long long Header::getOffset()
{
    return (unsigned long long)TrueImageSize*AllocatedFrames+(unsigned long long)ImageOffset;
}
