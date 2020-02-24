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
#include "ImageComposer.hpp"
#include "Viewer.h"
#include "iSeq.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoesInFrame.hpp"
#include "MosquitoPosition.hpp"
#include "MosquitoRegistry.hpp"
#include <time.h>
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

ImageComposer::ImageComposer()
{
    ready=0;
    vw=0;
    vh=0;
    background=0;
    for(int n=0; n<4; n++)
     for(int m=0; m<4; m++)
      have_positions[n][m]=false;
    for(int n=0; n<4; n++)
     for(int m=0; m<4; m++)
      have_tracks[n][m]=false;
    for(int n=0; n<4; n++)
     for(int m=0; m<4; m++)
      isOpen[n][m]=false;
    minTrigger=0;
    maxTrigger=0;
    mm2px=0;
    for(int n=0; n<4; n++)
    {
      ox[n]=0;
      oy[n]=0;
      wx[n]=0;
      wy[n]=0;
    }
    timeBasedOnSeq=false;
    t0=0;
    dt=0; 
}

Image<unsigned char>* ImageComposer::getImage(int t)
{
 if(ready)
 {
 for(int i=0; i<vw; i++)
  for(int j=0; j<vh; j++)
 {
   if(isOpen[i][j])
 {
   int trigger=seq[i][j]->lookUpTrigger(t);
   int useOld=0;
   if(trigger<0){
    trigger=seq[i][j]->lookUpTrigger(t+trigger);
    useOld=1;
   }
   if(trigger>=0)
   {
    seq[i][j]->loadImage(trigger);   
    Image<unsigned char>* itmp=seq[i][j]->getImagePointer(0);
    if(useOld==0)
    {
     current.setTsec(itmp->getTsec());
     current.setTmsec(itmp->getTmsec());
     current.setTusec(itmp->getTusec());
    }
    int ofx=getOffX(i,j);
    int ofy=getOffY(i,j);
    cerr<<"Inserting image with offset +" << ofx<<"+"<<ofy<<"\n"; 
    current.insert(itmp,ofx,ofy);
   }
 }
 }
 if(timeBasedOnSeq)
 {
     double currT=t0+t*dt;
     long currSec=int(currT);
     unsigned short currMSec=int((currT-currSec)*1000.);
     unsigned short currUSec=int((currT-currSec-currMSec/1000.)*1000000.); 
     current.setTsec(currSec);
     current.setTmsec(currMSec);
     current.setTusec(currUSec);
 }
 }
 else
 {
 cerr<<"Not ready...\n";
 }
 return &current;
}

vector<int> ImageComposer::getTime(int t)
{
 vector<int> timeV;
 if(timeBasedOnSeq)
 {
     double currT=t0+t*dt;
     long currSec=int(currT);
     unsigned short currMSec=int((currT-currSec)*1000.);
     unsigned short currUSec=int((currT-currSec-currMSec/1000.)*1000000.);
     timeV.push_back((int)currSec);
     timeV.push_back((int)currMSec);
     timeV.push_back((int)currUSec);     
 }
 else
 {
  for(int i=0; i<vw; i++)
   for(int j=0; j<vh; j++)
  {
   if(isOpen[i][j])
   {
    int trigger=seq[i][j]->lookUpTrigger(t);
    int useOld=0;
    if(trigger<0){
     trigger=seq[i][j]->lookUpTrigger(t+trigger);
     useOld=1;
    }
    if(trigger>=0)
    {
     timeV = seq[i][j]->timeOfFrameI(trigger);   
    }
   }
  }
 }
 return timeV;
}

int ImageComposer::getOffX(int n, int m)
{
 int ofx=0;
 ofx=ox[n];
 int imageWidth=seq[n][m]->pointerToHeader()->width();

 ofx+=(wx[n]-imageWidth)/2.;
 return ofx;
}

int ImageComposer::getOffY(int n, int m)
{
 int ofy=0;
 ofy=oy[m];
 int imageHeight=seq[n][m]->pointerToHeader()->height();
 ofy+=(wy[m]-imageHeight)/2.;
 return ofy;
}

void ImageComposer::update()
{
  int imageWidth[4][4];
  int imageHeight[4][4];
  for(int i=0; i< vw; i++){
   for(int j=0; j< vh; j++){
    if(isOpen[i][j]){
     imageWidth[i][j]=seq[i][j]->pointerToHeader()->width();
     imageHeight[i][j]=seq[i][j]->pointerToHeader()->height();
    }
    else
    {
     imageWidth[i][j]=0;
     imageHeight[i][j]=0;
    }   
   }
  }
   ox[0]=0;
   oy[0]=0;
   for(int i=0; i< vw; i++)
   {
    int maxW=0;
    for(int j=0; j< vh; j++)
    {
    if(isOpen[i][j])
     if(seq[i][j]->pointerToHeader()->width()>maxW)
      maxW=seq[i][j]->pointerToHeader()->width();
    }
    wx[i]=maxW;
    if(i>0)
    ox[i]=ox[i-1]+wx[i-1];  
   }

   for(int j=0; j< vh; j++)
   {
    int maxH=0;
    for(int i=0; i< vw; i++)
    {
    if(isOpen[i][j])
     if(seq[i][j]->pointerToHeader()->height()>maxH)
     maxH=seq[i][j]->pointerToHeader()->height();
    }
    wy[j]=maxH;
    if(j>0)
    oy[j]=oy[j-1]+wy[j-1];
   }
   width=ox[vw-1]+wx[vw-1];
   height=oy[vh-1]+wy[vh-1];
  
  if(width!=0 && height !=0)
    ready=1;
  else
    ready=0;
  if(ready)
  {
     cerr<<"Creating image of size "<<width<<"x"<<height<<"\n";
     current.create(width,height,background);
  }
  cerr<<"ox=["<<ox[0]<<","<<ox[1]<<","<<ox[2]<<","<<ox[3]<<"]\n";
  cerr<<"oy=["<<oy[0]<<","<<oy[1]<<","<<oy[2]<<","<<oy[3]<<"]\n";
  cerr<<"wx=["<<wx[0]<<","<<wx[1]<<","<<wx[2]<<","<<wx[3]<<"]\n";
  cerr<<"wy=["<<wy[0]<<","<<wy[1]<<","<<wy[2]<<","<<wy[3]<<"]\n";
}

int ImageComposer::setC(int c){
 if(c>0 && c<=4)
 { 
  vw=c;
  update();
 }
  return 0;
}

int ImageComposer::setR(int r){
 if(r>0 && r<=4)
 { 
  vh=r;
  update();
 }
 return 0;
}

int ImageComposer::setSeq(iSeq* s, int c, int r)
{
  if(c<vw && r<vh && c>=0 && r>=0)
  {
    seq[c][r]=s;
    isOpen[c][r]=1;
    update();
  }
  else
    cerr<<"Error: Index out of bounds. You tried to set Seq for ("<<c<<","<<r<<")\n";
  return 0;
}

int ImageComposer::setP(MosquitoesInSeq* p, int c, int r)
{
  if(c<vw && r< vh && c>=0 && r>=0 && isOpen[c][r])
  {
    MosqInSeq[c][r]=p;
    have_positions[c][r]=1;
    updateCombinedPositions();
  }
  else
    cerr<<"Error: Index out of bounds. You tried to set Positions for ("<<c<<","<<r<<")\n";
  return 0;
}

int ImageComposer::setT(MosquitoRegistry* t, int c, int r)
{
  if(c<vw && r< vh && c>=0 && r>=0 && isOpen[c][r])
  {
    MosqReg[c][r]=t;
    have_tracks[c][r]=1;
  }
  else
    cerr<<"Error: Index out of bounds. You tried to set Tracks for ("<<c<<","<<r<<")\n";
  return 0;
}

int ImageComposer::getMaxTrigger()
{
 int mt=-1;
 for(int i=0;i<vw;i++)
  for(int j=0;j<vh;j++)
  {
   if(isOpen[i][j])
    if(seq[i][j]->metaData(seq[i][j]->allocatedFrames()-1)>mt)
     mt=seq[i][j]->metaData(seq[i][j]->allocatedFrames()-1);  
  }
 return mt;
}

int ImageComposer::getMinTrigger()
{
 int mt=-1;
 for(int i=0;i<vw;i++)
  for(int j=0;j<vh;j++)
  {
   if(isOpen[i][j])
    if(mt<0 || seq[i][j]->metaData(0)<mt)
     mt=seq[i][j]->metaData(0);
  }
 return mt;
}

int ImageComposer::updateCombinedPositions()
{
  cerr<<"Updating combined positions...\n";
  minTrigger=getMinTrigger();
  maxTrigger=getMaxTrigger();
  cerr<<"Currently we have Trigger values from "<<minTrigger <<" to "<< maxTrigger<<"\n";
  if(maxTrigger!=-1){
  currentPositions.setNoOfFrames(maxTrigger+1);
  if(seq[0][0])currentPositions.giveSeqPointer(seq[0][0]);
  for(int t=minTrigger; t<=maxTrigger; t++)
  {
   MosquitoesInFrame* opos=currentPositions.mosqInFrameI(t);
   for(int i=0;i<vw;i++)
    for(int j=0;j<vh;j++)
     if(isOpen[i][j])
   {   
     int n=seq[i][j]->lookUpTrigger(t);
     if(n>=0)
     {
      MosquitoesInFrame* ipos=MosqInSeq[i][j]->mosqInFrameI(n);
      int ofx=getOffX(i,j);
      int ofy=getOffY(i,j);
      for(int m=0; m<ipos->numOfMosquitoes(); m++)
      {
       MosquitoPosition* imosquito=ipos->mosqi(m);
       MosquitoPosition mosquito=*imosquito;
       double xoff=0;
       double yoff=0;
       double px,py,pz;
       if(mosquito.MosquitoHasContour())
       {
        vector<cv::Point> c=mosquito.getContour();
        for(int ic=0; ic<c.size();ic++)
        {
          bool ret2=false;
          double xoff2=0;
          double yoff2=0;
          c[ic].x=c[ic].x+ofx+xoff2;
          c[ic].y=c[ic].y+ofy+yoff2;
        }
        mosquito.setContour(c,true);
       }
       opos->addMosquito(mosquito,ofx+xoff,ofy+yoff);
       
      }
      if(opos->getTsSec()==0)
      {
       opos->setTsSec(ipos->getTsSec());
       opos->setTsMSec(ipos->getTsMSec());
       opos->setTsUSec(ipos->getTsUSec());
      }
      if(opos->getF()==0) 
      {
       opos->setFrameNo(t);
      }
      if(opos->getMaxDiff()<ipos->getMaxDiff())
       opos->setMaxDiff(ipos->getMaxDiff());
     }
   }
  }
  }
  else
  {
    cerr<< "Got invalid meta data... \n";
  }
  return 0;
}

MosquitoesInSeq* ImageComposer::currMosqPos()
{
 return &currentPositions;
}

bool ImageComposer::haveImage(int t)
{
 bool haveOne=false;
 if(ready)
 {
 for(int i=0; i<vw; i++)
  for(int j=0; j<vh; j++)
   if(isOpen[i][j])
 {
   int n=seq[i][j]->lookUpTrigger(t);
   if(n>=0){ haveOne=true;break;}
 }
 }
 return haveOne;
}

MosquitoRegistry* ImageComposer::currTracks()
{
 return &currentTracks;
}

bool ImageComposer::setTimeBasedOnCurrentSeq(int nnn/*=0*/, int mmm /*=0*/, int takeN /*=100*/)
{
  if(seq[nnn][mmm])
  {
    t0=0;
    dt=1/50;
    vector<double> timePoint;
    vector<int> trigger;
    for(unsigned int q=0; q<takeN; q++)
    {
     vector<int> t=seq[nnn][mmm]->timeOfFrameI(q);
     int triggerNo = seq[nnn][mmm]->metaData(q);
     if(t.size()==3)
     {
      double tt=t[0]+t[1]/1000.+t[2]/1000000.;     
      timePoint.push_back(tt);
      trigger.push_back(triggerNo);
     }
    }
    if(trigger.size()>0)
    {
    std::vector<std::vector<double> > Avec;
    for(int nn=0; nn<timePoint.size(); nn++)
    {
     vector<double> tmp;
     tmp.push_back(trigger[nn]);
     tmp.push_back(1);
     Avec.push_back(tmp);                  
    }
    cv::Mat A(Avec.size(), Avec.at(0).size(), CV_64FC1);
    for(int ii=0; ii<A.rows; ++ii)
     for(int jj=0; jj<A.cols; ++jj)
      A.at<double>(ii, jj) = Avec.at(ii).at(jj);
    cv::Mat B=cv::Mat(timePoint);
    cv::Mat ab;
    cv::solve(A, B, ab, cv::DECOMP_SVD);
    dt=ab.at<double>(0);
    t0=ab.at<double>(1);
    cerr<<"Time estimated as:\n time(trig)="<<dt<<"*trig+"<<t0<<"\n";
    timeBasedOnSeq=true;
    for(int t=0; t<=maxTrigger; t++)
    {
     MosquitoesInFrame* opos=currentPositions.mosqInFrameI(t);
     if(opos)
     {
      double currT=t0+t*dt;
      long currSec=int(currT);
      unsigned short currMSec=int((currT-currSec)*1000.);
      unsigned short currUSec=int((currT-currSec-currMSec/1000.)*1000000.); 
      opos->setTsSec(currSec);
      opos->setTsMSec(currMSec);
      opos->setTsUSec(currUSec);
     }
     else
      cerr<<"Don't have postions for trigger "<< t <<"\n";
    }
    }
    else
    {
     return false;
    }
    return true;
  }
  else
   return false;
}
