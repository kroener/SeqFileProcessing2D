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
#ifndef Metadata_hpp
#define Metadata_hpp
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

class Metadata
{
public:
    Metadata();
    ~Metadata();
    struct timeValue{
     int sec = 0;
     unsigned short msec = 0;
     unsigned short usec = 0;
    };

    void setInMemory(int i);
    int load(string filen);
    int create(string filen,int withTime=0);
    unsigned int getNum(unsigned int i);
    vector<int> getT(unsigned int i);

    unsigned int getNo();
    bool timeIncluded();
    void createLookupTable();
    int getFrameNumberForTrigger(unsigned int i);
    int write(unsigned int n, unsigned int i);
    int write(unsigned int n, unsigned int i,timeValue ts);
private:
    int openMeta();
    int createMeta();
    int createInMemoryMeta();
    int readHeader();
    int initializeHeader();
    int updateHeader();
    int readAll();
    unsigned int read(unsigned int n);
    Metadata::timeValue readT(unsigned int n);

    int haveMetaData;
    int includesTime;
    int nFrames;
    int metaSize; 
    int inMemory;
    int metaType;
    unsigned int offset;
    string filename;
    fstream metaFile;
    unsigned int* number;
    timeValue* t;
    unsigned int minT;
    unsigned int maxT;
    int nT;
    int* lut;
    int haveLut;
    int wantLut;
    int metadataOffset;
};
#endif
