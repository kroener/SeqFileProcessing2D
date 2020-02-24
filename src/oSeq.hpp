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
#ifndef oSeq_hpp
#define oSeq_hpp
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
#include "Header.hpp"
#include "Metadata.hpp"

using namespace std;

class oSeq
{
public:
    oSeq();
    oSeq(int version);
    ~oSeq();
    int open(string filename);
    int close();
    int saveImage(Image<unsigned char>* img);
    int setFramerate(double v);
    string metaName();
    int metaData(unsigned long i,unsigned int n);
    int metaDataToCurrentFrame(unsigned int n);
    int metaData(unsigned long i,unsigned int n, Metadata::timeValue tv);
    int metaDataToCurrentFrame(unsigned int n, Metadata::timeValue tv);
    int hasMeta();
    int setNumberOfFrames(unsigned int n);
    int setSize(int w, int h);

private:
    int writeHeader();
    int initDefaultHeader();
    int openMeta();
    bool fileExists(const char *fileName);
    Header header;
    fstream seqFile;
    string seqfilename;

    unsigned long numFrames;
    int streampix6;
    int headerReady;
    Metadata meta;
    int have_meta;
};
#endif
