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
#include "Metadata.hpp"

using namespace std;

Metadata::Metadata()
{
    haveMetaData=0;
    includesTime=0;
    nFrames=0;
    metaSize=0; 
    inMemory=0;
    metaType=5;
    offset=0;
    haveLut=0;
    minT=0;
    maxT=0;
    nT=0;
    wantLut=1;
    metadataOffset=0;
}

Metadata::~Metadata()
{
   if(inMemory){   
       delete[] number;
       if(includesTime)
        delete[] t;
   }
   if(metaFile.is_open())
       metaFile.close();
   if(haveLut) delete[] lut;
}

void Metadata::setInMemory(int i)
{
  if(!haveMetaData)
      inMemory=i;
  else
      cerr<< "Can't select inMemory. I have already loaded some data!\n";
}

int Metadata::load(string filen)
{
    filename=filen;
    int ret=0;

    ret=openMeta();
    if(ret)
      ret=readHeader();
    else
      cerr<<"Open Meta failed\n";
    if(!ret)
    {
        if(inMemory)
        {
          ret=readAll();
          if(ret)
             cerr<<"Error reading All\n";
          else
             haveMetaData=1;       
        }
        else 
            haveMetaData=1;
    }
    else
    {
        cerr<<"Error reading Header!\n";
        haveMetaData=0;
    }
    if(haveMetaData && wantLut) 
        createLookupTable();
    return ret;
}

int Metadata::openMeta()
{
    metaFile.open(filename, ios::in | ios::out | ios::binary);
    if(!metaFile.is_open()){
       cerr<<"MetaFile "<<filename<<" could not be opened\n";
       metaFile.close();
       }
    return metaFile.is_open();
}

int Metadata::createMeta()
{
    metaFile.open(filename,ios::out | ios::binary);
    if(!metaFile.is_open()){
       cerr<<"MetaFile "<<filename<<" could not be created\n";
       metaFile.close();
       }
    return metaFile.is_open();
}

int Metadata::readHeader()
{
    if(metaFile.is_open())
    {
        metaFile.seekg (0, metaFile.beg);
        unsigned int Magic;
        metaFile.read(reinterpret_cast<char*>(&Magic), sizeof(unsigned int));
        if(Magic!=1)
         return 1;    

        metaFile.read(reinterpret_cast<char*>(&offset), sizeof(unsigned int));
          
        metaFile.read(reinterpret_cast<char*>(&nFrames), sizeof(unsigned int));
        metaFile.read(reinterpret_cast<char*>(&metaSize), sizeof(unsigned int));
        metaFile.seekg(0,metaFile.end);
        unsigned int filesize = metaFile.tellg();
        metaFile.seekg(0,metaFile.beg);
        if(metaSize!=0)
        {
            int metaCount=int((filesize-offset)/double(metaSize));
            if(metaCount != nFrames)
            {
                cerr<<"File size("<<metaCount<<") does not fix included number("<<nFrames<<")\n";
                cerr<<"Assuming nFrames="<<metaCount<<"\n";
                nFrames=metaCount;
            }
        }
        if(metaSize==36) 
         includesTime=1;
        return 0;
    }
    else
    {
        cout<< "Could not read from File"<<endl;
        return 1;
    }
    return 0;
}

int Metadata::readAll()
{
    unsigned long nextOffset=0;
    number=new unsigned int[nFrames];
    if(includesTime)
    {
     t=new timeValue[nFrames];
     for(int i=0; i<nFrames; i++)
     {
        t[i].sec=0;
        t[i].msec=0;
        t[i].usec=0;
     }
    }
    for(int i=0; i<nFrames; i++)
        number[i]=0;
    if(metaFile.is_open())
    {
        int NotAllFound=0;
        for(unsigned int n=0; n<nFrames; n++)
        {
            if(metaSize!=0)
             metaFile.seekg (offset+metaSize*n, metaFile.beg);
            else
             metaFile.seekg (offset+nextOffset, metaFile.beg);            
            int found=0;
            int tfound=0;
            unsigned int mSize;
            unsigned int mNumber;
            unsigned int nType;
            unsigned int nSize;
            metaFile.read(reinterpret_cast<char*>(&mSize), sizeof(unsigned int));
            if(mSize!=metaSize && metaSize!=0)
            {
                cerr<<"Error: Wrong size entry for frame "<< n <<" ignoring the frame\n";
                number[n]=0;
                found=1;
            }
            else
            {
                if(metaSize==0)
                 nextOffset+=mSize;
                metaFile.read(reinterpret_cast<char*>(&mNumber), sizeof(unsigned int));
                for(int i=0; i< mNumber; i++)
                {
                    metaFile.read(reinterpret_cast<char*>(&nType), sizeof(unsigned int));
                    metaFile.read(reinterpret_cast<char*>(&nSize), sizeof(unsigned int));
                    if(nType==5 && nSize==4)
                    {
                        unsigned int tmp;
                        metaFile.read(reinterpret_cast<char*>(&tmp), sizeof(unsigned int));
                        if(found==0)
                        {
                            number[n]=tmp;
                            found=1;
                        }
                    }
                    else if(nType==99 && nSize==8)
                    {
                        int ts;
                        unsigned short tms;
                        unsigned short tus;
                        
                        metaFile.read(reinterpret_cast<char*>(&ts), sizeof(int));
                        metaFile.read(reinterpret_cast<char*>(&tms), sizeof(unsigned short));
                        metaFile.read(reinterpret_cast<char*>(&tus), sizeof(unsigned short));
                        
                        if(tfound==0)
                        {
                            t[n].sec=ts;
                            t[n].msec=tms;
                            t[n].usec=tus;
                            tfound=1;
                        }
                        else
                        {
                            cerr<<"I already have one value for this time?!?\n";
                        }
                    
                    }
                    else
                    {
                        cerr<< "Ignoring Data of Type:" <<nType<<"\n";
                        metaFile.seekg (nSize, metaFile.cur);
                    }
                }
            }
            if(found==0)
            {
                cerr<< "Could not find number for frame "<<n<<"\n";
                NotAllFound=1;
            }
       }
       return NotAllFound;
    }
    else
    {
        cout<< "Could not read from File"<<endl;
        return 1;
    }
    return 0;
}

unsigned int Metadata::read(unsigned int n)
{
    if(metaSize==0) {cerr<<"Reading only supported for fixed size meta data!\n";return 0;}
    unsigned int num=0;
    if(metaFile.is_open())
    {
       metaFile.seekg (offset+metaSize*n, metaFile.beg);
       int found=0;
       unsigned int mSize;
       unsigned int mNumber;
       unsigned int nType;
       unsigned int nSize;
       metaFile.read(reinterpret_cast<char*>(&mSize), sizeof(unsigned int));
       if(mSize!=metaSize)
       {
               cerr<<"Error: Wrong size entry for frame "<< n <<"\n";
       }
       else
       {
       metaFile.read(reinterpret_cast<char*>(&mNumber), sizeof(unsigned int));
       for(int i=0; i< mNumber; i++)
        {
            metaFile.read(reinterpret_cast<char*>(&nType), sizeof(unsigned int));
            metaFile.read(reinterpret_cast<char*>(&nSize), sizeof(unsigned int));
           if(nType==5 && nSize==4)
            {
                unsigned int tmp;
                metaFile.read(reinterpret_cast<char*>(&tmp), sizeof(unsigned int));
               num=tmp;
                found=1;
            }
            else
            {
                cerr<< "Ignoring Data of Type:" <<nType<<"\n";
                metaFile.seekg (nSize, metaFile.cur);
            }
        }
        }
        if(found==0)
        {
            cerr<< "Could not find number for frame "<<n<<"\n";
            return 0;
        }
        else
        {
            return num;
        }
    }
    else
    {
        cout<< "Could not read from File"<<endl;
        return 0;
    }
}

Metadata::timeValue Metadata::readT(unsigned int n)
{
    timeValue ts;
    ts.sec=0;
    ts.usec=0;
    ts.msec=0;
    if(includesTime)
    unsigned int num=0;
    if(metaFile.is_open())
    {
       metaFile.seekg (offset+metaSize*n, metaFile.beg);
        int found=0;
        unsigned int mSize;
        unsigned int mNumber;
        unsigned int nType;
        unsigned int nSize;
        metaFile.read(reinterpret_cast<char*>(&mSize), sizeof(unsigned int));
        if(mSize!=metaSize)
        {
                cerr<<"Error: Wrong size entry for frame "<< n <<"\n";
        }
        else
        {
        
        metaFile.read(reinterpret_cast<char*>(&mNumber), sizeof(unsigned int));
       for(int i=0; i< mNumber; i++)
        {
            metaFile.read(reinterpret_cast<char*>(&nType), sizeof(unsigned int));
            metaFile.read(reinterpret_cast<char*>(&nSize), sizeof(unsigned int));
            if(nType==99 && nSize==8)
            {
                int tsec;
                unsigned short tms;
                unsigned short tus;
                        
                metaFile.read(reinterpret_cast<char*>(&tsec), sizeof(int));
                metaFile.read(reinterpret_cast<char*>(&tms), sizeof(unsigned short));
                metaFile.read(reinterpret_cast<char*>(&tus), sizeof(unsigned short));
                ts.sec=tsec;
                ts.msec=tms;
                ts.usec=tus;
                found=1;
            }
            else
            {
                cerr<< "Ignoring Data of Type:" <<nType<<"\n";
                metaFile.seekg (nSize, metaFile.cur);
            }
        }
        }
        if(found==0)
        {
            cerr<< "Could not find number for frame "<<n<<"\n";
            return ts;
        }
        else
        {
            return ts;
        }
    }
    else
    {
        cout<< "Could not read from File"<<endl;
        return ts;
    }
}


unsigned int Metadata::getNum(unsigned int i)
{
    if(i<nFrames && haveMetaData)
    {
        if(inMemory)
            return number[i]+metadataOffset;
        else
        {
            return read(i)+metadataOffset;
        }
    }
    else
    {
        return 0;
    }
}

vector<int> Metadata::getT(unsigned int i)
{
 vector<int> tmp;
 if(includesTime)
 {
    if(i<nFrames && haveMetaData)
    {
        if(inMemory)
        {
            tmp.push_back(t[i].sec);
            tmp.push_back(t[i].msec);
            tmp.push_back(t[i].usec);
        }
        else
        {
            timeValue ts=readT(i);
            tmp.push_back(ts.sec);
            tmp.push_back(ts.msec);
            tmp.push_back(ts.usec);
        }
    }
    else
    {
        return tmp;
    }
 }
 else
  return tmp;
 return tmp;
}

unsigned int Metadata::getNo()
{
    return nFrames;
}

bool Metadata::timeIncluded()
{
    return includesTime;
}

void Metadata::createLookupTable()
{
    if(haveMetaData)
    {
        minT=getNum(0);
        maxT=getNum(0);
        for(int i=0; i<nFrames;i++)
        {
          unsigned int tmp=getNum(i);
          if(tmp<minT)minT=tmp;
          if(tmp>maxT)maxT=tmp;
        }
        nT=maxT-minT+1;
        if(haveLut) delete[] lut;
        lut=new int [nT];
        for(int i=0; i<nT;i++) lut[i]=-1;
        for(int i=0; i<nFrames;i++)
        {
          lut[getNum(i)-minT]=i;
        }
        haveLut=1;
        int lastgood=0;
        for(int i=0; i<nT;i++)
        {
         if(lut[i]>=0)
          lastgood=i;
         else
          lut[i]=lastgood-i;
        }
    }
}

int Metadata::getFrameNumberForTrigger(unsigned int i)
{
   int value=-1;
   i=i-metadataOffset;
   if(i>=minT && i<=maxT && haveMetaData && haveLut)
   {
       value=lut[i-minT];   
   }
   return value;
}

int Metadata::create(string filen, int withTime /* =0 */ )
{
    wantLut=0;
    inMemory=0;
    includesTime=withTime;
    filename=filen;
    int ret=0;
    ret=openMeta();
    if(!ret)
    {
       cerr<<" Metafile "<<filename<<" does not exists, trying to create it:\n";
       ret=createMeta();
       if(!ret)
          cerr<<"Something went wrong...\n";
       
    }
    else
     cerr<<" Merafile exists open in read and write mode...\n";
    if(ret)
       ret=initializeHeader();
    else
      cerr<<"Open Meta failed\n";
    if(ret==0)
    {
       haveMetaData=1;
       return 0;       
    }
    else
    {
        cerr<<"Error creating Header!\n";
        haveMetaData=0;
        return 1;
    }
}

int Metadata::initializeHeader()
{
    if(metaFile.is_open())
    {
        offset=256;
        nFrames=0; 
        if(!includesTime)
         metaSize=20;
        else
         metaSize=36;
        metaFile.seekp (0, metaFile.beg);
        unsigned int Magic=1;
        metaFile.write(reinterpret_cast<char*>(&Magic), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&offset), sizeof(unsigned int));         
        metaFile.write(reinterpret_cast<char*>(&nFrames), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&metaSize), sizeof(unsigned int));
        return 0;
    }
    else
    {
        cout<< "Could not read from File"<<endl;
        return 1;
    }
    return 0;
}

int Metadata::updateHeader()
{
    if(metaFile.is_open())
    {
        metaFile.seekp (2*sizeof(unsigned int), metaFile.beg);
        metaFile.write(reinterpret_cast<char*>(&nFrames), sizeof(unsigned int));
        return 0;
    }
    else
    {
        cout<< "Could not update file"<<endl;
        return 1;
    }
    return 0;
}

int Metadata::write(unsigned int n, unsigned int i)
{
    if(metaFile.is_open())
    {
        metaFile.seekp (offset+metaSize*n, metaFile.beg);
        unsigned int mSize=20;
        if(includesTime) mSize=36;
        unsigned int mNumber=1;
        unsigned int nType=5;
        unsigned int nSize=4;
        metaFile.write(reinterpret_cast<char*>(&mSize), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&mNumber), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&nType), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&nSize), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&i), sizeof(unsigned int));
        if(n>=nFrames)
        {
         nFrames=n+1;
         updateHeader();
        }
        return 0;
    }
    else
    {
        cout<< "Could not write from File"<<endl;
        return 1;
    }
}

int Metadata::write(unsigned int n, unsigned int i,timeValue ts)
{
if(!includesTime)
{
 write(n,i);
}
else
{
    if(metaFile.is_open())
    {
        metaFile.seekp (offset+metaSize*n, metaFile.beg);
        unsigned int mSize=36;
        unsigned int mNumber=2;
        unsigned int nType=5;
        unsigned int nSize=4;
        unsigned int tType=99;
        unsigned int tSize=8;
        
        metaFile.write(reinterpret_cast<char*>(&mSize), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&mNumber), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&nType), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&nSize), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&i), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&tType), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&tSize), sizeof(unsigned int));
        metaFile.write(reinterpret_cast<char*>(&ts.sec), sizeof(int));
        metaFile.write(reinterpret_cast<char*>(&ts.msec), sizeof(unsigned short));
        metaFile.write(reinterpret_cast<char*>(&ts.usec), sizeof(unsigned short));

        if(n>=nFrames)
        {
         nFrames=n+1;
         updateHeader();
        }
        return 0;
    }
    else
    {
        cout<< "Could not write from File"<<endl;
        return 1;
    }
}
return 0;
}
