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
#include <stdio.h>

#include "Image.hpp"
#include "iSeq.hpp"
#include "MyVideoReader.hpp"
#include "Header.hpp"
#include "Metadata.hpp"
//opencv
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
//end opencv
#include "MosquitoesInFrame.hpp"

using namespace std;
using namespace cv;

iSeq::iSeq()
{
//seqFile=new ifstream;
    requiredImages=1;
    nWorkImages=2;
    nShortImages=1;
    InMemory=new Image<unsigned char>[requiredImages];
    DoubleWorkImages=new Image<double>[nWorkImages];
    ShortWorkImages=new Image<short>[nShortImages];
    nextImage=0;
    framePointer=new int[requiredImages];
    thisFrame=0;
    has_buffer=1;
    haveBackupFile=0;
    backupInterval=1;
    have_meta=0;
    isVid=0;
    isImg=0;
    positionFileName="";
    trackFileName="";
    hasRoi=false;
    roi.x=0;
    roi.y=0;
    roi.width=0;
    roi.height=0;
    hasPolyRoi=false;
}

iSeq::~iSeq()
{
if(has_buffer==1)
{
    delete[] InMemory;
    delete[] DoubleWorkImages;
    delete[] ShortWorkImages;
    delete[] framePointer;
}
if(seqFile != nullptr)
{
 seqFile->close();
 delete seqFile;
}
if(isVid==1)
 if(vid != nullptr)
  delete vid;
}

iSeq::iSeq(iSeq&& other)
{
    requiredImages=other.requiredImages;
    nWorkImages=other.nWorkImages;
    nShortImages=other.nShortImages;
    InMemory=other.InMemory;
    DoubleWorkImages=other.DoubleWorkImages;
    ShortWorkImages=other.ShortWorkImages;
    header=other.header;
    nextImage=other.nextImage;
    framePointer=other.framePointer;
    thisFrame=other.thisFrame;
    seqfilename=other.seqfilename;
    positionFileName=other.positionFileName;
    trackFileName=other.trackFileName;
    nextImage=other.nextImage;
    thisFrame=other.thisFrame;
    seqFile=other.seqFile;
    has_buffer=1;
    other.has_buffer=0;  
    other.seqFile=nullptr;
    isVid=other.isVid;
    isImg=other.isImg;
    for(unsigned i=0;i<other.requiredImages;i++) other.InMemory[i].hasNoBuffer();
    for(unsigned i=0;i<other.nWorkImages;i++) other.DoubleWorkImages[i].hasNoBuffer();
    for(unsigned i=0;i<other.nShortImages;i++) other.ShortWorkImages[i].hasNoBuffer();
    haveBackupFile=other.haveBackupFile;
    backupInterval=other.backupInterval;
    have_meta=other.have_meta;
    if(have_meta) loadMeta();
    if(isVid) {
    vid=new MyVideoReader();
    vid->open(seqfilename);    
    }
    hasRoi=other.hasRoi;
    roi=other.roi;
    hasPolyRoi=other.hasPolyRoi;
    polyRoi=other.polyRoi;
    if(other.isImg)
    {
     other.img.copyTo(img);
     Img.copyFrom(img);
    }
}

iSeq& iSeq::operator=(iSeq&& other)
{
    if(this != &other)
    {
    if(has_buffer==1)
    {
    delete[] InMemory;
    delete[] DoubleWorkImages;
    delete[] ShortWorkImages;
    delete[] framePointer;
    }
    requiredImages=other.requiredImages;
    nWorkImages=other.nWorkImages;
    nShortImages=other.nShortImages;
    InMemory=other.InMemory;
    DoubleWorkImages=other.DoubleWorkImages;
    ShortWorkImages=other.ShortWorkImages;
    header=other.header;
    nextImage=other.nextImage;
    framePointer=other.framePointer;
    thisFrame=other.thisFrame;
    seqfilename=other.seqfilename;
    positionFileName=other.positionFileName;
    trackFileName=other.trackFileName;
    nextImage=other.nextImage;
    thisFrame=other.thisFrame;
    seqFile=other.seqFile;
    haveBackupFile=other.haveBackupFile;
    backupInterval=other.backupInterval;
    have_meta=other.have_meta;
    if(have_meta) loadMeta();
    isVid=other.isVid;
    isImg=other.isImg;
    if(other.isVid) other.vid->release();
    if(isVid) vid->open(seqfilename);    
    has_buffer=1;
    other.has_buffer=0;
    other.seqFile=nullptr;
    for(unsigned i=0;i<other.requiredImages;i++) other.InMemory[i].hasNoBuffer();
    for(unsigned i=0;i<other.nWorkImages;i++) other.DoubleWorkImages[i].hasNoBuffer();
    for(unsigned i=0;i<other.nShortImages;i++) other.ShortWorkImages[i].hasNoBuffer();
    hasRoi=other.hasRoi;
    roi=other.roi;
    hasPolyRoi=other.hasPolyRoi;
    polyRoi=other.polyRoi;
    if(other.isImg)
    {
     other.img.copyTo(img);
     Img.copyFrom(img);
    }
    return *this;    
   }
}

int iSeq::open(string filename)
{
    seqFile=new fstream;
    seqfilename=filename;
    seqFile->open(filename.c_str(),ios::binary|ios::in);
    int status=0;
    if(seqFile->is_open())
    {
        status=header.ReadHeader(seqFile);
        if(status==1)
        {
           seqFile->close();
           if(has_suffix(filename,".mp4") || has_suffix(filename,".mkv") || has_suffix(filename,".avi"))
           {
           vid=new MyVideoReader();
           status=vid->open(filename);
           if(status==1)
           {
              header.readWrite();
              header.isVid();
              header.width(vid->getW());
              header.height(vid->getH());
              header.allocatedFrames(vid->get_total_frames());
              header.framerate(vid->get_fps());
              header.depth(8);
              isVid=1;
              status=0;
           }
           else
           {
              vid->release();
              delete vid;
              status=1;
           }
           }
           else
           {
           img = cv::imread(filename, cv::IMREAD_GRAYSCALE);
           if(img.data)
           {
              header.readWrite();
              header.isImg();
              header.width(img.size().width);
              header.height(img.size().height);
              header.allocatedFrames(1);
              header.framerate(1);
              header.depth(8);
              isImg=1;
              Img.copyFrom(img);
              status=0;
           }
           else
           {
              status=1;
           }           
           }
        }
    }
    else
    {
        cerr << "Could not read from"<<filename<<endl;
        return 1;
    }
    if(status==1)
    {
        cerr << "Error: Not a valid seq file:"<<filename<<endl;
        return 1;
    }
    else
    {
     loadMeta();     
     return 0;
    }
}

int iSeq::loadImage(unsigned long i)
{
    if(!isImg)
    {
    if(isVid==0 && seqFile->is_open())
    {
        if(i == (unsigned) thisFrame+1 && i>requiredImages)
        {
            if(i<header.allocatedFrames())
                InMemory[nextImage].loadFromSeq(seqFile, header, i);
            thisFrame=i;
            for(unsigned long i=requiredImages-1; i>0; i--)
                framePointer[i]=framePointer[i-1];
            framePointer[0]=nextImage;
            nextImage=framePointer[requiredImages-1];
        }
        else
        {
            if(i<header.allocatedFrames())
            {
                if(requiredImages<=(unsigned)i+1)
                {
                    for(unsigned long o=0; o<requiredImages; o++)
                    {
                        InMemory[o].loadFromSeq(seqFile, header, i-o);
                    }
                }
                else
                {
                    for(unsigned long o=0; o<=(unsigned)i; o++)
                    {
                        InMemory[o].loadFromSeq(seqFile, header, i-o);
                    }
            
                }
            }
            thisFrame=i;
            for(unsigned long i=0; i<requiredImages; i++)
                framePointer[i]=i;
            nextImage=framePointer[requiredImages-1];
        }
    return 0;
    }
    else if(isVid==1 && vid->isOpened())
    {
        if(i == (unsigned) thisFrame+1 && i>requiredImages)
        {
            if(i<header.allocatedFrames()){
                loadFromVid(InMemory[nextImage]);
                if(have_meta)
                 if(meta.timeIncluded())
                 {
                  vector<int> tv=meta.getT(i);
                  if(tv.size()==3)
                  {
                   InMemory[nextImage].setTsec(tv[0]);
                   InMemory[nextImage].setTmsec(tv[1]);
                   InMemory[nextImage].setTusec(tv[2]);
                  }
                 }
            }
            thisFrame=i;
            for(unsigned long i=requiredImages-1; i>0; i--)
                framePointer[i]=framePointer[i-1];
            framePointer[0]=nextImage;
            nextImage=framePointer[requiredImages-1];
        }
        else
        {
            if(i<header.allocatedFrames())
            {
                if(requiredImages<=(unsigned)i+1)
                {
                    for(unsigned long o=0; o<requiredImages; o++)
                    {
                        if(o==0){
                            loadFromVid(InMemory[requiredImages-o-1], i-requiredImages+1);
                            if(have_meta)
                             if(meta.timeIncluded())
                            {
                                vector<int> tv=meta.getT(i-requiredImages+1);
                                if(tv.size()==3)
                                {
                                    InMemory[requiredImages-o-1].setTsec(tv[0]);
                                    InMemory[requiredImages-o-1].setTmsec(tv[1]);
                                    InMemory[requiredImages-o-1].setTusec(tv[2]);
                                }
                            }
                        }
                        else
                        {
                            loadFromVid(InMemory[requiredImages-o-1]);
                            if(have_meta)
                             if(meta.timeIncluded())
                            {
                                vector<int> tv=meta.getT(i-requiredImages+1+o);
                                if(tv.size()==3)
                                {
                                    InMemory[requiredImages-o-1].setTsec(tv[0]);
                                    InMemory[requiredImages-o-1].setTmsec(tv[1]);
                                    InMemory[requiredImages-o-1].setTusec(tv[2]);
                                }
                            }
                        }
                    }
                }
                else
                {
                    for(unsigned long o=0; o<=(unsigned)i; o++)
                    {
                        if(o==0){
                            loadFromVid(InMemory[i-o], 0);
                            if(have_meta)
                             if(meta.timeIncluded())
                            {
                                vector<int> tv=meta.getT(0);
                                if(tv.size()==3)
                                {
                                    InMemory[i-o].setTsec(tv[0]);
                                    InMemory[i-o].setTmsec(tv[1]);
                                    InMemory[i-o].setTusec(tv[2]);
                                }
                            }
                        }
                        else
                        {
                            loadFromVid(InMemory[i-o]);
                            if(have_meta)
                             if(meta.timeIncluded())
                            {
                                vector<int> tv=meta.getT(o);
                                if(tv.size()==3)
                                {
                                    InMemory[i-o].setTsec(tv[0]);
                                    InMemory[i-o].setTmsec(tv[1]);
                                    InMemory[i-o].setTusec(tv[2]);
                                }
                            }
                        }
                    }

                }
            }
            thisFrame=i;
            for(unsigned long i=0; i<requiredImages; i++)
                framePointer[i]=i;
            nextImage=framePointer[requiredImages-1];
        }
    return 0;   
    }
    }
    else
    {
     if(have_meta)
      if(meta.timeIncluded())
     {
      vector<int> tv=meta.getT(i);
      if(tv.size()==3)
      {
       Img.setTsec(tv[0]);
       Img.setTmsec(tv[1]);
       Img.setTusec(tv[2]);
      }
     }
     return 0;
    }
    return 1;
}

Image<short int>  iSeq::diffImage(unsigned long i, unsigned long j)
{
 if(!isImg)
 {
    Image<short int> tmp;
    if(i<requiredImages && j<requiredImages)
    {
        tmp.create(header.width(),header.height());
        if(hasRoi)
        {
        InMemory[framePointer[i]].diff(InMemory[framePointer[j]],tmp,roi);        
        }
        else
        {
        InMemory[framePointer[i]].diff(InMemory[framePointer[j]],tmp);
        }
    }
    
    return tmp;
    }
    else
    {
     Image<short int> tmp;
     return tmp;
    }

}

unsigned long iSeq::allocatedFrames()
{
    return header.allocatedFrames();
}

unsigned long iSeq::currentFrame()
{
    return thisFrame;
}

Image<unsigned char>* iSeq::getImagePointer(unsigned long i)
{
    if(isImg)
    {
      return &Img;
    }
    if(i<requiredImages)
    {
        return &InMemory[framePointer[i]];
    }
    else
        return 0;
}

vector<int>  iSeq::timeOfFrameI(unsigned long n)
{
 if(!isImg)
 {
    vector<int> tmp;

    if(isVid==0 && n<header.allocatedFrames())
    {
        if(seqFile)
        {
            seqFile->seekg(header.TimeStartOffset(n), seqFile->beg);
            if(header.streampix6())
            {
                int sec;
                unsigned short ms,us;
                seqFile->read(reinterpret_cast<char*>(&sec), sizeof(int));
                seqFile->read(reinterpret_cast<char*>(&ms), sizeof(unsigned short));
                seqFile->read(reinterpret_cast<char*>(&us), sizeof(unsigned short));
                tmp.push_back(sec);
                tmp.push_back(ms);
                tmp.push_back(us);
            }
            else
            {
                int sec;
                unsigned short ms,us;
                seqFile->read(reinterpret_cast<char*>(&sec), sizeof(int));
                seqFile->read(reinterpret_cast<char*>(&ms), sizeof(unsigned short));
                seqFile->read(reinterpret_cast<char*>(&us), sizeof(unsigned short));
                tmp.push_back(sec);
                tmp.push_back(ms);
                tmp.push_back(us);
            }
        }
    }
    else if(isVid && have_meta && n<header.allocatedFrames())
    {
     tmp=meta.getT(n);
    }
    return tmp;
  }
  else
  {
   if(have_meta && n<header.allocatedFrames())
   {
    vector<int> tmp=meta.getT(n);
    return tmp;
   }
   else
    return vector<int>();
  }
}

int iSeq::sec(unsigned long i)
{
    if(i<requiredImages)
     if(!isImg)
        return InMemory[framePointer[i]].getTsec();
    return 0;
}

unsigned short iSeq::ms(unsigned long i)
{
    if(i<requiredImages)
     if(!isImg)
        return InMemory[framePointer[i]].getTmsec();
    return 0;
}

unsigned short iSeq::us(unsigned long i)
{
    if(i<requiredImages)
     if(!isImg)
        return InMemory[framePointer[i]].getTusec();
    return 0;
}

Mat iSeq::image(unsigned long n, unsigned long i)
{
    Mat tmp;
    if(isImg)
    {
     cerr<<"Return image of size:"<<img.size().width<<"x"<<img.size().height<<"\n";
     return img;
    }
    if(n==0)
    {
        if(i<requiredImages)
        {
         if(!isImg)
            tmp=InMemory[framePointer[i]].image();
        }
    }
    else if(n==1)
    {
        if(i<nWorkImages)
         if(!isImg)
            tmp=DoubleWorkImages[i].image();
    }
    else
    {
        if(i<nShortImages)
         if(!isImg)
            tmp=ShortWorkImages[i].image();
    }

    return tmp;
}

int iSeq::setRequired(int n)
{
    if(!isImg)
    {
     requiredImages=n;
     if(has_buffer==1)
     {
      delete[] InMemory;
      delete[] framePointer;
     }
     InMemory=new Image<unsigned char>[requiredImages];
     framePointer=new int[requiredImages];
     reloadImages(thisFrame);
    }
    return 0;
}


int iSeq::reloadImages(unsigned long i)
{
 if(!isImg)
 {
    if(isVid==0 && seqFile->is_open())
    {
        if(i<header.allocatedFrames() && i+1>=requiredImages)
        {
            for(unsigned long o=0; o<requiredImages; o++)
            {
                InMemory[o].loadFromSeq(seqFile, header, i-o);
            }
        }
        else
        {
            i=requiredImages+1;
            for(unsigned long o=0; o<requiredImages; o++)
            {
                InMemory[o].loadFromSeq(seqFile, header, i-o);
            }
        }
        thisFrame=i;
        for(unsigned long i=0; i<requiredImages; i++)
            framePointer[i]=i;
        nextImage=framePointer[requiredImages-1];
    }
    else
    {
        if(i<header.allocatedFrames() && i+1>=requiredImages)
        {
            for(unsigned long o=0; o<requiredImages; o++)
            {
                loadFromVid(InMemory[o], i-o);
                if(have_meta)
                 if(meta.timeIncluded())
                 {
                  vector<int> tv=meta.getT(i-o);
                  if(tv.size()==3)
                  {
                   InMemory[o].setTsec(tv[0]);
                   InMemory[o].setTmsec(tv[1]);
                   InMemory[o].setTusec(tv[2]);
                  }
                 }
            }
        }
        else
        {
            i=requiredImages+1;
            for(unsigned long o=0; o<requiredImages; o++)
            {
                loadFromVid(InMemory[o], i-o);
                if(have_meta)
                 if(meta.timeIncluded())
                 {
                  vector<int> tv=meta.getT(i-o);
                  if(tv.size()==3)
                  {
                   InMemory[o].setTsec(tv[0]);
                   InMemory[o].setTmsec(tv[1]);
                   InMemory[o].setTusec(tv[2]);
                  }
                 }
            }
        }
        thisFrame=i;
        for(unsigned long i=0; i<requiredImages; i++)
            framePointer[i]=i;
        nextImage=framePointer[requiredImages-1];
    
    }
    }
    return 0;
}

string iSeq::filename()
{
    return seqfilename;
}

void iSeq::openBackupFile()
{
    if(!haveBackupFile)
    {
    backupFile=new ofstream;
    string bname=backupName();
    backupFile->open(bname.c_str(),ios::binary|ios::app);
    if(backupFile->is_open())
    {
       haveBackupFile=1;
    }
    else
       cerr<< "Failed to open Backup File for writing\n";
    }
}

void iSeq::backupCurrentPositions(MosquitoesInFrame* currentFrame)
{
    if(haveBackupFile)
        currentFrame->appendToBackup(backupFile);
}

string iSeq::settingsName()
{
    string sname = seqfilename.substr(0, seqfilename.find_last_of("."))+"_pos.yml";
    return sname;
}

string iSeq::backupName()
{

    string bname = seqfilename.substr(0, seqfilename.find_last_of("."))+"_pos.tmp";
    return bname;
}

string iSeq::metaName()
{

    string mname = seqfilename+".metadata";
    return mname;
}

void iSeq::closeBackupFile()
{
    if(haveBackupFile)
    {
        backupFile->close();
        delete backupFile;
        haveBackupFile=0;
    }
    
}

int iSeq::loadMeta()
{
    string mname=metaName();
    ifstream fm(mname.c_str());
    if(fm.good())
    {
        fm.close();
        meta.setInMemory(1);
        meta.load(metaName());
        unsigned int nMeta=meta.getNo();
        if(nMeta==header.allocatedFrames())
        {
           have_meta=1;
           header.setSeqHasMeta(1);
        }    
        else if(isImg)
        {
           cerr<<"Have successfully loaded "<<nMeta<<" for " <<header.allocatedFrames()<<"\n";
           header.allocatedFrames(nMeta);
           have_meta=1;
           header.setSeqHasMeta(1);
        }
        else
        {
           cerr<<"Have unsuccessfully loaded "<<nMeta<<" for " <<header.allocatedFrames()<<"\n";
        }
    }
    else{
        have_meta=0;
    }
    return !have_meta;
}

int iSeq::hasMeta()
{
 return have_meta;
}

int iSeq::metaData(unsigned long i)
{
 int trigger=-1;
 if(have_meta)
 {
  if(i<meta.getNo())
   trigger=meta.getNum(i);
 }
 return trigger;
}

int iSeq::lookUpTrigger(unsigned int i)
{
 int frame=-1;
 if(have_meta)
 {
    frame=meta.getFrameNumberForTrigger(i); 
 }
 return frame;
}

int iSeq::lookUpValidTrigger(unsigned int i)
{
 int frame=-1;
 if(have_meta)
 {
    frame=meta.getFrameNumberForTrigger(i);
    if(frame<0)
      frame=meta.getFrameNumberForTrigger(i+frame);
    if(frame<0)
     frame=0;
 }
 return frame;
}

int iSeq::loadFromVid(Image<unsigned char> &img,unsigned long i)
{
    vid->seek((int64_t)i);
    if(vid->getNext(img))
    {
     cerr<<"Error reading frame "<<i<<"!\n";
     return 1;
    }
    return 0;
}

int iSeq::loadFromVid(Image<unsigned char> &img)
{
    if(vid->getNext(img))
    {
     cerr<<"Error reading frame!\n";
     return 1;
    }
    return 0;
}

string iSeq::posfilename()
{
    return positionFileName;
}

string iSeq::trackfilename()
{
    return trackFileName;
}

void iSeq::setPosfilename(string s)
{
    positionFileName=s;
}

void iSeq::setTrackfilename(string s)
{
    trackFileName=s;
}

bool iSeq::isImage()
{
 return isImg;
}

Rect* iSeq::getRoi()
{
 if(hasRoi)
  return &roi;
 else
  return nullptr;
}

bool iSeq::setRoi(int rx,int ry,int rw,int rh)
{
 if(rw <= 0 || rh <= 0)
 {
  hasRoi=false;
 }
 else
 {
  roi.x=rx;
  roi.y=ry;
  roi.width=rw;
  roi.height=rh;
  hasRoi=true;
 }
 return true;
}

bool iSeq::gotPolyRoi()
{
  return hasPolyRoi;
}

vector<cv::Point> iSeq::getPolyRoi()
{
 if(hasPolyRoi)
  return polyRoi;
 else
  return vector<cv::Point>();
}

bool iSeq::setPolyRoi(vector<cv::Point> poly)
{
 if(poly.size()<=2)
 {
  hasPolyRoi=false;
 }
 else
 {
  polyRoi=poly;
  hasPolyRoi=true;
 }
 return true;
}

//https://stackoverflow.com/questions/20446201/how-to-check-if-string-ends-with-txt
bool iSeq::has_suffix(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}
