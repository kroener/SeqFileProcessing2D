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
#include "MainWindow.h"
#include "VideoPlayerWidget.h"
#include "SeqInfoWidget.h"
#include "TrackingParameterWidget.h"
#include "FrameSelectWidget.h"
#include "HistogramWidget.h"
#include "Viewer.h"
#include "ViewerWithMouse.h"
#include "DisplayAllPointsWidget.h"
#include "SegmentationWorker.hpp"
#include "MosquitoesInFrame.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoPosition.hpp"
#include "iSeq.hpp"
#include "DisplaySettingsParameterWidget.h"
#include "ImageComposer.hpp"
#include <QApplication>
#include <QDialog>
#include <QInputDialog>
#include <QFileDialog>
#include <QMenuBar>
#include <QThread>
#include <QSettings>
#include <QDate>
#include <fstream>
#include <QTableWidget>
#include <QKeyEvent>
#include <QPointF>
#include <QLabel>
#include <QDockWidget>
#include <QProgressDialog>
#include <QMessageBox>
#include <vector>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include "MosquitoRegistry.hpp"
#include "TrackPoint.hpp"
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
#include <unistd.h>
#include <time.h>
#include <libavcodec/avcodec.h>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <cmath>

using namespace std;
using namespace cv;


int MainWindow::seqPointerToSeqNum(iSeq* sp)
{
 int ret=-1;
 for(unsigned int i=0; i<seq.size(); i++)
  if(sp == &seq[i])
   ret=i;
 return ret;
}

void MainWindow::updatePointerOfTracks(int c, int r)
{
 if(seq.size()>0)
 if(m_player->getSeqPointer(c,r) && m_player->getTrackPointer(c,r) && m_player->getMosqPointer(c,r))
 {
  m_player->getTrackPointer(c,r)->updateMosquitoPointer(m_player->getMosqPointer(c,r));
 }
}

bool MainWindow::backupPos(unsigned int n, bool force /*=false*/)
{
 if(!disablePosBackup && !force)
  cerr<<"I don't want to backup..\n";
 else
  cerr<<"I want to backup..\n";
 if(n<MosqPos.size() && (disablePosBackup || force))
 {
  MosquitoesInSeq tmp;
  MosqPos[n].copyTo(tmp);
  MosqPosHist[n].push_back(tmp);
  cerr<<"Have added Positions to backup.\nNow I have "<< MosqPosHist[n].size()<< " backups for " << n<<"\n";
  for(unsigned int i=0; i<MosqPosHist[n].size(); i++)
   cerr<<"backup ["<<i<<"] contains "<<MosqPosHist[n][i].getNumOfAllPos()<<" Mosquitoes\n";
  if(MosqPosHist[n].size()>maxHistSize)
  {
   unsigned int N=MosqPosHist[n].size()-maxHistSize;
   vector<MosquitoesInSeq> tmp(MosqPosHist[n].begin() + N, MosqPosHist[n].begin()+MosqPosHist[n].size());
   MosqPosHist[n].swap(tmp);
   cerr<<"Reduced backup size to "<< MosqPosHist[n].size()<< " backups for " << n<<"\n"; 
  }
  
  return true;
 }
 else
 {
  return false;
 }
}

bool MainWindow::backupTracks(unsigned int n)
{
 if(n<Reg.size())
 {
  MosquitoRegistry tmp;
  Reg[n].copyTo(tmp);
  RegHist[n].push_back(tmp);
  cerr<<"Have added Tracks to backup.\nNow I have "<< RegHist[n].size()<< " backups for " << n<<"\n";
  for(unsigned int i=0; i<RegHist[n].size(); i++)
   cerr<<"backup ["<<i<<"] contains "<<RegHist[n][i].numOfTracks()<<" Tracks\n";
  if(RegHist[n].size()>maxHistSize)
  {
   unsigned int N=RegHist[n].size()-maxHistSize;
   vector<MosquitoRegistry> tmp(RegHist[n].begin() + N, RegHist[n].begin()+RegHist[n].size());
   RegHist[n].swap(tmp);
   cerr<<"Reduced backup size to "<< RegHist[n].size()<< " backups for " << n<<"\n"; 
  }
  return true;
 }
 else
  return false;
}

bool MainWindow::restorePos(unsigned int n)
{
 if(n<MosqPos.size())
 {
  if(MosqPosHist[n].size()>0)
  {
   MosquitoesInSeq tmp=MosqPosHist[n].back();
   tmp.copyTo(MosqPos[n]);
   MosqPosHist[n].pop_back();
   return true;
  }
 }
 else
 {
  cerr<<"I don't have any backups left!\n";
 }
 return false;
}

bool MainWindow::restoreTracks(unsigned int n)
{
 if(n<Reg.size())
 {
  if(RegHist[n].size()>0)
  {
   MosquitoRegistry tmp=Reg[n] = RegHist[n].back();
   tmp.copyTo(Reg[n]);
   RegHist[n].pop_back();
   return true;
  }
 }
 return false;
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

vector<cv::Point> MainWindow::polyBB(vector<cv::Point> ppp, cv::Point2f center, double scale /*= 1.0*/)
{
  vector<cv::Point> bb;
  if(ppp.size()>2)
  {
    double minx=ppp[0].x*scale-center.x;
    double maxx=ppp[0].x*scale-center.x;
    double miny=ppp[0].y*scale-center.y;
    double maxy=ppp[0].y*scale-center.y;
    for(unsigned long i=1;i<ppp.size(); i++)
    {
     if(ppp[i].x*scale-center.x<minx)minx=ppp[i].x*scale-center.x;
     if(ppp[i].x*scale-center.x>maxx)maxx=ppp[i].x*scale-center.x;
     if(ppp[i].y*scale-center.y<miny)miny=ppp[i].y*scale-center.y;
     if(ppp[i].y*scale-center.y>maxy)maxy=ppp[i].y*scale-center.y;
    }
    bb.push_back(Point(int(minx+0.5)-1,int(miny+0.5)-1));
    bb.push_back(Point(int(maxx+0.5)+1,int(maxy+0.5)+1)); 
  }
  return bb;
}

vector<QPointF> MainWindow::toVectorOfQPointF(vector<vector<double> > in)
{
 vector<QPointF> tmp;
 for(int i=0; i<in.size(); i++)
  if(in[i].size()==2)
   tmp.push_back(QPointF(in[i][0],in[i][1]));
  else
   cerr<<"Error: Data needs to be in nx2 array...\n";
 return tmp;
}

vector<vector<double> > MainWindow::segmentROI(iSeq* seqPointer,cv::Rect roi, unsigned long workF, double &maxdiff)
{
    vector<vector<double> > centers;
    if(seqPointer)
    {
    if(ith_changed)
    {
     seqPointer->setRequired(useIth+1);
     ith_changed=0;
    }
    seqPointer->loadImage(workF);
    Image<unsigned char>* a;
    Image<unsigned char>* b;
    a=seqPointer->getImagePointer(0);
    b=seqPointer->getImagePointer(useIth);    
    cv::Mat noise;
    bool haveNoise=false;
    Image<short> work;
    Image<double> dwork;
    work.create(roi.width,roi.height);
    dwork.create(roi.width,roi.height);
    int medianBlur1=segParam->getMedianBlur1Param();
    int medianBlur2=segParam->getMedianBlur2Param();
    int gaussK1=segParam->getGaussianBlur1Param();
    double gaussS1=segParam->getGaussianSigma1Param();
    int gaussK2=segParam->getGaussianBlur2Param();
    double gaussS2=segParam->getGaussianSigma2Param();
    bool useDouble=false;
    if(medianBlur1)
    {
     Image<unsigned char> a2(a);
     Image<unsigned char> b2(b);
     a2.medianBlur(medianBlur1);
     b2.medianBlur(medianBlur1);
     if(gaussK1 && gaussK1>0 && gaussK1%2==1)
     {
      Image<double> a3(a2);
      Image<double> b3(b2);
      a3.gaussianBlur(gaussK1,gaussS1);
      b3.gaussianBlur(gaussK1,gaussS1);
      useDouble=true;
      if(blackOnWhite)
       b3.diff(&a3,&dwork,roi);
      else
       a3.diff(&b3,&dwork,roi);            
     }
     else
     {
      if(blackOnWhite)
       b2.diff(&a2,&work,roi);
      else
       a2.diff(&b2,&work,roi);
     }
    }
    else
    {
     if(gaussK1 && gaussK1>0 && gaussK1%2==1)
     {
      useDouble=true;
      Image<double> a2(a);
      Image<double> b2(b);
      a2.gaussianBlur(gaussK1,gaussS1);
      b2.gaussianBlur(gaussK1,gaussS1);
      if(blackOnWhite)
       b2.diff(&a2,&dwork,roi);
      else
       a2.diff(&b2,&dwork,roi);
     }
     else
     {
      if(blackOnWhite)
       b->diff(a,&work,roi);
      else
       a->diff(b,&work,roi);
     }
    }   
    if(medianBlur2)
    {
     if(useDouble)
     {
      dwork.medianBlur(medianBlur2);
     }
     else
     {
      work.medianBlur(medianBlur2);
     }
    }
    if(gaussK2 && gaussK2>0 && gaussK2%2==1)
    { 
     if(!useDouble) {
      dwork=Image<double>(work);
      useDouble=true;
     }
     dwork.gaussianBlur(gaussK2,gaussS2);
    }
    
    int maxi=0;
    vector<vector<int> > hist;
    if(useDouble)
     hist=dwork.dHistogram(&maxi);
    else
     hist=work.dHistogram(&maxi);
    
    if(hist.size())
    {
        double threshold=hist[maxi][0]+fracN*(hist[hist.size()-1][0]-hist[maxi][0]);
        maxdiff=hist[hist.size()-1][0];
        if(!haveNoise)
        {
         if(threshold<minThreshold+hist[maxi][0])threshold=minThreshold+hist[maxi][0];
         if(useDouble)
          dwork.Threshold(threshold);
         else
          work.Threshold(threshold);
        }
        else
        {
         if(useDouble)
          dwork.Threshold(threshold,noise(roi),minThreshold, 2.0);        
         else
          work.Threshold(threshold,noise(roi),minThreshold, 2.0);        
        }
        if(useDouble)
         dwork.erodeAndDilate(erode, dilute);
        else
         work.erodeAndDilate(erode, dilute);
        if(useDouble)
         centers=dwork.removeSmallBlobsNew(minA,maxA,threshold,maxdiff);
        else
         centers=work.removeSmallBlobsNew(minA,maxA,threshold,maxdiff);
        for(unsigned int i=0; i<centers.size(); i++)
        {
         if(centers[i].size()>1)
         {
          centers[i][0]+=roi.x;
          centers[i][1]+=roi.y;
          if(centers[i].size()>8)
          {
           for(unsigned long j=0; j<centers[i][8]; j++)
            if(9+j*2+1<centers[i].size())
           {
            centers[i][9+j*2]+=roi.x;
            centers[i][9+j*2+1]+=roi.y;
           }
          }
         }
        }
    }
    else
    {
     maxdiff=-1;
    }
    }
    else
    {
     maxdiff=-1;
    }
    return centers;
}



