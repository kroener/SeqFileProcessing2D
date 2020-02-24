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
#include <string.h>
#include <sstream>
#include <vector>
#include "Image.hpp"
#include "oSeq.hpp"
#include "Header.hpp"
#include "Metadata.hpp"

using namespace std;

oSeq::oSeq()
{
    headerReady=0;
    streampix6=1;
    numFrames=0;
    header.init(streampix6);
    have_meta=0;
}

oSeq::oSeq(int version)
{
    headerReady=0;
    if(version==6)
        streampix6=1;
    else
        streampix6=0;
    numFrames=0;
    header.init(streampix6);
    have_meta=0;
}

oSeq::~oSeq()
{
    if(seqFile.is_open())
    {
        close();
    }
}

int oSeq::open(string filename)
{
    seqfilename=filename;
    if(fileExists(filename.c_str()))
        seqFile.open(filename.c_str(),ios::binary|ios::out|ios::in);
    else
        seqFile.open(filename.c_str(),ios::binary|ios::out); 
    if(seqFile.is_open())
    {
        cerr << "Could not open "<<filename<<endl;
    }
    return 0;
}

int oSeq::close()
{
    if(seqFile.is_open())
    {
        if(headerReady)
            header.WriteHeader(seqFile);
        seqFile.close();
    }
    headerReady=0;
    streampix6=0;
    numFrames=0;
    header.init(streampix6);
    return 0;
}

int oSeq::saveImage(Image<unsigned char>* img)
{
    if(seqFile.is_open())
    {
        if(header.width()==0)
        {
            header.width(img->getX());
            header.height(img->getY());
            header.update();
            headerReady=1;
        }
        img->saveToSeq(seqFile,header);
        header.addFrame();
    }
    else
        cerr<<"Error file is not open?\n";
    return 0;
}

int oSeq::setFramerate(double v)
{
    header.framerate(v);
    return 0;
}

string oSeq::metaName()
{
    string mname = seqfilename+".metadata";
    return mname;
}

int oSeq::openMeta()
{
    cerr<< "Trying to open: "<<metaName()<<"\n";
    int ret=meta.create(metaName(),1);
    if(ret==0)
    {
        have_meta=1;
        header.setSeqHasMeta(1);
    }    
    else{
        cerr<<"Error creating Metadata file\n";
        have_meta=0;
    }
    return !have_meta;
}

int oSeq::hasMeta()
{
    return have_meta;
}

int oSeq::metaData(unsigned long i, unsigned int n)
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

int oSeq::metaData(unsigned long i, unsigned int n,Metadata::timeValue tv)
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

int oSeq::metaDataToCurrentFrame(unsigned int n)
{
    unsigned int i=header.allocatedFrames();
    if(i>0)
    {
        i--;  
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

int oSeq::metaDataToCurrentFrame(unsigned int n, Metadata::timeValue tv)
{
    unsigned int i=header.allocatedFrames();
    if(i>0)
    {
        i--;  
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
            ret=meta.write(i, n, tv);
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

int oSeq::setNumberOfFrames(unsigned int n)
{
    header.allocatedFrames(n);
    numFrames=n;
    return 0;
}

int oSeq::setSize(int w, int h)
{
    header.width(w);
    header.height(h);
    header.update();
    headerReady=1;
    return 0;
}

bool oSeq::fileExists(const char *fileName)
{
    ifstream infile(fileName);
    return infile.good();
}
