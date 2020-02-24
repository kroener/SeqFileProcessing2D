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
#include <functional>
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

using namespace std;
using namespace cv;

void MainWindow::showContextMenuClicked(QPoint m, int c, int r)
{
  QMenu contextMenu(tr("Context menu"), this);
  QMenu* seqList = contextMenu.addMenu(tr("&Select Sequence"));
  for(unsigned int i=0; i<seq.size(); i++)
  {
     seqList-> addAction(QString::fromStdString(seq[i].filename()).section("/",-1,-1), this,[this,i,c,r](){displaySeqIn(i,c,r);});
  }
  QMenu* displayMode = contextMenu.addMenu(tr("&Display Menu"));
  displayMode-> addAction(tr("Toggle Points"), this,[this](){displayParam->setShowPoints(!displayParam->getShowPoints());});
  displayMode-> addAction(tr("Toggle Tracks"), this,[this](){displayParam->setShowTracks(!displayParam->getShowTracks());});
  displayMode-> addAction(tr("Show Points of selected frames"), this,[this,c,r](){
  if(seq.size()>0)m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
  showAllPoints=true;
  });
  displayMode-> addAction(tr("Clear Points"), this,[this,c,r](){
  showAllPoints=false;
  });
    
  displayMode-> addAction(tr("Clear Polygons"), this,[this,c,r](){
            m_player->clearPolygons(c,r);   
          if(showAllPoints)
           m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
  });
    
  displayMode-> addAction(tr("Undo Position"), this,[this,c,r](){
      if(seq.size()>0)
      {
         int sqn=seqPointerToSeqNum(m_player->getSeqPointer(c,r));
         if(sqn!=-1)
         {
          restorePos(sqn);
          m_player->showImageWithPoints(m_player->getCurrentFrameNo());
          if(showAllPoints)
           m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
         }
      }
  });
  displayMode-> addAction(tr("Undo Tracks"), this,[this,c,r](){
      if(seq.size()>0)
      {
         int sqn=seqPointerToSeqNum(m_player->getSeqPointer(c,r));
         if(sqn!=-1)
         {
          restoreTracks(sqn);
          m_player->showImageWithPoints(m_player->getCurrentFrameNo());
          if(showAllPoints)
           m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
         }
      }
  });

  displayMode-> addAction(tr("Clear Track Highlight"), this,[this,c,r](){
      if(seq.size()>0)
      {
        m_player->clearTrackToHighlight(c,r);
          if(showAllPoints)
           m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
      }
  });

  displayMode-> addAction(tr("Display difference Image"), this,[this,c,r](){
      if(seq.size()>0)
      { 
       if(ith_changed)
       {
        m_player->getSeqPointer(c,r)->setRequired(useIth+1);
        ith_changed=0;
       }
       Image<short> *work;
       work=new Image<short>;
       Image<double> *dwork;
       dwork=new Image<double>;
       
       Image<unsigned char>* a=currSeq->getImagePointer(0);
       Image<unsigned char>* b=currSeq->getImagePointer(useIth);
       if(!(work->getX()==a->getX() && work->getY()==a->getY()))
        work->create(a->getX(),a->getY());
       if(!(dwork->getX()==a->getX() && dwork->getY()==a->getY()))
        dwork->create(a->getX(),a->getY());

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
          b3.diff(&a3,dwork);
         else
          a3.diff(&b3,dwork);            
        }
        else
        {
        if(blackOnWhite)
         b2.diff(&a2,work);
        else
         a2.diff(&b2,work);
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
          b2.diff(&a2,dwork);
         else
          a2.diff(&b2,dwork);
        }
        else
        {
         if(blackOnWhite)
          b->diff(a,work);
         else
          a->diff(b,work);
        }
       }
       
       if(medianBlur2)
       {
        if(useDouble)
        {
         dwork->medianBlur(medianBlur2);
        }
        else
        {
         work->medianBlur(medianBlur2);
        }
       }
       if(gaussK2 && gaussK2>0 && gaussK2%2==1)
       { 
        if(!useDouble) {
         (*dwork)=Image<double>(work);
         useDouble=true;
        }
        dwork->gaussianBlur(gaussK2,gaussS2);
       }
       Mat R,B;
       if(useDouble)
       {
        R=dwork->image(1);
        B=dwork->image(2);
       }
       else
       {
        R=work->image(1);
        B=work->image(2);
       }
       Mat G=Mat::zeros(B.size(), B.type());       
       std::vector<cv::Mat> array_to_merge;
       array_to_merge.push_back(B);
       array_to_merge.push_back(G);
       array_to_merge.push_back(R);
       cv::Mat color;
       cv::merge(array_to_merge, color);

       m_player->showImage(color, m_player->getSeqPointer(c,r)->currentFrame(),c,r);
      }
  });

  displayMode-> addAction(tr("Display segmented Image"), this,[this,c,r](){
      if(seq.size()>0)
      { 
       if(ith_changed)
       {
        m_player->getSeqPointer(c,r)->setRequired(useIth+1);
        ith_changed=0;
       }
       Image<short> *work;
       work=new Image<short>;
       Image<double> *dwork;
       dwork=new Image<double>;
       
       Image<unsigned char>* a=currSeq->getImagePointer(0);
       Image<unsigned char>* b=currSeq->getImagePointer(useIth);
       if(!(work->getX()==a->getX() && work->getY()==a->getY()))
        work->create(a->getX(),a->getY());
       if(!(dwork->getX()==a->getX() && dwork->getY()==a->getY()))
        dwork->create(a->getX(),a->getY());

       int medianBlur1=segParam->getMedianBlur1Param();
       int medianBlur2=segParam->getMedianBlur2Param();
       int gaussK1=segParam->getGaussianBlur1Param();
       double gaussS1=segParam->getGaussianSigma1Param();
       int gaussK2=segParam->getGaussianBlur2Param();
       double gaussS2=segParam->getGaussianSigma2Param();
       double minA=segParam->getMinA();
       double maxA=segParam->getMaxA();
       double fracN=segParam->getThreshold();
       double minThreshold=segParam->getMinThreshold();
       int erode=segParam->getErode();
       int dilute=segParam->getDilute();

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
          b3.diff(&a3,dwork);
         else
          a3.diff(&b3,dwork);            
        }
        else
        {
        if(blackOnWhite)
         b2.diff(&a2,work);
        else
         a2.diff(&b2,work);
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
          b2.diff(&a2,dwork);
         else
          a2.diff(&b2,dwork);
        }
        else
        {
         if(blackOnWhite)
          b->diff(a,work);
         else
          a->diff(b,work);
        }
       }
       
       if(medianBlur2)
       {
        if(useDouble)
        {
         dwork->medianBlur(medianBlur2);
        }
        else
        {
         work->medianBlur(medianBlur2);
        }
       }
       if(gaussK2 && gaussK2>0 && gaussK2%2==1)
       { 
        if(!useDouble) {
         (*dwork)=Image<double>(work);
         useDouble=true;
        }
        dwork->gaussianBlur(gaussK2,gaussS2);
       }
       int maxi=0;
       vector<vector<int> > hist;
       if(useDouble)
        hist=dwork->dHistogram(&maxi);
       else
        hist=work->dHistogram(&maxi);
       if(hist.size())
       {
        double threshold=hist[maxi][0]+fracN*(hist[hist.size()-1][0]-hist[maxi][0]);
        double maxdiff=hist[hist.size()-1][0];
        if(threshold<minThreshold+hist[maxi][0])threshold=minThreshold+hist[maxi][0];
        if(useDouble)
         dwork->Threshold(threshold);
        else
         work->Threshold(threshold);        
        cerr<<"Image thresholded with "<<threshold<<"\n";
        vector<vector<double> > centers;
        if(useDouble)
        {
         dwork->erodeAndDilate(erode, dilute);
         centers=dwork->removeSmallBlobsNew(minA,maxA,threshold,maxdiff);
        }
        else
        {
         work->erodeAndDilate(erode, dilute);
         centers=work->removeSmallBlobsNew(minA,maxA,threshold,maxdiff);
        } 
        cerr<<"I got "<<centers.size()<<" points\n";
       }
       Mat R,B;
       if(useDouble)
       {
        R=dwork->image(1);
        B=dwork->image(2);
       }
       else
       {
        R=work->image(1);
        B=work->image(2);       
       }
       Mat G=Mat::zeros(B.size(), B.type());       
       std::vector<cv::Mat> array_to_merge;
       array_to_merge.push_back(B);
       array_to_merge.push_back(G);
       array_to_merge.push_back(R);
       cv::Mat color;
       cv::merge(array_to_merge, color);
       m_player->showImage(color, m_player->getSeqPointer(c,r)->currentFrame(),c,r);
    }
  });

  QMenu* mouseModeList = contextMenu.addMenu(tr("&Mouse Menu"));
  mouseModeList-> addAction(tr("None"), this,[this](){mouseMode=NONE;m_player->setCursorForWidgets(0);});
  mouseModeList-> addAction(tr("Zoom"), this,[this](){mouseMode=ZOOM;m_player->setCursorForWidgets(5);});
  mouseModeList-> addAction(tr("Pan"), this,[this](){mouseMode=PAN;m_player->setCursorForWidgets(2);});
  mouseModeList-> addAction(tr("Add"), this,[this](){mouseMode=ADDPOINT;m_player->setCursorForWidgets(1);});
  mouseModeList-> addAction(tr("Remove"), this,[this](){mouseMode=REMOVEPOINT;m_player->setCursorForWidgets(1);});
  mouseModeList-> addAction(tr("Reset"), this,[this,c,r](){m_player->setZoom(QPoint(0,0),c,r,0,0);});
  mouseModeList-> addAction(tr("Get Value"), this,[this](){mouseMode=DISPLAYPIXELVALUE;m_player->setCursorForWidgets(1);});
  mouseModeList-> addAction(tr("PolygonSelection"), this,[this,c,r](){
      if(seq.size()>0)
      {
      mouseMode=POLYGONSELECTION;
      m_player->setCursorForWidgets(4);
      m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
      showAllPoints=true;
      polygon.clear();
      m_player->clearPolygon(c,r);
      }
  });
  mouseModeList-> addAction(tr("PolygonSelection(inv)"), this,[this,c,r](){
      if(seq.size()>0)
      {
      mouseMode=INVERSEPOLYGONSELECTION;
      m_player->setCursorForWidgets(4);
      m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
      showAllPoints=true;
      polygon.clear();
      m_player->clearPolygon(c,r);
      }
  });

  mouseModeList-> addAction(tr("Select Mosquito"), this,[this,c,r](){
      if(seq.size()>0)
      {
      mouseMode=SELECTMOSQUITO;
      m_player->setCursorForWidgets(1);
      m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
      }
  });
  mouseModeList-> addAction(tr("Setting ROI"), this,[this,c,r](){mouseMode=SELECTROI;m_player->setCursorForWidgets(1);});
  mouseModeList-> addAction(tr("Setting Poly ROI"), this,[this,c,r](){
      if(seq.size()>0)
      {
       mouseMode=SELECTPOLYROI;
       m_player->setCursorForWidgets(1);
       polyROI.clear();
       m_player->clearPolygon(c,r);
      }
  });
  
  mouseModeList-> addAction(tr("Remove Point From Track"), this,[this,c,r](){if(seq.size()>0){m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());showAllPoints=true;}mouseMode=REMOVEPOINTFROMTRACK;m_player->setCursorForWidgets(1);});
  mouseModeList-> addAction(tr("Add Point to track"), this,[this,c,r](){if(seq.size()>0){m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo(),true);showAllPoints=true;}mouseMode=ADDPOINTTOTRACK;m_player->setCursorForWidgets(1);selectedTrack=-1;});
  mouseModeList-> addAction(tr("Remove Track"), this,[this,c,r](){if(seq.size()>0){m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo(),true);showAllPoints=true;}
  mouseMode=REMOVETRACK;m_player->setCursorForWidgets(1);});
  mouseModeList-> addAction(tr("Join tracks"), this,[this,c,r](){if(seq.size()>0){m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo(),true);showAllPoints=true;}mouseMode=JOINTRACKS;m_player->setCursorForWidgets(1);selectedTrack=-1;});
  mouseModeList-> addAction(tr("Split tracks"), this,[this,c,r](){if(seq.size()>0){m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());showAllPoints=true;}mouseMode=SPLITTRACK;m_player->setCursorForWidgets(1);selectedTrack=-1;});
  mouseModeList-> addAction(tr("Remove Point From Track(poly)"), this,[this,c,r](){if(seq.size()>0){m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());showAllPoints=true;}mouseMode=POLYGONSELECTIONFORTRACKS;m_player->setCursorForWidgets(1);});
  mouseModeList-> addAction(tr("Create Track from Points(poly)"), this,[this,c,r](){if(seq.size()>0){m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo(),true);showAllPoints=true;}mouseMode=POLYGONSELECTIONTOTRACK;m_player->setCursorForWidgets(1);});
  
  mouseModeList-> addAction(tr("Edit Polygon"), this,[this](){mouseMode=EDITPOLYGON;m_player->setCursorForWidgets(2);});
  mouseModeList-> addAction(tr("Draw Estimate for Track Start"), this,[this,c,r](){mouseMode=SELECTTRACKFROMESTIMATE;m_player->setCursorForWidgets(1);if(seq.size()>0)updatePointerOfTracks(c,r);});
  mouseModeList-> addAction(tr("Draw Estimate for Track End"), this,[this,c,r](){mouseMode=SELECTTRACKTOESTIMATE;m_player->setCursorForWidgets(1);if(seq.size()>0)updatePointerOfTracks(c,r);});
  mouseModeList-> addAction(tr("Search Mosquito around here"), this,[this,c,r](){mouseMode=SELECTPOINTFORLOCALSEARCH;m_player->setCursorForWidgets(1);});
  mouseModeList-> addAction(tr("Search Mosquito at track start"), this,[this,c,r](){mouseMode=SELECTTRACKFORLOCALBACKWARDSSEARCH;m_player->setCursorForWidgets(1);});
  mouseModeList-> addAction(tr("Search Mosquito inside of track"), this,[this,c,r](){mouseMode=SELECTTRACKFORLOCALINBETWEENSEARCH;m_player->setCursorForWidgets(1);});
  mouseModeList-> addAction(tr("Search Mosquito at track end"), this,[this,c,r](){mouseMode=SELECTTRACKFORLOCALSEARCH;m_player->setCursorForWidgets(1);});

  QMenu* fMode = contextMenu.addMenu(tr("&Functions"));
    //Move to other menu
  fMode-> addAction(tr("auto join tracks"), this,[this,c,r](){
      if(seq.size()>0)
      {
        backupTracks(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
        currReg->autoJoinTracks(trackParam->getMaxDAutoJoin(),trackParam->getMaxGAutoJoin());
          if(showAllPoints)
           m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
        updatePointerOfTracks(c,r);
        updateTableOfTracks();
      }
  });
   fMode-> addAction(tr("Clear Flags"), this,[this,c,r](){
    if(seq.size()>0)
    {
     bool ret=m_player->getMosqPointer(c,r)->resetFlags();
    }
   });

   fMode-> addAction(tr("update Pointer of Tracks"), this,[this,c,r](){
    if(seq.size()>0)
    {
     updatePointerOfTracks(c,r);
    }
   });
   
   fMode-> addAction(tr("Connect Resting Tracks..."), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
     if(m_player->getTrackPointer(c,r))
    {
     m_player->getTrackPointer(c,r)->connectRestingTracks(100, 4.0);
    }
   });

   fMode-> addAction(tr("Fix time in loaded data"), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
    {
     m_player->getMosqPointer(c,r)->giveSeqPointer(m_player->getSeqPointer(c,r));
     bool ret=m_player->getMosqPointer(c,r)->fixTime();
     if(m_player->getTrackPointer(c,r))
      m_player->getTrackPointer(c,r)->fixTime(m_player->getMosqPointer(c,r));
    }
   });

   fMode-> addAction(tr("Remove Points not on Track"), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r)) 
    {
        updatePointerOfTracks(c,r);     
        m_player->getMosqPointer(c,r)->setKeepMeFlag(false);
    }
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      if(m_player->getTrackPointer(c,r))
    {
     m_player->getTrackPointer(c,r)->markMosquitoesToKeep();
     backupPos(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
     m_player->getMosqPointer(c,r)->removeMarkedMosquitoes(frameSelect->getFrom(),frameSelect->getTo(),m_player->useMetaMode());
    }
   });

   fMode-> addAction(tr("Fill In all Tracks"), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      if(m_player->getTrackPointer(c,r))
    {
    backupPos(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
    for(int q=0; q<m_player->getTrackPointer(c,r)->numOfTracks(); q++)
    { 
    double ox;
    double oy;
    unsigned long of;
    vector<vector<TrackPoint> > pieces=m_player->getTrackPointer(c,r)->getMissingPieces(q);
    if(pieces.size()>0)
    {
     cerr<<"I need to loop over "<< pieces.size() << "missing pieces...\n";
     for(int p=0;p<pieces.size();p++)
     {
      cerr<<"piece "<<p<<" has a gap of "<< pieces[p][1].frame-pieces[p][0].frame << "\n";
      for(int of=pieces[p][0].frame+1;of<pieces[p][1].frame; of++)
      {
       cerr<<"checking "<< of<<"\n";
       double l=double(of-pieces[p][0].frame)/double(pieces[p][1].frame-pieces[p][0].frame);
       double ox=pieces[p][0].x+l*(pieces[p][1].x-pieces[p][0].x);
       double oy=pieces[p][0].y+l*(pieces[p][1].y-pieces[p][0].y);

       double radius=15;
       int x0=ox-radius; if(x0<0)x0=0;if(x0>=m_player->getSeqPointer(c,r)->pointerToHeader()->width())x0=m_player->getSeqPointer(c,r)->pointerToHeader()->width()-1;
       int y0=oy-radius; if(y0<0)y0=0;if(y0>=m_player->getSeqPointer(c,r)->pointerToHeader()->height())y0=m_player->getSeqPointer(c,r)->pointerToHeader()->height()-1;
       int x1=ox+radius; if(x1<0)x1=0;if(x1>=m_player->getSeqPointer(c,r)->pointerToHeader()->width())x1=m_player->getSeqPointer(c,r)->pointerToHeader()->width()-1;
       int y1=oy+radius; if(y1<0)y1=0;if(y1>=m_player->getSeqPointer(c,r)->pointerToHeader()->height())y1=m_player->getSeqPointer(c,r)->pointerToHeader()->height()-1;   
       cv::Rect croi=cv::Rect((int)x0,(int)y0, (int)(x1-x0+1),(int) (y1-y0+1));
       cerr << "Searching ROI:"<<croi<< "for frame "<<of<<"\n";
       double maxdiff;
       vector<vector<double> > centers=segmentROI(m_player->getSeqPointer(c,r),croi,of,maxdiff);
       if(maxdiff>0)
       {
        MosquitoesInFrame* currentFrame=m_player->getMosqPointer(c,r)->mosqInFrameI(of);
        if(currentFrame)
        {
        if(currentFrame->getFrameNo()==0 || currentFrame->getTsSec()==0)
        {
         currentFrame->setFrameNo(of);
         vector<int> t=m_player->getSeqPointer(c,r)->timeOfFrameI(of);
         if(t.size()==3)
         {
          currentFrame->setTsSec(t[0]);
          currentFrame->setTsMSec(t[1]);
          currentFrame->setTsUSec(t[2]);
         }
        }
        vector<QPointF> proi;
        proi.push_back(QPointF(x0,y0));
        proi.push_back(QPointF(x1,y0));
        proi.push_back(QPointF(x1,y1));
        proi.push_back(QPointF(x0,y1));
        proi.push_back(QPointF(x0,y0));
        vector<cv::Point2f> omosqs=currentFrame->pointsInsideOfPolygon(proi);
        if(omosqs.size()>0)
        {
         cerr<<"I stop here, already have points in roi...\n";
        }
        else
        {
        cerr<<"Adding "<<centers.size()<<" Mosquitoes... To Frame "<< currentFrame->getFrameNo()<<" with time "<<currentFrame->getTsSec() <<"."<<currentFrame->getTsMSec()<<":"<<currentFrame->getTsUSec()<<"\n";
        for(unsigned long i=0; i<centers.size(); i++)
        {
            vector<cv::Point> c;
            if(centers[i].size()>8)
            {
             for(unsigned long j=0; j<centers[i][8]; j++)
              if(9+j*2+1<centers[i].size())
             {
               cv::Point tmp=cv::Point(centers[i][9+j*2],centers[i][9+j*2+1]);
               c.push_back(tmp);
             }
             currentFrame->addMosquito(centers[i][0], centers[i][1], centers[i][2], centers[i][3],
                                   centers[i][4],centers[i][5],centers[i][6],centers[i][7],c);       
            }
            else
            {
              currentFrame->addMosquito(centers[i][0], centers[i][1], centers[i][2], centers[i][3],
                                   centers[i][4],centers[i][5],centers[i][6],centers[i][7]);
            }
        }
        if(centers.size()==1)
        {
         vector<unsigned long> tmp=m_player->getMosqPointer(c,r)->findMosquito(QPointF(centers[0][0],centers[0][1]),of,of+1);
         if(tmp.size()==2)
         {
          cerr<<"This is Mosquito "<<tmp[1]<<" in frame "<<tmp[0]<<"\n";
          MosquitoesInFrame* cF=m_player->getMosqPointer(c,r)->mosqInFrameI(tmp[0]);
          if(cF)
          {
          MosquitoPosition* cM=cF->mosqi(tmp[1]);
          if(cM)
          {
           TrackPoint NewTrackPoint(cM->getX(),cM->getY(),tmp[0],cF->getTsSec(),cF->getTsMSec(),cF->getTsUSec(),
                            cM->getI(),cM->getA(),cF->getMaxDiff(),cF->getMinArea(),cF->getMaxArea(),
                            cF->getThreshold(),cF->getMinThreshold(),cF->getIth());
           m_player->getTrackPointer(c,r)->addPointToTrack(q, NewTrackPoint);
           updatePointerOfTracks(c,r);
           pieces[p][0]=NewTrackPoint;
          }
          }  
         }       
        }
        }  
        }
        else
        {
         cerr<<"CurrentFrame not valid...\n";
        }
       }    
       
      }
     }
    }
    else
    {
       cerr<<"Track has not missing pieces or is invalid...\n";     
    }
   }
   }
   });

   fMode-> addAction(tr("Save ROI/polyROI to File"), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
    {
     QFileDialog dialog(this);
     dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
     dialog.setFileMode(QFileDialog::AnyFile);
     dialog.setNameFilter(tr("OpenCV File Storage (*.yml)"));
     QStringList fileNames;
     vector<cv::Point> poly=m_player->getSeqPointer(c,r)->getPolyRoi(); 
     if (dialog.exec())
        fileNames = dialog.selectedFiles();
     for (int i = 0; i < fileNames.size(); ++i)
     {
       savePolyToFile(fileNames.at(i),poly);
     }
    }
   });

   fMode-> addAction(tr("Save Segmentation Settings to File"), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
    {
     QFileDialog dialog(this);
     dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
     dialog.setFileMode(QFileDialog::AnyFile);
     dialog.setNameFilter(tr("OpenCV File Storage (*.yml)"));
     QStringList fileNames;
     vector<cv::Point> poly=m_player->getSeqPointer(c,r)->getPolyRoi(); 
     if (dialog.exec())
        fileNames = dialog.selectedFiles();
     for (int i = 0; i < fileNames.size(); ++i)
     {
       savePolyToFile(fileNames.at(i),poly);
     }
    }
   });

   fMode-> addAction(tr("Load polyROI from File"), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
    {
     QFileDialog dialog(this);
     dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
     dialog.setFileMode(QFileDialog::ExistingFiles);
     dialog.setNameFilter(tr("OpenCV File Storage (*.yml)"));
     QStringList fileNames;
     vector<cv::Point> poly=m_player->getSeqPointer(c,r)->getPolyRoi(); 
     if (dialog.exec())
        fileNames = dialog.selectedFiles();
     for (int i = 0; i < fileNames.size(); ++i)
     {
       vector<cv::Point> poly=loadPolyFromFile(fileNames.at(i));
       for(int j=0; j<poly.size(); j++)
        m_player->addToPolygon(poly[j].x,poly[j].y,c,r);
       m_player->addToPolygon(poly[0].x,poly[0].y,c,r);
       m_player->getSeqPointer(c,r)->setPolyRoi(poly);   
     }
    }
   });

   fMode-> addAction(tr("Save Segmentation Parameters to File"), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
    {
     QFileDialog dialog(this);
     dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
     dialog.setFileMode(QFileDialog::AnyFile);
     dialog.setAcceptMode(QFileDialog::AcceptSave);
     dialog.setNameFilter(tr("OpenCV File Storage (*.yml)"));
     QStringList fileNames;
     if (dialog.exec())
        fileNames = dialog.selectedFiles();
     for (int i = 0; i < fileNames.size(); ++i)
     {
      FileStorage fs( fileNames.at(i).toUtf8().constData(), FileStorage::WRITE );
      fs << "minArea" << minA;
      fs << "maxArea" << maxA;
      fs << "fracN" << fracN;
      fs << "minThreshold" << minThreshold;
      fs << "useIth" << useIth;
      fs << "erode" << erode;
      fs << "dilute" << dilute;
      fs << "blackOnWhite" << blackOnWhite;
      int m1=segParam->getMedianBlur1Param();
      fs << "medianBlur1" << m1;
      int m2=segParam->getMedianBlur2Param();
      fs << "medianBlur2" << m2;
      int g1=segParam->getGaussianBlur1Param();
      fs << "gaussK1" << g1;
      int g2=segParam->getGaussianBlur2Param();
      fs << "gaussK2" << g2;
      double s1=segParam->getGaussianSigma1Param();
      fs << "gaussS1" << s1;
      double s2=segParam->getGaussianSigma2Param();
      fs << "gaussS2" << s2;
      int maskT=segParam->getMaskThreshold();
      fs << "maskT" << maskT; 
     }
    }
   });

   fMode-> addAction(tr("Load Segmentation Parameters form File"), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
    {
     QFileDialog dialog(this);
     dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
     dialog.setFileMode(QFileDialog::AnyFile);
     dialog.setNameFilter(tr("OpenCV File Storage (*.yml)"));
     QStringList fileNames;
     if (dialog.exec())
        fileNames = dialog.selectedFiles();
     for (int i = 0; i < fileNames.size(); ++i)
     {
      FileStorage fs( fileNames.at(i).toUtf8().constData(), FileStorage::READ );
      fs["minArea"] >> minA;
      fs["maxArea"] >> maxA;
      fs["fracN"] >> fracN;
      fs["minThreshold"] >> minThreshold;
      fs["useIth"] >> useIth;
      fs["erode"] >> erode;
      fs["dilute"] >> dilute;
      fs["blackOnWhite"] >> blackOnWhite;
      int m1,m2,g1,g2,maskT;
      double s1,s2;
      fs["medianBlur1"] >> m1;
      fs["medianBlur2"] >> m2;
      fs["gaussK1"] >> g1;
      fs["gaussK2"] >> g2;
      fs["gaussS1"] >> s1;
      fs["gaussS2"] >> s2;
      fs["maskT"] >> maskT; 
      
      segParam->setMinA(minA);
      segParam->setMaxA(maxA);
      segParam->setFracN(fracN);
      segParam->setMThreshold(minThreshold);
      segParam->setUseIth(useIth);
      segParam->setDilute(dilute);
      segParam->setErode(erode);
      segParam->setBlackOnWhite(blackOnWhite);
      segParam->setMedianBlur1Param(m1);
      segParam->setMedianBlur2Param(m2);
      segParam->setGaussianBlur1Param(g1);
      segParam->setGaussianBlur2Param(g2);
      segParam->setGaussianSigma1Param(s1);
      segParam->setGaussianSigma2Param(s2);
      segParam->setMaskThreshold(maskT);    
     }
    }
   });

   fMode-> addAction(tr("Remove too short tracks"), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
      if(m_player->getTrackPointer(c,r))
    {
     bool ok=false;
     int x = QInputDialog::getInt(this, tr("QInputDialog::getInt()"),
                                       tr("Min Track Length:"), 3, 0, 10000, 1, &ok);
     if(ok)
     {
         int rem=m_player->getTrackPointer(c,r)->removeTooShort(x); 
         cerr<<"I have removed "<<rem<<" too short tracks...\n";
         m_player->getTrackPointer(c,r)->updateMosquitoPointer(m_player->getMosqPointer(c,r));
     }
     else
     {
         cerr<<"Input not valid! Using 2.0\n";
     }    
    }
   });

   fMode-> addAction(tr("Clean Cluster(currentFrame)"), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
    {
     bool ok=false;
     int x = QInputDialog::getInt(this, tr("QInputDialog::getInt()"),
                                       tr("Min Cluster Size:"), 3, 0, 10000, 1, &ok);
     bool ok2=false;
     double x2 = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Ministance:"), 32.000, 0, 10000., 2, &ok2);
     if(ok && ok2)
     {
         MosquitoesInFrame* currentFrame;
         currentFrame=currMosqPos->mosqInFrameI(m_player->getSeqPointer(c,r)->currentFrame());
         if(currentFrame)
         {
          int n=currentFrame->cleanClusters(x2,x);
          cerr<<"Removes "<<n<<"Mosquitoes from current Frame...\n";
         }
     }
     else
     {
         cerr<<"Input not valid!\n";
     }    
    }
   });

   fMode-> addAction(tr("Show Cluster(currentFrame)"), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
    {
     bool ok=false;
     int x = QInputDialog::getInt(this, tr("QInputDialog::getInt()"),
                                       tr("Min Cluster Size:"), 3, 0, 10000, 1, &ok);
     bool ok2=false;
     double x2 = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Ministance:"), 32.000, 0, 10000., 2, &ok2);
     if(ok && ok2)
     {
         MosquitoesInFrame* currentFrame;
         currentFrame=m_player->getMosqPointer(c,r)->mosqInFrameI(m_player->getSeqPointer(c,r)->currentFrame());
         if(currentFrame)
         {
          vector<vector<int> > n=currentFrame->getClusters(x2,x);
          cerr<<"Showing"<<n.size()<<"Cluster in current Frame...\n";
          Image<unsigned char>* timg = m_player->getSeqPointer(c,r)->getImagePointer(0);
          unsigned char* tmpdata = timg->bufferPointer();
          Mat A = Mat(timg->getY(), timg->getX(), CV_8UC1, &tmpdata[0]);
          Mat B;
          cvtColor(A, B, COLOR_GRAY2RGB);
          RNG rng(12345);
          for(unsigned int i=0; i<n.size(); i++)
          {
           Scalar color = Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
           cerr<<"Cluster "<<i<<" has color:"<<color<<"\n";
           for(unsigned int j=0; j<n[i].size(); j++)
           {
            MosquitoPosition* m=currentFrame->mosqi(n[i][j]);
            if(m)
             circle(B, Point2f(m->getX(),m->getY()), 10,color);
           }
          }
          m_player->showImage(B,m_player->getCurrentFrameNo(),c,r);
         }
     }
     else
     {
         cerr<<"Input not valid!\n";
     }    
    }
   });

   fMode-> addAction(tr("Clean Cluster(selection)"), this,[this,c,r](){
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
    {
     bool ok=false;
     int x = QInputDialog::getInt(this, tr("QInputDialog::getInt()"),
                                       tr("Min Cluster Size:"), 3, 0, 10000, 1, &ok);
     bool ok2=false;
     double x2 = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Ministance:"), 32.000, 0, 10000., 2, &ok2);
     if(ok && ok2)
     {
         MosquitoesInFrame* currentFrame;
         for(int i=frameSelect->getFrom();i<=frameSelect->getTo(); i++)
         {
          currentFrame=currMosqPos->mosqInFrameI(i);
          if(currentFrame)
           if(currentFrame->numOfMosquitoes()>1)
          {
          int n=currentFrame->cleanClusters(x2,x);
          cerr<<"From Frame "<< i<<" I removed "<<n<<"Mosquitoes...\n";
          }
         }
     }
     else
     {
         cerr<<"Input not valid!\n";
     }    
    }
   });
 
  QMenu* filterMode = contextMenu.addMenu(tr("&Filter Points by"));
  filterMode-> addAction(tr("minArea"), this,[this,c,r](){
   if(seq.size()>0)
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      {
        bool ok=false;
        double v = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("MinArea:"), 2.000, 0.0, 1000000000.0, 2, &ok);
        if(ok)
        {
         m_player->getMosqPointer(c,r)->filterByArea(v);
        }
      }
  });
  filterMode-> addAction(tr("maxArea"), this,[this,c,r](){
   if(seq.size()>0)
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      {
        bool ok=false;
        double v = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("MaxArea:"), 2.000, 0.0, 1000000000.0, 2, &ok);
        if(ok)
        {
         m_player->getMosqPointer(c,r)->filterByArea(v,true);
        }
      }
  });

  filterMode-> addAction(tr("min maxIntensity"), this,[this,c,r](){
   if(seq.size()>0)
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      {
        bool ok=false;
        double v = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Min Intenity:"), 2.000, 0.0, 1000000000.0, 2, &ok);
        if(ok)
        {
         m_player->getMosqPointer(c,r)->filterByMaxInt(v);
        }
      }
  });

  filterMode-> addAction(tr("max maxIntensity"), this,[this,c,r](){
   if(seq.size()>0)
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      {
        bool ok=false;
        double v = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Max Intenity:"), 2.000, 0.0, 1000000000.0, 2, &ok);
        if(ok)
        {
         m_player->getMosqPointer(c,r)->filterByMaxInt(v,true);
        }
      }
  });

  filterMode-> addAction(tr("min Circularity"), this,[this,c,r](){
   if(seq.size()>0)
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      {
        m_player->getMosqPointer(c,r)->contourStatistics();
        bool ok=false;
        double v = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Min Circularity:"), 0.500, 0.0, 1.0, 2, &ok);
        if(ok)
        {
         m_player->getMosqPointer(c,r)->filterByCircularity(v);
        }
      }
  });

  filterMode-> addAction(tr("max Circularity"), this,[this,c,r](){
   if(seq.size()>0)
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      {
        m_player->getMosqPointer(c,r)->contourStatistics();
        bool ok=false;
        double v = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Max Circularity:"), 0.500, 0.0, 1.0, 2, &ok);
        if(ok)
        {
         m_player->getMosqPointer(c,r)->filterByCircularity(v,true);
        }
      }
  });

  filterMode-> addAction(tr("min Inertia"), this,[this,c,r](){
   if(seq.size()>0)
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      {
        m_player->getMosqPointer(c,r)->contourStatistics();
        bool ok=false;
        double v = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Min Inertia:"), 0.500, 0.0, 1.0, 2, &ok);
        if(ok)
        {
         m_player->getMosqPointer(c,r)->filterByInertia(v);
        }
      }
  });

  filterMode-> addAction(tr("max Inertia"), this,[this,c,r](){
   if(seq.size()>0)
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      {
        m_player->getMosqPointer(c,r)->contourStatistics();
        bool ok=false;
        double v = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Max Inertia:"), 0.500, 0.0, 1.0, 2, &ok);
        if(ok)
        {
         m_player->getMosqPointer(c,r)->filterByInertia(v,true);
        }
      }
  });

  filterMode-> addAction(tr("min Convexity"), this,[this,c,r](){
   if(seq.size()>0)
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      {
        m_player->getMosqPointer(c,r)->contourStatistics();
        bool ok=false;
        double v = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Min Convexity:"), 0.500, 0.0, 1.0, 2, &ok);
        if(ok)
        {
         m_player->getMosqPointer(c,r)->filterByConvexity(v);
        }
      }
  });

  filterMode-> addAction(tr("max Convexity"), this,[this,c,r](){
   if(seq.size()>0)
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      {
        m_player->getMosqPointer(c,r)->contourStatistics();
        bool ok=false;
        double v = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Max Convexity:"), 0.500, 0.0, 1.0, 2, &ok);
        if(ok)
        {
         m_player->getMosqPointer(c,r)->filterByConvexity(v,true);
        }
      }
  });

  filterMode-> addAction(tr("min Radius"), this,[this,c,r](){
   if(seq.size()>0)
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      {
        m_player->getMosqPointer(c,r)->contourStatistics();
        bool ok=false;
        double v = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Min Radius:"), 2.000, 0.0, 1000000000.0, 2, &ok);
        if(ok)
        {
         m_player->getMosqPointer(c,r)->filterByRadius(v);
        }
      }
  });

  filterMode-> addAction(tr("max Radius"), this,[this,c,r](){
   if(seq.size()>0)
    if(m_player->getSeqPointer(c,r))
     if(m_player->getMosqPointer(c,r))
      {
        m_player->getMosqPointer(c,r)->contourStatistics();
        bool ok=false;
        double v = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
                                       tr("Max Radius:"), 2.000, 0.0, 1000000000.0, 2, &ok);
        if(ok)
        {
         m_player->getMosqPointer(c,r)->filterByRadius(v,true);
        }
      }
  });
  contextMenu.exec(m);
}
