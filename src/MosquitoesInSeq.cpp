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
#include "MosquitoesInSeq.hpp"
#include "MosquitoesInFrame.hpp"
#include "MosquitoPosition.hpp"
#include <vector>
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
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include "iSeq.hpp"
#include <QPointF>

//for sorting...
#include <algorithm>
#include <iterator>

//dialog box?   
#include <QProgressDialog>
#include <QMessageBox>

using namespace std;

MosquitoesInSeq::MosquitoesInSeq()
{
    initialized=0;
    nFrames=0;
    haveSeq=0;
    dynamic=false;
    triggerBased=false;
}

MosquitoesInSeq::MosquitoesInSeq(MosquitoesInSeq&& other)
{
    mframes=other.mframes;
    nFrames=other.nFrames;
    haveSeq=other.haveSeq;
    currSeq=other.currSeq;
    initialized=other.initialized;

    if(other.initialized)
    {
     vector<MosquitoesInFrame>().swap(mframes);  
     nFrames=0;
     other.initialized=0;
    }
    dynamic=other.dynamic;
    triggerBased=other.triggerBased;
    FrameLookup=other.FrameLookup;
}
MosquitoesInSeq::~MosquitoesInSeq()
{
}
MosquitoesInSeq& MosquitoesInSeq::operator=(MosquitoesInSeq&& other)
{
    if(this!=&other)
    {
    mframes=other.mframes;
    nFrames=other.nFrames;
    initialized=other.initialized;
    haveSeq=other.haveSeq;
    currSeq=other.currSeq;
    if(other.initialized)
    {
     vector<MosquitoesInFrame>().swap(other.mframes);  
     other.nFrames=0;
     other.initialized=0;
    }
    }
    dynamic=other.dynamic;
    triggerBased=other.triggerBased;
    FrameLookup=other.FrameLookup;
    return *this;
}

MosquitoesInSeq& MosquitoesInSeq::operator=(MosquitoesInSeq& other)
{
    initialized=other.initialized;
    nFrames=other.nFrames;
    if(initialized)
    {
    vector<MosquitoesInFrame>().swap(mframes);  
    for(unsigned long i=0; i<nFrames; i++)
      mframes.push_back(other.mframes[i]);
    }
    haveSeq=other.haveSeq;
    currSeq=other.currSeq;
    dynamic=other.dynamic;
    triggerBased=other.triggerBased;
    FrameLookup=other.FrameLookup;
    return *this;
}

MosquitoesInSeq::MosquitoesInSeq(const MosquitoesInSeq& a)
{
    initialized=a.initialized;
    nFrames=a.nFrames;
    haveSeq=a.haveSeq;
    currSeq=a.currSeq;

    vector<MosquitoesInFrame>().swap(mframes);  
    
    for(unsigned long i=0; i<nFrames; i++)
      mframes.push_back(a.mframes[i]);
    dynamic=a.dynamic;
    triggerBased=a.triggerBased;
    FrameLookup=a.FrameLookup;
}

int MosquitoesInSeq::setNoOfFrames(unsigned long frames)
{
    if(!initialized)
    {
        initialized=1;
        nFrames=frames;
        vector<MosquitoesInFrame>().swap(mframes);  
        for(unsigned long n=0;n<nFrames; n++)
        {
         mframes.push_back(MosquitoesInFrame());
         mframes[n].setFrameNo(n);
        }
    }
    else
    {
        vector<MosquitoesInFrame>().swap(mframes);  
        initialized=1;
        nFrames=frames;
        for(unsigned long n=0;n<nFrames; n++)
        {
         mframes.push_back(MosquitoesInFrame());
         mframes[n].setFrameNo(n);
        }

    }
    return 0;
}

MosquitoesInFrame* MosquitoesInSeq::mosqInFrameI(unsigned long i, bool directAccess /*= false */,bool isTrigger /* = false */)
{
    MosquitoesInFrame* pointer=nullptr;
    if(directAccess || dynamic)
    {
     if(dynamic)
     {
      long wf=findTrigger(i);
      if(wf!=-1) i=wf;
      else
       return nullptr;
     }
     if(i<mframes.size())
     {
      pointer=&mframes[i];
      return pointer;
     }
     else
     {
      return pointer;    
     } 
    }
    if(triggerBased)
    {
     if(!isTrigger)
     {
        long tmp=getTriggerFromSeq(i);
        if(tmp>0)
        {
         long wf=findTrigger(tmp);
         if(wf!=-1)
          pointer=&mframes[wf];
        }
     }     
     else
     {
     long wf=findTrigger(i);
     if(wf!=-1)
        pointer=&mframes[wf];
     }
    }
    else
    {
    if(i<nFrames)
        pointer=&mframes[i];
    }
    return pointer;
}

vector<cv::Point2f> MosquitoesInSeq::getAllPos(unsigned long i, bool isTrigger /*= false*/)
{
    vector<cv::Point2f> tmp;
    MosquitoesInFrame* currentFrame=nullptr;
    if(!dynamic)
    {
    if(i<nFrames)
    {
        currentFrame=&mframes[i];
        MosquitoPosition* currentMosquito=nullptr;
        for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
        {
            currentMosquito=currentFrame->mosqi(ii);
            cv::Point2f m(currentMosquito->getX(),currentMosquito->getY());
            tmp.push_back(m);           
        }
    }
    }
    else
    {
     long t=i;
     if(!isTrigger)
     {
      t=getTriggerFromSeq(i);
     }
     if(t>0)
     {
      long wfi=findTrigger(t);
      if(wfi!=-1)
      {
        currentFrame=&mframes[wfi];
        MosquitoPosition* currentMosquito=nullptr;
        for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
        {
            currentMosquito=currentFrame->mosqi(ii);
            cv::Point2f m(currentMosquito->getX(),currentMosquito->getY());
            tmp.push_back(m);
        }
      }
     }
    }
    return tmp;
}

vector<cv::Point2f> MosquitoesInSeq::getAllPos(unsigned long n, unsigned long m, bool isTrigger /*= false*/)
{
    vector<cv::Point2f> tmp;
    MosquitoesInFrame* currentFrame=nullptr;
    if(!dynamic)
    {
    if(m>nFrames)m=nFrames;
    for(unsigned long i=n; i<m;i++)
    {
    if(i<nFrames)
    {
        currentFrame=&mframes[i];
        MosquitoPosition* currentMosquito=nullptr;
        for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
        {
            currentMosquito=currentFrame->mosqi(ii);
            cv::Point2f m(currentMosquito->getX(),currentMosquito->getY());
            tmp.push_back(m);
        }
    }
    }
    }
    else
    {
     long tmin=n;
     long tmax=m;
     if(!isTrigger)
     {
      tmin=getTriggerFromSeq(n);
      tmax=getTriggerFromSeq(m);
     }
     if(tmin>0 && tmax>0 && tmax>tmin)
     {
      long wfi=findTriggerNearest(tmin);
      long wfj=findTriggerNearest(tmax,false);
      if(wfi!=-1)
      {
       if(wfj==-1) wfj=wfi+1;
       for(unsigned long i=wfi; i<wfj;i++)
       {
        currentFrame=&mframes[i];
        MosquitoPosition* currentMosquito=nullptr;
        for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
        {
            currentMosquito=currentFrame->mosqi(ii);
            cv::Point2f m(currentMosquito->getX(),currentMosquito->getY());
            tmp.push_back(m);
        }

       }
      }
     }
    }
    return tmp;
}

vector<double> MosquitoesInSeq::getPointMeta(unsigned long n, unsigned long m, bool isTrigger /* =false */)
{
    vector<double> tmp;
    MosquitoesInFrame* currentFrame=nullptr;
    if(!dynamic)
    {
    if(m>nFrames)m=nFrames;
    for(unsigned long i=n; i<m;i++)
    {
    if(i<nFrames)
    {
        currentFrame=&mframes[i];
        for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
        {
           MosquitoPosition* currentMosquito=nullptr;
           currentMosquito=currentFrame->mosqi(ii);
           tmp.push_back((i-n)/double(m-n));
        }        
    }
    }
    }
    else
    {
     long tmin=n;
     long tmax=m;
     if(!isTrigger)
     {
      tmin=getTriggerFromSeq(n);
      tmax=getTriggerFromSeq(m);
     }
     if(tmin>0 && tmax>0 && tmax>tmin)
     {
      long wfi=findTriggerNearest(tmin);
      long wfj=findTriggerNearest(tmax,false);
      if(wfi!=-1)
      {
       if(wfj==-1) wfj=wfi+1;
       for(unsigned long i=wfi; i<wfj;i++)
       {
         currentFrame=&mframes[i];
         for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
         {
           MosquitoPosition* currentMosquito=nullptr;
           currentMosquito=currentFrame->mosqi(ii);
           tmp.push_back((FrameLookup[i]-tmin)/double(tmax-tmin));
         }        
       }
      }
     }
    }
    return tmp;
}

vector<double> MosquitoesInSeq::getPointMeta(unsigned long n, bool isTrigger /*= false */)
{
    vector<double> tmp;
    MosquitoesInFrame* currentFrame=nullptr;
    if(!dynamic)
    {
    if(n<nFrames)
    {
        currentFrame=&mframes[n];
        MosquitoPosition* currentMosquito;
        for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
        {
            currentMosquito=nullptr;
            currentMosquito=currentFrame->mosqi(ii);
            tmp.push_back(0.5);
        }
    }
    }
    else
    {
     long t=n;
     if(!isTrigger)
     {
      t=getTriggerFromSeq(n);
     }
     if(t>0)
     {
      long wfi=findTrigger(t);
      if(wfi!=-1)
      {
        currentFrame=&mframes[wfi];
        MosquitoPosition* currentMosquito;
        for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
        {
            currentMosquito=nullptr;
            currentMosquito=currentFrame->mosqi(ii);
            tmp.push_back(0.5);
        }
      }
     }
    }
    return tmp;
}

int  MosquitoesInSeq::getNumOfAllPos()
{
    int tmp=0;
    for(unsigned long i=0;i<mframes.size();i++)
    {
        tmp += mframes[i].numOfMosquitoes();
    }
    return tmp;
}

vector<vector<double> > MosquitoesInSeq::getAllPosAsVector(unsigned long i, int select /*=0*/, bool isTrigger /*= false */)
{
    vector<vector<double> > tmp;
    MosquitoesInFrame* currentFrame=nullptr;
    if(!dynamic)
    {
    if(i<nFrames)
    {
        currentFrame=&mframes[i];
        MosquitoPosition* currentMosquito=nullptr;
        for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
        {
            currentMosquito=currentFrame->mosqi(ii);
            if(currentMosquito->getFlag() & select)
            {
            vector<double> tmp2;
            tmp2.push_back(currentMosquito->getX());
            tmp2.push_back(currentMosquito->getY());
            tmp2.push_back((double)i);
            tmp2.push_back((double)currentFrame->getTsSec());            
            tmp2.push_back((double)currentFrame->getTsMSec());            
            tmp2.push_back((double)currentFrame->getTsUSec()); 
            tmp2.push_back(currentMosquito->getI());             
            tmp2.push_back(currentMosquito->getA());
            tmp2.push_back(currentFrame->getMaxDiff());
            tmp2.push_back(currentFrame->getMinArea());
            tmp2.push_back(currentFrame->getMaxArea());
            tmp2.push_back(currentFrame->getThreshold());
            tmp2.push_back(currentFrame->getMinThreshold());
            tmp2.push_back((double)currentFrame->getIth());
            tmp.push_back(tmp2);
            }
        }
    }
    }
    else
    {
     long t=i;
     if(!isTrigger)
     {
      t=getTriggerFromSeq(i);
     }
     if(t>0)
     {
      long wfi=findTrigger(t);
      if(wfi!=-1)
      {
        currentFrame=&mframes[wfi];
        MosquitoPosition* currentMosquito=nullptr;
        for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
        {
            currentMosquito=currentFrame->mosqi(ii);
            if(currentMosquito->getFlag() & select)
            {
            vector<double> tmp2;
            tmp2.push_back(currentMosquito->getX());
            tmp2.push_back(currentMosquito->getY());
            tmp2.push_back((double)currentFrame->getF());
            tmp2.push_back((double)currentFrame->getTsSec());            
            tmp2.push_back((double)currentFrame->getTsMSec());            
            tmp2.push_back((double)currentFrame->getTsUSec()); 
            tmp2.push_back(currentMosquito->getI());             
            tmp2.push_back(currentMosquito->getA());
            tmp2.push_back(currentFrame->getMaxDiff());
            tmp2.push_back(currentFrame->getMinArea());
            tmp2.push_back(currentFrame->getMaxArea());
            tmp2.push_back(currentFrame->getThreshold());
            tmp2.push_back(currentFrame->getMinThreshold());
            tmp2.push_back((double)currentFrame->getIth());
            tmp.push_back(tmp2);
            }
        }
      }
     }
    }
    return tmp;
}

int MosquitoesInSeq::saveToFile(string filename,int saveFrom /*=-1*/, int saveTo /*=-1*/)
{
    FILE * pFile;
    if(!( access( filename.c_str(), F_OK ) != -1 ))
    {
        pFile = fopen (filename.c_str(),"w");
        if(pFile == NULL)
        {
            QMessageBox msgBox;
            msgBox.setText(QString("Error: File already exists, choose another file...\n"));
            msgBox.exec();
            cerr << "Error file "<<filename<< " does exists!"<<endl;
        }
        else
        {
            MosquitoesInFrame* currentFrame=nullptr;
            MosquitoPosition* currentMosquito=nullptr;
            if(dynamic)
            {
              long wfa=findTriggerNearest(saveFrom,true);
              long wfb=findTriggerNearest(saveTo,false);
              if(wfa!=-1) saveFrom=wfa; 
              if(wfb!=-1) saveTo=wfb; 
            }
            if(saveFrom==-1) saveFrom=0;
            if(saveTo==-1) saveTo=nFrames;
            if(dynamic)
            {
              long wfa=findTriggerNearest(saveFrom,true);
              long wfb=findTriggerNearest(saveTo,false);
              if(wfa!=-1) saveFrom=wfa; 
              if(wfb!=-1) saveTo=wfb; 
            }
            if(saveTo<saveFrom) saveTo=saveFrom;
            for(unsigned long i=saveFrom; i<saveTo; i++)
            {
                currentFrame=&mframes[i];
                for(int j=0; j<currentFrame->numOfMosquitoes(); j++)
                {
                    currentMosquito=currentFrame->mosqi(j);
                    fprintf(pFile,"%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\n",
                            currentMosquito->getX(),
                            currentMosquito->getY(),
                            currentMosquito->getI(),
                            currentMosquito->getA(),
                            currentFrame->getMaxDiff(),
                            static_cast<double>(currentFrame->getF()),
                            static_cast<double>(currentFrame->getTsSec()),
                            static_cast<double>(currentFrame->getTsMSec()),
                            static_cast<double>(currentFrame->getTsUSec())
                            );
                     if(currentMosquito->MosquitoHasContour())
                     {
                      vector<cv::Point> c=currentMosquito->getContour();
                      fprintf(pFile,"#contour %u",c.size());
                      for(int cc=0; cc<c.size(); cc++)
                       fprintf(pFile," %i %i",c[cc].x,c[cc].y);
                      fprintf(pFile,"\n");
                     }
                }
            }
            fclose(pFile);
        }
    }
    return 0;
}

int MosquitoesInSeq::loadFromFile(string filename, bool enableDynamic /* = false */, bool displayStatus /* =true */)
{
    if(enableDynamic && !dynamic)
    { 
     vector<MosquitoesInFrame>().swap(mframes);
     dynamic=true;
     triggerBased=true;
    }
        
    int counter=0;
    int lastFrame=0;
    FILE * pFile;
    MosquitoPosition* lastMosi=nullptr;
    pFile = fopen (filename.c_str(),"r");
    fseek( pFile, 0, SEEK_END );
    int Size = ftell( pFile );
    rewind( pFile );
    bool haveDialog=false;
    QProgressDialog* DisplayProgress;
    if(displayStatus)
    {
     DisplayProgress=new QProgressDialog("Loding Positions...", "Cancel", 0, Size);
     haveDialog=true;
     DisplayProgress->setWindowModality(Qt::WindowModal);
    }
    double rx,ry,ri,ra,rd,rf,rs,rm,ru;
    char line[4096];
    while(fgets(line, sizeof(line), pFile))
    {
        if(strncmp(line, "#contour", 8) == 0)
        {
        if(lastMosi)
        {
         char *data = line;
         int offset;
         int n;
         char word[100];
         int ret=sscanf(data, "%s %i%n", word, &n, &offset);
         if(ret == 2)
         {
         vector<cv::Point> c;
         ret=0;
         data+=offset;
         int x;
         int y;
         for(int i=0;i< n; i++)
         {
          if(sscanf(data, "%i %i%n", &x, &y, &offset) == 2)
          {
           c.push_back(cv::Point(x,y));
           data+=offset;          
          }
          else
          {
           ret=-1;
           break;
          }
         }
         if(ret==0)
         {
          lastMosi->setContour(c);
         }
         }
        }
        }
        else
        {
        int fields=sscanf(line,"%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n",
                 &rx,
                 &ry,
                 &ri,
                 &ra,
                 &rd,
                 &rf,
                 &rs,
                 &rm,
                 &ru);
    if(fields==9){
        if((unsigned long)rf<nFrames || dynamic)
        {
            if(!dynamic)
            {
            mframes[(unsigned long)rf].setTsSec((int)rs);
            mframes[(unsigned long)rf].setTsMSec((unsigned short)rm);
            mframes[(unsigned long)rf].setTsUSec((unsigned short)ru);
            mframes[(unsigned long)rf].setFrameNo((unsigned long)rf);
            mframes[(unsigned long)rf].setMaxDiff(rd);
            mframes[(unsigned long)rf].addMosquito(rx,ry,ra,ri);
            }
            else
            {
             long wf=findTrigger((unsigned long)rf);
             if(wf==-1)
             {
              mframes.push_back(MosquitoesInFrame());
              FrameLookup.push_back((unsigned long)rf);
              wf=mframes.size()-1;
             }
             mframes[wf].setTsSec((int)rs);
             mframes[wf].setTsMSec((unsigned short)rm);
             mframes[wf].setTsUSec((unsigned short)ru);
             mframes[wf].setFrameNo((unsigned long)rf);
             mframes[wf].setMaxDiff(rd);
             mframes[wf].addMosquito(rx,ry,ra,ri);
            }
            counter++;
            if(lastFrame<rf)
             lastFrame=rf;
        }
    }
    else
    {
        cerr<<"Error reading Position file! " <<fields<<" fields but I need 9!"<<endl;
    }
    }
    int cSize = ftell( pFile );
    if(displayStatus)
    {
     DisplayProgress->setValue(cSize);
     if (DisplayProgress->wasCanceled())
     {
       DisplayProgress->setValue(Size);
       delete DisplayProgress;
       haveDialog=false;
       break;
     }
    } 
    }
    if(haveDialog)
    {
      DisplayProgress->setValue(Size);
      delete DisplayProgress;
      haveDialog=false;    
    }
    fclose(pFile);
    if(dynamic)
     sortFrames();
    return rf;
}

int MosquitoesInSeq::loadFromFile(string filename, unsigned long rstart, unsigned long rstop , bool enableDynamic /* = false */, bool displayStatus /* =true */ )
{
    if(enableDynamic && !dynamic)
    { 
     vector<MosquitoesInFrame>().swap(mframes);
     dynamic=true;
     triggerBased=true;
    }
    bool loadedLastMosq=false;    
    int counter=0;
    int lastFrame=0;
    FILE * pFile;
    MosquitoPosition* lastMosi=nullptr;
    pFile = fopen (filename.c_str(),"r");
    fseek( pFile, 0, SEEK_END );
    int Size = ftell( pFile );
    rewind( pFile );
    bool haveDialog=false;
    QProgressDialog* DisplayProgress;
    if(displayStatus)
    {
     DisplayProgress=new QProgressDialog("Loding Positions...", "Cancel", 0, Size);
     haveDialog=true;
     DisplayProgress->setWindowModality(Qt::WindowModal);
    }
    double rx,ry,ri,ra,rd,rf,rs,rm,ru;
    char line[4096];
    while(fgets(line, sizeof(line), pFile))
    {
        if(strncmp(line, "#contour", 8) == 0)
        {
        if(lastMosi && loadedLastMosq)
        {
         char *data = line;
         int offset;
         int n;
         char word[100];
         int ret=sscanf(data, "%s %i%n", word, &n, &offset);
         if(ret == 2)
         {
         vector<cv::Point> c;
         ret=0;
         data+=offset;
         int x;
         int y;
         for(int i=0;i< n; i++)
         {
          if(sscanf(data, "%i %i%n", &x, &y, &offset) == 2)
          {
           c.push_back(cv::Point(x,y));
           data+=offset;          
          }
          else
          {
           ret=-1;
           break;
          }
         }
         if(ret==0)
         {
          lastMosi->setContour(c);
         }
         }
        }
        }
        else
        {
        int fields=sscanf(line,"%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n",
                 &rx,
                 &ry,
                 &ri,
                 &ra,
                 &rd,
                 &rf,
                 &rs,
                 &rm,
                 &ru);
    if(fields==9){
        if(((unsigned long)rf<nFrames || dynamic) && ( (unsigned long)rf>=rstart && (unsigned long)rf<rstop))
        {
            if(!dynamic)
            {
            mframes[(unsigned long)rf].setTsSec((int)rs);
            mframes[(unsigned long)rf].setTsMSec((unsigned short)rm);
            mframes[(unsigned long)rf].setTsUSec((unsigned short)ru);
            mframes[(unsigned long)rf].setFrameNo((unsigned long)rf);
            mframes[(unsigned long)rf].setMaxDiff(rd);
            mframes[(unsigned long)rf].addMosquito(rx,ry,ra,ri);
            }
            else
            {
             long wf=findTrigger((unsigned long)rf);
             if(wf==-1)
             {
              mframes.push_back(MosquitoesInFrame());
              FrameLookup.push_back((unsigned long)rf);
              wf=mframes.size()-1;
             }
             mframes[wf].setTsSec((int)rs);
             mframes[wf].setTsMSec((unsigned short)rm);
             mframes[wf].setTsUSec((unsigned short)ru);
             mframes[wf].setFrameNo((unsigned long)rf);
             mframes[wf].setMaxDiff(rd);
             mframes[wf].addMosquito(rx,ry,ra,ri);
            }
            counter++;
            if(lastFrame<rf)
             lastFrame=rf;
            loadedLastMosq=true;
        }
        else
        {
            loadedLastMosq=false;
        }
    }
    else
    {
        cerr<<"Error reading Position file! " <<fields<<" fields but I need 9/10 or 13!"<<endl;
        loadedLastMosq=false;

    }
    }
    int cSize = ftell( pFile );
    if(displayStatus)
    {
     DisplayProgress->setValue(cSize);
     if (DisplayProgress->wasCanceled())
     {
       DisplayProgress->setValue(Size);
       delete DisplayProgress;
       haveDialog=false;
       break;
     }
    } 
    }
    if(haveDialog)
    {
      DisplayProgress->setValue(Size);
      delete DisplayProgress;
      haveDialog=false;    
    }
    fclose(pFile);
    if(dynamic)
     sortFrames();
     
    return rf;
}

int MosquitoesInSeq::MosqsInFrame(unsigned long f, bool isTrigger /* = false */)
{
    if(!dynamic)
    {
    if(f<nFrames && mframes.size()>0)
        return mframes[f].numOfMosquitoes();
    else
        return 0;
    }
    else
    {
     long t=f;
     if(!isTrigger)
     {
      t=getTriggerFromSeq(f);
     }
     if(t>0)
     {
      long wfi=findTrigger(t);
      if(wfi!=-1)
      {
        return mframes[wfi].numOfMosquitoes();
      }
      else
       return 0;
     }
     else
      return 0;
    }
}

int MosquitoesInSeq::clean(unsigned long i, double minDist, bool isTrigger /*= false*/)
{
    if(triggerBased)
    {
     long tmp=i;
     if(!isTrigger)
     {
      tmp=getTriggerFromSeq(i);
     }
     if(tmp>0)
     {
      long wf=findTrigger(tmp);
      if(wf!=-1)
      {
       return mframes[wf].clean(minDist);
      }
      else
       return 0;
     }
    }
    else
    {
    if(i<nFrames)
    {
     return mframes[i].clean(minDist);
    } 
    else
     return 0;
    }
}

int MosquitoesInSeq::clean(unsigned long i, double minArea, double maxArea, bool isTrigger /*= false*/)
{
    if(triggerBased)
    {
     long tmp=i;
     if(!isTrigger)
     {
       tmp=getTriggerFromSeq(i);
     }
     if(tmp>0)
     {
      long wf=findTrigger(tmp);
      if(wf!=-1)
      {
         return mframes[wf].clean(minArea,maxArea);
      }
      else
       return 0;
     }
    }
    else
    { 
    if(i<nFrames)
    {
         return mframes[i].clean(minArea,maxArea);
    }
    else
     return 0;
    }
}

int MosquitoesInSeq::giveSeqPointer(iSeq* iseq)
{
   currSeq=iseq;
   haveSeq=1;
   return 0;
}

int MosquitoesInSeq::getTsSec(unsigned long i, bool isTrigger /* = false */)
{
  int ts=0;
  if(!dynamic)
  {
   long tr=i;
   if(!isTrigger)
   {
    if(triggerBased)
    {
     long t=getTriggerFromSeq(tr);
     long wfi=findTrigger(t);
     if(wfi!=-1)
     {
      tr=wfi;
     }
     else
     {
      return -1;
     }
    }
    if(tr<nFrames && tr>=0)
    {
     if(mframes[tr].numOfMosquitoes()!=0)
      ts=mframes[tr].getTsSec();
     else
     {
      ts=mframes[tr].getTsSec();
      if(ts==0 && haveSeq)
      {
       vector<int> t=currSeq->timeOfFrameI(tr);
       if(t.size()==3)
       {
         mframes[tr].setTsSec(t[0]);
         mframes[tr].setTsMSec((unsigned short) t[1]);
         mframes[tr].setTsUSec((unsigned short) t[2]);
         ts=t[0];
       }
      }    
     }
    }
   }
   else
   {
    if(triggerBased)
    {
     long wfi=findTrigger(tr);
     if(wfi!=-1)
     {
      tr=wfi;
     }
     else
     {
      return -1;
     } 
    }
    else
    {
     if(haveSeq)
     {
      if(currSeq->hasMeta())
      {
       int tmp=currSeq->lookUpTrigger(tr);
       if(tmp!=-1)
        tr=tmp;
       else
       {
        cerr<<"I can't lookup the trigger, lookup failed...\n";
        return -1;             
       }
      }
      else
      {
       cerr<<"I can't lookup the trigger, seq does not have metadata...\n";
       return -1;      
      }
     }
     else
     {
      cerr<<"I can't lookup the trigger, I don't have a seq to search...\n";
      return -1;
     }
    }
    if(tr<nFrames && tr>=0)
    {
     if(mframes[tr].numOfMosquitoes()!=0)
      ts=mframes[tr].getTsSec();
     else
     {
      ts=mframes[tr].getTsSec();
      if(ts==0 && haveSeq)
      {
       vector<int> t=currSeq->timeOfFrameI(tr);
       if(t.size()==3)
       {
         mframes[tr].setTsSec(t[0]);
         mframes[tr].setTsMSec((unsigned short) t[1]);
         mframes[tr].setTsUSec((unsigned short) t[2]);
         ts=t[0];
       }
      }    
     }
    } 
   }
  }
  else
  {
   long tr=i;
   if(!isTrigger)
   {
    long tmp=getTriggerFromSeq(tr);
    if(tmp!=-1)
    {
     tr=tmp;
    }
    else
    {
     cerr<<"Could not translate the frame to a trigger...\n";
     return -1;
    }
   }
   if(tr>0)
   {
    long wfi=findTrigger(tr);
    if(wfi!=-1)
    {
    if(mframes[wfi].numOfMosquitoes()!=0)
      ts=mframes[wfi].getTsSec();
    else
    {
      ts=mframes[wfi].getTsSec();
      if(ts==0 && haveSeq)
      {
        int fn=currSeq->lookUpTrigger(tr);
        if(fn!=-1)
        {
        vector<int> t=currSeq->timeOfFrameI(fn);
        if(t.size()==3)
        {
          mframes[wfi].setTsSec(t[0]);
          mframes[wfi].setTsMSec((unsigned short) t[1]);
          mframes[wfi].setTsUSec((unsigned short) t[2]);
          ts=t[0];
        }
        }
      }    
     }
    }
   }
  }
  return ts;
}


unsigned short MosquitoesInSeq::getTsMSec(unsigned long i, bool isTrigger /* = false */)
{
  unsigned short tsm=0;
  if(!dynamic)
  {
   long tr=i;
   if(!isTrigger)
   {
    if(triggerBased)
    {
     long t=getTriggerFromSeq(tr);
     long wfi=findTrigger(t);
     if(wfi!=-1)
     {
      tr=wfi;
     }
     else
     {
      return 0;
     }
    }
    if(tr<nFrames && tr>=0)
    {
     if(mframes[tr].numOfMosquitoes()!=0)
      tsm=mframes[tr].getTsMSec();
     else
     {
      tsm=mframes[tr].getTsMSec();
      int ts=mframes[tr].getTsSec();
      if(ts==0 && haveSeq)
      {
       vector<int> t=currSeq->timeOfFrameI(tr);
       if(t.size()==3)
       {
        mframes[tr].setTsSec(t[0]);
        mframes[tr].setTsMSec((unsigned short) t[1]);
        mframes[tr].setTsUSec((unsigned short) t[2]);
        tsm=t[1];
       }
      }    
     }
    }
   }
   else
   {
    if(triggerBased)
    {
     long wfi=findTrigger(tr);
     if(wfi!=-1)
     {
      tr=wfi;
     }
     else
     {
      return 0;
     } 
    }
    else
    {
     if(haveSeq)
     {
      if(currSeq->hasMeta())
      {
       int tmp=currSeq->lookUpTrigger(tr);
       if(tmp!=-1)
        tr=tmp;
       else
       {
        cerr<<"I can't lookup the trigger, lookup failed...\n";
        return 0;             
       }
      }
      else
      {
       cerr<<"I can't lookup the trigger, seq does not have metadata...\n";
       return 0;      
      }
     }
     else
     {
      cerr<<"I can't lookup the trigger, I don't have a seq to search...\n";
      return 0;
     }
    }
    if(tr<nFrames && tr>=0)
    {
     if(mframes[tr].numOfMosquitoes()!=0)
      tsm=mframes[tr].getTsMSec();
     else
     {
      tsm=mframes[tr].getTsMSec();
      int ts=mframes[tr].getTsSec();
      if(ts==0 && haveSeq)
      {
       vector<int> t=currSeq->timeOfFrameI(tr);
       if(t.size()==3)
       {
        mframes[tr].setTsSec(t[0]);
        mframes[tr].setTsMSec((unsigned short) t[1]);
        mframes[tr].setTsUSec((unsigned short) t[2]);
        tsm=t[1];
       }
      }    
     }
    }
   }
  }
  else
  {
   long tr=i;
   if(!isTrigger)
   {
    long tmp=getTriggerFromSeq(tr);
    if(tmp!=-1)
    {
     tr=tmp;
    }
    else
    {
     cerr<<"Could not translate the frame to a trigger...\n";
     return 0;
    }
   }
   if(tr>0)
   {
    long wfi=findTrigger(tr);
    if(wfi!=-1)
    {
    if(mframes[wfi].numOfMosquitoes()!=0)
      tsm=mframes[wfi].getTsMSec();
    else
    {
      tsm=mframes[wfi].getTsMSec();
      int ts=mframes[wfi].getTsSec();
      if(ts==0 && haveSeq)
      {
        int fn=currSeq->lookUpTrigger(tr);
        if(fn!=-1)
        {
        vector<int> t=currSeq->timeOfFrameI(fn);
        if(t.size()==3)
        {
          mframes[wfi].setTsSec(t[0]);
          mframes[wfi].setTsMSec((unsigned short) t[1]);
          mframes[wfi].setTsUSec((unsigned short) t[2]);
          tsm=t[1];
        }
        }
      }    
     }
    }
   }
  }
  return tsm;
}

unsigned short MosquitoesInSeq::getTsUSec(unsigned long i, bool isTrigger /* = false */)
{
  unsigned short tsu=0;
  if(!dynamic)
  {
   long tr=i;
   if(!isTrigger)
   {
    if(triggerBased)
    {
     long t=getTriggerFromSeq(tr);
     long wfi=findTrigger(t);
     if(wfi!=-1)
     {
      tr=wfi;
     }
     else
     {
      return 0;
     }
    }
    if(tr<nFrames && tr>=0)
    {
     if(mframes[tr].numOfMosquitoes()!=0)
      tsu=mframes[tr].getTsUSec();
     else
     {
      tsu=mframes[tr].getTsUSec();
      int ts=mframes[tr].getTsSec();
      if(ts==0 && haveSeq)
      {
       vector<int> t=currSeq->timeOfFrameI(tr);
       if(t.size()==3)
       {
        mframes[tr].setTsSec(t[0]);
        mframes[tr].setTsMSec((unsigned short) t[1]);
        mframes[tr].setTsUSec((unsigned short) t[2]);
        tsu=t[2];
       }
      }    
     }
    }
   }
   else
   {
    if(triggerBased)
    {
     long wfi=findTrigger(tr);
     if(wfi!=-1)
     {
      tr=wfi;
     }
     else
     {
      return 0;
     } 
    }
    else
    {
     if(haveSeq)
     {
      if(currSeq->hasMeta())
      {
       int tmp=currSeq->lookUpTrigger(tr);
       if(tmp!=-1)
        tr=tmp;
       else
       {
        cerr<<"I can't lookup the trigger, lookup failed...\n";
        return 0;             
       }
      }
      else
      {
       cerr<<"I can't lookup the trigger, seq does not have metadata...\n";
       return 0;      
      }
     }
     else
     {
      cerr<<"I can't lookup the trigger, I don't have a seq to search...\n";
      return 0;
     }
    }
    if(tr<nFrames && tr>=0)
    {
     if(mframes[tr].numOfMosquitoes()!=0)
      tsu=mframes[tr].getTsUSec();
     else
     {
      tsu=mframes[tr].getTsUSec();
      int ts=mframes[tr].getTsSec();
      if(ts==0 && haveSeq)
      {
       vector<int> t=currSeq->timeOfFrameI(tr);
       if(t.size()==3)
       {
        mframes[tr].setTsSec(t[0]);
        mframes[tr].setTsMSec((unsigned short) t[1]);
        mframes[tr].setTsUSec((unsigned short) t[2]);
        tsu=t[2];
       }
      }    
     }
    }
   }
  }
  else
  {
   long tr=i;
   if(!isTrigger)
   {
    long tmp=getTriggerFromSeq(tr);
    if(tmp!=-1)
    {
     tr=tmp;
    }
    else
    {
     cerr<<"Could not translate the frame to a trigger...\n";
     return 0;
    }
   }
   if(tr>0)
   {
    long wfi=findTrigger(tr);
    if(wfi!=-1)
    {
    if(mframes[wfi].numOfMosquitoes()!=0)
      tsu=mframes[wfi].getTsUSec();
    else
    {
      tsu=mframes[wfi].getTsUSec();
      int ts=mframes[wfi].getTsSec();
      if(ts==0 && haveSeq)
      {
        int fn=currSeq->lookUpTrigger(tr);
        if(fn!=-1)
        {
        vector<int> t=currSeq->timeOfFrameI(fn);
        if(t.size()==3)
        {
          mframes[wfi].setTsSec(t[0]);
          mframes[wfi].setTsMSec((unsigned short) t[1]);
          mframes[wfi].setTsUSec((unsigned short) t[2]);
          tsu=t[2];
        }
        }
      }    
     }
    }
   }
  }
  return tsu;
}

int MosquitoesInSeq::removeAll()
{
    if(initialized)
    {
        vector<MosquitoesInFrame>().swap(mframes);  
        for(unsigned long n=0;n<nFrames; n++)
            mframes.push_back(MosquitoesInFrame());
    }
    return 0;
}

int MosquitoesInSeq::removePointsInsideOfPolygon(vector<QPointF> poly,unsigned long n, unsigned long m, bool inverse /* = false */, bool isTrigger /* = false */)
{
    int num=0;
    if(!dynamic)
    {
    if(m>nFrames)m=nFrames;
    for(unsigned long i=n; i<m;i++)
     if(i<nFrames)
     {
         num+=mframes[i].removePointsInsideOfPolygon(poly,inverse);
     }
    }
    else
    {
     long tmin=n;
     long tmax=m;
     if(!isTrigger)
     {
      tmin=getTriggerFromSeq(n);
      tmax=getTriggerFromSeq(m);
     }
     if(tmin>0 && tmax>0 && tmax>tmin)
     {
      long wfi=findTriggerNearest(tmin);
      long wfj=findTriggerNearest(tmax,false);
      if(wfi!=-1)
      {
       if(wfj==-1) wfj=wfi+1;
       for(unsigned int i=wfi; i<wfj; i++)
       {
         num+=mframes[i].removePointsInsideOfPolygon(poly,inverse);
       }
      }
     }
    }
    return num; 
}

int MosquitoesInSeq::removePoints(double x, double y,unsigned long n, unsigned long m, bool isTrigger /* = false */, double searchRadius /*=8*/, bool onlyOne /*=true*/)
{
  int p=-1;
  int q=-1;
  double minDist=searchRadius;
  if(!dynamic)
  {
    if(m>nFrames)m=nFrames;
    for(unsigned long i=n; i<m;i++)
     if(i<nFrames)
     {
       if(!onlyOne)
         mframes[i].removeMosquito(x,y,searchRadius,onlyOne);
       else
       {
        double dist;
        int j;
        bool ret=mframes[i].getNearest(x,y,minDist,j,dist);
        if(ret) {
         minDist=dist;
         p=i;
         q=j;
        }
       }
     }
    if(p!=-1 && q!=-1)
    {
     mframes[p].removeMosquito(q);
    }
  }
  else
  {
     long tmin=n;
     long tmax=m;
     if(!isTrigger)
     {
      tmin=getTriggerFromSeq(n);
      tmax=getTriggerFromSeq(m);
     }
     if(tmin>0 && tmax>0 && tmax>tmin)
     {
      long wfi=findTriggerNearest(tmin);
      long wfj=findTriggerNearest(tmax,false);
      if(wfi!=-1)
      {
       if(wfj==-1) wfj=wfi+1;
       for(unsigned int i=wfi; i<wfj; i++)
       {
        if(!onlyOne)
         mframes[i].removeMosquito(x,y,onlyOne);       
        else
        {
         double dist;
         int j;
         bool ret=mframes[i].getNearest(x,y,minDist,j,dist);
         if(ret) {
          minDist=dist;
          p=i;
          q=j;
         }
        }
       }
      }
     }
    if(p!=-1 && q!=-1)
    {
     mframes[p].removeMosquito(q);
    }      
  }
    return 0; 
}

int MosquitoesInSeq::removePoints(unsigned long n, unsigned long m, bool isTrigger /* = false */)
{
  if(!dynamic)
  {
    if(m>nFrames)m=nFrames;
    for(unsigned long i=n; i<m;i++)
     if(i<nFrames)
     {
         mframes[i].removeAllMosquito();
     }
  }
  else
  {
     long tmin=n;
     long tmax=m;
     if(!isTrigger)
     {
      tmin=getTriggerFromSeq(n);
      tmax=getTriggerFromSeq(m);
     }
     if(tmin>0 && tmax>0 && tmax>tmin)
     {
      long wfi=findTriggerNearest(tmin);
      long wfj=findTriggerNearest(tmax,false);
      if(wfi!=-1)
      {
       if(wfj==-1) wfj=wfi+1;
       for(unsigned int i=wfi; i<wfj; i++)
       {
         mframes[i].removeAllMosquito();       
       }
      }
     }
     
  }
    return 0; 
}

vector<cv::Point2f> MosquitoesInSeq::pointsInsideOfPolygon(vector<QPointF> poly,unsigned long n, unsigned long m, bool isTrigger /* = false */)
{
    vector<cv::Point2f> listOfPoints;
    if(!dynamic)
    {
    if(m>nFrames)m=nFrames;
    for(unsigned long i=n; i<m;i++)
     if(i<nFrames)
     {
         vector<cv::Point2f> tmp=mframes[i].pointsInsideOfPolygon(poly);
         for(unsigned int j=0; j<tmp.size(); j++)
          listOfPoints.push_back(tmp[j]);
     }
    }
    else
    {
     long tmin=n;
     long tmax=m;
     if(!isTrigger)
     {
      tmin=getTriggerFromSeq(n);
      tmax=getTriggerFromSeq(m);
     }
     if(tmin>0 && tmax>0 && tmax>tmin)
     {
      long wfi=findTriggerNearest(tmin);
      long wfj=findTriggerNearest(tmax,false);
      if(wfi!=-1)
      {
       if(wfj==-1) wfj=wfi+1;
       for(unsigned int i=wfi; i<wfj; i++)
       {
         vector<cv::Point2f> tmp=mframes[i].pointsInsideOfPolygon(poly);
         for(unsigned int j=0; j<tmp.size(); j++)
          listOfPoints.push_back(tmp[j]);       
       }
      }
     }
    }
    return listOfPoints; 
}

unsigned long MosquitoesInSeq::getFirstFrameWithMosquito()
{
 unsigned long f=0;
 for (unsigned int i=0; i<nFrames; i++)
 {
  if(mframes[i].numOfMosquitoes()!=0)
  {
   f=i;
   break;
  }
 }
 if(dynamic && triggerBased)
 {
   f=FrameLookup[f];
 } 
 return f; 
}

unsigned long MosquitoesInSeq::getLastFrameWithMosquito()
{
 unsigned long l=0;
 for (int i=nFrames-1; i>=0; i--)
 {
  if(mframes[i].numOfMosquitoes()!=0)
  {
   l=i;
   break;
  }
 }
 if(dynamic && triggerBased)
 {
   l=FrameLookup[l];
 } 
 return l; 
}

long MosquitoesInSeq::getMaxTrigger()
{
 long l=triggerOfIdx(mframes.size()-1);
 return l; 
}

vector<unsigned long> MosquitoesInSeq::findMosquito(QPointF p,unsigned long n, unsigned long m, bool isTrigger /*= false*/)
{
    vector<unsigned long> ret;
    unsigned long frameNo=0;
    unsigned int MosquiNo=0;
    bool foundOne=false;
    double distance=100000.;
    if(!dynamic)
    {
    if(m>nFrames)m=nFrames;
    for(unsigned long i=n; i<m;i++)
     if(i<nFrames)
     {
         for(unsigned int j=0; j<mframes[i].numOfMosquitoes(); j++)
         {
          MosquitoPosition* tmp=mframes[i].mosqi(j);
          double dist=sqrt(pow(tmp->getX()-p.x(),2)+pow(tmp->getY()-p.y(),2));
          if(dist<distance)
          {
           distance=dist;
           frameNo=i;
           MosquiNo=j;
           foundOne=true;
          }
         }
     }
    }
    else
    {
     long tmin=n;
     long tmax=m;
     if(!isTrigger)
     {
      tmin=getTriggerFromSeq(n);
      tmax=getTriggerFromSeq(m);
     }
     if(tmin>0 && tmax>0 && tmax>tmin)
     {
      long wfi=findTriggerNearest(tmin);
      long wfj=findTriggerNearest(tmax,false);
      if(wfi!=-1)
      {
       if(wfj==-1) wfj=wfi+1;
       for(unsigned int i=wfi; i<wfj; i++)
       {
         for(unsigned int j=0; j<mframes[i].numOfMosquitoes(); j++)
         {
          MosquitoPosition* tmp=mframes[i].mosqi(j);
          double dist=sqrt(pow(tmp->getX()-p.x(),2)+pow(tmp->getY()-p.y(),2));
          if(dist<distance)
          {
           distance=dist;
           frameNo=i;
           MosquiNo=j;
           foundOne=true;
          }
         }   
       }
      }
     }
    }
     if(foundOne && distance<32.)
     {
      ret.push_back(frameNo);
      ret.push_back(MosquiNo);
     }
     return ret;
}

bool MosquitoesInSeq::copyTo(MosquitoesInSeq& a)
{
    if(a.initialized)
    {
      vector<MosquitoesInFrame>().swap(a.mframes);  
      a.nFrames=0;
      a.initialized=0;
    }
    
    a.haveSeq=haveSeq;
    a.currSeq=currSeq;
    a.initialized=initialized;
    a.nFrames=nFrames;
    if(a.initialized)
    {
      vector<MosquitoesInFrame>().swap(a.mframes);  
      for(unsigned long i=0; i<nFrames; i++)
          a.mframes.push_back(mframes[i]);
    }
    a.dynamic=dynamic;
    a.triggerBased=triggerBased;
    a.FrameLookup=FrameLookup;
    return true;
}

bool MosquitoesInSeq::resetFlags()
{
    MosquitoesInFrame* currentFrame=nullptr;
    for(unsigned long i=0; i<nFrames;i++)
    {
        currentFrame=&mframes[i];
        MosquitoPosition* currentMosquito=nullptr;
        for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
        {
            currentMosquito=currentFrame->mosqi(ii);
            currentMosquito->setFlag(17,true);
        }
    }
    return true;
}

vector<vector<cv::Point> > MosquitoesInSeq::getContours(unsigned long i, unsigned long j /*=0*/,bool isTrigger /*= false*/)
{
  vector<vector<cv::Point> > tmp;
  MosquitoesInFrame* currentFrame=nullptr;
  if(j<i) j=i+1;
  if(j>nFrames)j=nFrames;
  if(!dynamic)
  {
    if(i<nFrames && j<=nFrames)
    {
     for(unsigned int n=i; n<j; n++)
     {
      if(n<nFrames)
      {
        currentFrame=&mframes[n];
        for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
        {
          MosquitoPosition* currentMosquito=nullptr; 
          currentMosquito=currentFrame->mosqi(ii);
          if(currentMosquito)
          {
           if(currentMosquito->hasCont())
           {
            tmp.push_back(currentMosquito->getContour());
           }
          }
        }
      }
     }
    }
  }
  else
  {
   long tmin=i;
   long tmax=j;
   if(!isTrigger)
   {
    tmin=getTriggerFromSeq(i);
    tmax=getTriggerFromSeq(j);
   }
   if(tmin>0 && tmax>0 && tmax>tmin)
   {
    long wfi=findTriggerNearest(tmin);
    long wfj=findTriggerNearest(tmax,false);
    if(wfi!=-1)
    {
     if(wfj==-1) wfj=wfi+1;
     for(unsigned int n=wfi; n<wfj; n++)
     {
      currentFrame=&mframes[n];
      for(int ii=0; ii<currentFrame->numOfMosquitoes(); ii++)
      {
       MosquitoPosition* currentMosquito=nullptr; 
       currentMosquito=currentFrame->mosqi(ii);
       if(currentMosquito)
       {
        if(currentMosquito->hasCont())
        {
         tmp.push_back(currentMosquito->getContour());
        }
       }
      }
     }
    }
   }
  }
  return tmp;
}

bool MosquitoesInSeq::isDynamic()
{
 return dynamic;
}

bool MosquitoesInSeq::isTriggerBased()
{
 return triggerBased;
}

void MosquitoesInSeq::sortFrames()
{
 //sort mframes and FrameLookup
 vector<pair<unsigned long,MosquitoesInFrame> > tmp;
 //zip
 for(unsigned i=0; i<FrameLookup.size(); i++)
  tmp.push_back(make_pair(FrameLookup[i],mframes[i]));
 //sort
 sort(begin(tmp), end(tmp),[&](const pair<unsigned long,MosquitoesInFrame>& a, const pair<unsigned long,MosquitoesInFrame>& b){return a.first < b.first;});
 //unzip
 for(unsigned i=0; i<tmp.size(); i++)
 {
  FrameLookup[i] = tmp[i].first;
  mframes[i] = tmp[i].second;
 }
 nFrames=mframes.size();
}

vector<unsigned long> MosquitoesInSeq::getTriggerRange()
{
 vector<unsigned long> tmp;
 if(dynamic && triggerBased && FrameLookup.size()>0)
 {
  tmp.push_back(FrameLookup[0]);
  tmp.push_back(FrameLookup[FrameLookup.size()-1]);
 }
 return tmp;
}

long MosquitoesInSeq::findTrigger(unsigned long tmp)
{
     long wf=-1;
     for(unsigned long nn=0; nn<FrameLookup.size(); nn++)
      if(tmp == FrameLookup[nn])
       wf=nn;
     return wf;
}

long MosquitoesInSeq::findTriggerNearest(unsigned long tmp, bool next /*=true*/)
{
     long wf=-1;
     for(unsigned long nn=0; nn<FrameLookup.size(); nn++)
      if(tmp == FrameLookup[nn])
       wf=nn;
     if(wf==-1)
     {
     if(next)
     {
     for(unsigned long nn=0; nn<FrameLookup.size(); nn++)
      if(tmp < FrameLookup[nn])
      {
       wf=nn;
       break;
      }      
     }
     else
     {
     for(unsigned long nn=FrameLookup.size(); nn>0; nn--)
      if(tmp > FrameLookup[nn-1])
      {
       wf=nn-1;
       break;
      }      
     }
     }
     return wf;
}

long MosquitoesInSeq::getTriggerFromSeq(unsigned long i)
{
  if(!currSeq) return -1;
  long tmp=0;
  if(i>=nFrames)i=nFrames-1;
  if(currSeq->hasMeta())
   tmp=currSeq->metaData(i);
  if(tmp<0)
   tmp=currSeq->metaData(i+tmp);
  return tmp; 
}

bool MosquitoesInSeq::dataForTrigger(int t)
{
  long wf=findTrigger(t);
  if(wf==-1)
   return false;
  else
   return true;
}

int MosquitoesInSeq::triggerOfIdx(unsigned long i)
{
  int t=-1;
  if(dynamic && triggerBased)
  {
   if(i<FrameLookup.size())
    t=FrameLookup[i];
  }
  else if(haveSeq)
  {
   t=getTriggerFromSeq(i);
  }
  return t;
}

bool MosquitoesInSeq::fixTime()
{
 if(haveSeq && currSeq)
 {
  for(int i=0; i<nFrames; i++)
  {
   vector<int> ttt=currSeq->timeOfFrameI(i);
   mframes[i].setTime(ttt);
  }
 }
 return true;
}

int MosquitoesInSeq::removeMarkedMosquitoes(unsigned long n, unsigned long m, bool isTrigger /* = false */, bool directAccess /*= false */)
{
  //cerr<<"Remove Point "<<x<<","<<y<<" in range "<< n<<"->"<<m<<"\n";
  if(!dynamic || directAccess)
  {
    if(m>nFrames)m=nFrames;
    for(unsigned long i=n; i<m;i++)
     if(i<nFrames)
     {
         mframes[i].removeMarkedMosquitoes();
     }
  }
  else
  {
     long tmin=n;
     long tmax=m;
     if(!isTrigger)
     {
      tmin=getTriggerFromSeq(n);
      tmax=getTriggerFromSeq(m);
     }
     if(tmin>0 && tmax>0 && tmax>tmin)
     {
      long wfi=findTriggerNearest(tmin);
      long wfj=findTriggerNearest(tmax,false);
      if(wfi!=-1)
      {
       if(wfj==-1) wfj=wfi+1;
       for(unsigned int i=wfi; i<wfj; i++)
       {
         mframes[i].removeMarkedMosquitoes();       
       }
      }
     }
     
  }
    return 0; 
}

void MosquitoesInSeq::contourStatistics()
{
    for(unsigned long i=0;i<mframes.size();i++)
     mframes[i].contourStatistics();
}

int MosquitoesInSeq::filterByArea(double A, bool invert /*= false*/)
{
    int nRem=0;
    for(unsigned long i=0;i<mframes.size();i++)
     nRem+=mframes[i].filterByArea(A, invert);
    return nRem;
}

int MosquitoesInSeq::filterByMaxInt(double I, bool invert /*= false*/)
{
    int nRem=0;
    for(unsigned long i=0;i<mframes.size();i++)
     nRem+=mframes[i].filterByMaxInt(I, invert);
    return nRem;
}

int MosquitoesInSeq::filterByCircularity(double I, bool invert /*= false*/)
{
    int nRem=0;
    for(unsigned long i=0;i<mframes.size();i++)
     nRem+=mframes[i].filterByCircularity(I, invert);
    return nRem;
}

int MosquitoesInSeq::filterByInertia(double I, bool invert /*= false*/)
{
    int nRem=0;
    for(unsigned long i=0;i<mframes.size();i++)
     nRem+=mframes[i].filterByInertia(I, invert);
    return nRem;
}

int MosquitoesInSeq::filterByConvexity(double I, bool invert /*= false*/)
{
    int nRem=0;
    for(unsigned long i=0;i<mframes.size();i++)
     nRem+=mframes[i].filterByConvexity(I, invert);
    return nRem;
}

int MosquitoesInSeq::filterByRadius(double I, bool invert /*= false*/)
{
    int nRem=0;
    for(unsigned long i=0;i<mframes.size();i++)
     nRem+=mframes[i].filterByConvexity(I, invert);
    return nRem;
}

void MosquitoesInSeq::setKeepMeFlag(bool b)
{
    for(unsigned long i=0;i<mframes.size();i++)
     mframes[i].setKeepMeFlag(b);
}

vector<vector<double> >  MosquitoesInSeq::pointsInsideOfPolygonAsVector(vector<QPointF> poly,unsigned long n, unsigned long m, bool isTrigger /* = false */)
{
   vector<vector<double> > listOfPoints;
    if(!dynamic)
    {
    if(m>nFrames)m=nFrames;
    for(unsigned long i=n; i<m;i++)
     if(i<nFrames)
     {
         vector<vector<double> > tmp=mframes[i].pointsInsideOfPolygonAsVector(poly);
         listOfPoints.insert(listOfPoints.end(), tmp.begin(), tmp.end());
     }
    }
    else
    {
     long tmin=n;
     long tmax=m;
     if(!isTrigger)
     {
      tmin=getTriggerFromSeq(n);
      tmax=getTriggerFromSeq(m);
     }
     if(tmin>0 && tmax>0 && tmax>tmin)
     {
      long wfi=findTriggerNearest(tmin);
      long wfj=findTriggerNearest(tmax,false);
      if(wfi!=-1)
      {
       if(wfj==-1) wfj=wfi+1;
       for(unsigned int i=wfi; i<wfj; i++)
       {
         vector<vector<double> > tmp=mframes[i].pointsInsideOfPolygonAsVector(poly);
         listOfPoints.insert(listOfPoints.end(), tmp.begin(), tmp.end());
       }
      }
     }
    }
    return listOfPoints; 
}
