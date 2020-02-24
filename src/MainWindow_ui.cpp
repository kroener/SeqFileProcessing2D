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
#include "GraphWidget.h"
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
#include <QToolBar>
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

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu-> addAction("Open", this, SLOT( on_actionOpen_triggered() ) );
    fileMenu->addSeparator();
    for( int i =0; i< MaxRecentFiles; ++i)
    {
     QAction *tmpAct=fileMenu->addAction("foo",this, SLOT(openRecentFile()) );
     recentFileActs.push_back(tmpAct);
     recentFileActs[i]->setVisible(false);
    }
    fileMenu->addSeparator();
    loadPositionsAct=fileMenu-> addAction("Open Positions", this, SLOT( on_actionOpenPositions_triggered() ) );
    loadPositionsRangeAct=fileMenu-> addAction("Open Positions(only selection)", this, SLOT( on_actionOpenPositionsRange_triggered() ) );
    loadDynamicPositionsAct=fileMenu-> addAction("Open Positions(dynamic)", this, SLOT( on_actionOpenDynamicPositions_triggered() ) );
    savePositionsAct=fileMenu-> addAction("Save Positions", this, SLOT( on_actionSavePositions_triggered() ) );
    savePositions4Act=fileMenu-> addAction("Save Positions(selection)", this, SLOT( on_actionSavePositionsSelection_triggered() ) );
    loadTracksAct=fileMenu-> addAction("Open Tracks", this, SLOT( on_actionOpenTracks_triggered() ) );
    saveTracksAct=fileMenu-> addAction("Save Tracks", this, SLOT( on_actionSaveTracks_triggered() ) );
    closeAct=fileMenu-> addAction("Close", this, SLOT( on_actionClose_triggered() ) );
    loadProjectAct=fileMenu-> addAction("Open Project", this, SLOT( on_actionOpenProject_triggered() ) );
    saveProjectAct=fileMenu-> addAction("Save Project", this, SLOT( on_actionSaveProject_triggered() ) );
    fileMenu-> addAction("Quit", qApp, SLOT( quit() ) );
    loadPositionsAct->setEnabled(false);
    loadPositionsRangeAct->setEnabled(false);
    loadDynamicPositionsAct->setEnabled(false);
    savePositionsAct->setEnabled(false);
    saveProjectAct->setEnabled(false);
    savePositions4Act->setEnabled(false);
    loadTracksAct->setEnabled(false);
    saveTracksAct->setEnabled(false);
    closeAct->setEnabled(false);
    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu-> addAction("From (this)", this, SLOT( on_actionFrom_triggered() ) );
    editMenu-> addAction("To (this)", this, SLOT( on_actionTo_triggered() ) );
    editMenu-> addAction("Save png (this)", this, SLOT( on_actionSavePGM_triggered() ) );
    editMenu-> addAction("Save png (selection)", this, SLOT( on_actionSavePGMSelection_triggered() ) );
    editMenu-> addAction("Save png (+metadata)", this, SLOT( on_actionSavePGMMetadata_triggered() ) );
    editMenu-> addAction("Copy settings to All", this, SLOT( on_actionCopySettings_triggered() ) );
    editMenu-> addAction("Process All", this, SLOT( on_actionProcessAll_triggered() ) );
    editMenu-> addAction("Show Histogram", this, SLOT( on_actionShowHist_triggered() ) );
    editMenu-> addAction("Show Histogram of Img.", this, SLOT( on_actionShowHistOfImage_triggered() ) );
    editMenu-> addAction("Show Segmentation", this, SLOT( on_actionShowDiffImage_triggered() ) );
    editMenu-> addAction("Show All Points", this, SLOT( on_actionShowPoints_triggered() ) );
    editMenu-> addAction("Clean Cluster (current frame)", this, SLOT( on_actionCleanCluster_triggered() ) );
    editMenu-> addAction("Clean Cluster (all frames)", this, SLOT( on_actionCleanAllCluster_triggered() ) );
    editMenu-> addAction("Reset Positions", this, SLOT( on_actionResetPositions_triggered() ) );
    editMenu-> addAction("Reset Tracks", this, SLOT( on_actionResetTracks_triggered() ) );
    editMenu-> addAction("Show Table of Points", this, SLOT( on_actionShowTable_triggered() ) );
    editMenu-> addAction("Show Table of all P.", this, SLOT( on_actionShowTableOfAll_triggered() ) );
    editMenu-> addAction("Show Table of Tracks", this, SLOT( on_actionShowTableOfTracks_triggered() ) );
    editMenu-> addAction("Save Custom Settings", this, SLOT( saveSettingsAsDefault() ) );
    editMenu-> addAction("Remove Custom Settings", this, SLOT( clearCustomSettings() ) );
    editMenu-> addAction("Custom Settings to File", this, SLOT( customSettingsToFile() ) );
    editMenu-> addAction("Custom Settings from File", this, SLOT( customSettingsFromFile() ) );
    editMenu->setEnabled(false);
    viewMenu = menuBar()->addMenu(tr("&View"));
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu-> addAction("&About", this, SLOT( on_about_triggered() ) );
    createToolBar();
}

void MainWindow::createDockWindows()
{
    QDockWidget *dock = new QDockWidget(tr("Sequence Info"), this);
    dock->setObjectName("Sequence Info");
    dock->setAllowedAreas(Qt::RightDockWidgetArea);
    seqInfo = new SeqInfoWidget();
    dock->setWidget(seqInfo);
    addDockWidget(Qt::RightDockWidgetArea,dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("Frame Selection"), this);
    dock->setObjectName("Frame Selection");
    dock->setAllowedAreas(Qt::RightDockWidgetArea);
    frameSelect = new FrameSelectWidget();
    dock->setWidget(frameSelect);
    addDockWidget(Qt::RightDockWidgetArea,dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("Segmentation Parameter"), this);
    dock->setObjectName("SegmentationParameter");
    dock->setAllowedAreas(Qt::RightDockWidgetArea);
    segParam = new SegParameterWidget();
    dock->setWidget(segParam);
    addDockWidget(Qt::RightDockWidgetArea,dock);
    viewMenu->addAction(dock->toggleViewAction());
    
   dock = new QDockWidget(tr("Histogram"), this);
   dock->setObjectName("Histogram");
   dock->setAllowedAreas(Qt::RightDockWidgetArea);
   Histogram = new HistogramWidget();
   dock->setWidget(Histogram);
   addDockWidget(Qt::RightDockWidgetArea,dock);
   viewMenu->addAction(dock->toggleViewAction());
   dock->hide();
   HistoDock=dock;

   dock = new QDockWidget(tr("Graph"), this);
    dock->setObjectName("Graph");
   dock->setAllowedAreas(Qt::RightDockWidgetArea);
   Graph = new GraphWidget();
   dock->setWidget(Graph);
   addDockWidget(Qt::RightDockWidgetArea,dock);
   viewMenu->addAction(dock->toggleViewAction());
   dock->hide();
   GraphDock=dock;
   
    dock = new QDockWidget(tr("Image Viewer"), this);
    dock->setObjectName("Image Viewer");
    dock->setAllowedAreas(Qt::RightDockWidgetArea);
    ImgViewer = new ViewerWithMouse();
    dock->setWidget(ImgViewer);
    addDockWidget(Qt::RightDockWidgetArea,dock);
    viewMenu->addAction(dock->toggleViewAction());
    dock->hide();
    ImgViewerDock=dock;

    dock = new QDockWidget(tr("Tracking Parameter"), this);
    dock->setObjectName("Tracking Parameter");
    dock->setAllowedAreas(Qt::RightDockWidgetArea);
    trackParam = new TrackingParameterWidget();
    dock->setWidget(trackParam);
    addDockWidget(Qt::RightDockWidgetArea,dock);
    viewMenu->addAction(dock->toggleViewAction());
    dock->hide();
    
    dock = new QDockWidget(tr("Display Settings"), this);
    dock->setObjectName("Display Settings");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    displayParam = new DisplaySettingsParameterWidget();
    dock->setWidget(displayParam);
    addDockWidget(Qt::LeftDockWidgetArea,dock);
    viewMenu->addAction(dock->toggleViewAction());
    dock->hide();

}

void MainWindow::displayTable()
{
 if(!m_pTableWidget)
 {
  m_pTableWidget = new QTableWidget();
  connect(m_pTableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(pTableClicked(int,int)));
  connect(m_pTableWidget,SIGNAL(cellActivated(int,int)),this,SLOT(pTableClicked(int,int)));
  connect(m_pTableWidget,SIGNAL(customContextMenuRequested(const QPoint &) ), this, SLOT( on_pTable_ContextMenuRequest(const QPoint &) ) );
  m_pTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
 }
 unsigned long f = m_player->getCurrentFrameNo();
  initTable(currMosqPos->MosqsInFrame(f));
 fillTable(f);
}

void MainWindow::initTable(int n)
{
 m_pTableWidget->setRowCount(n);
 m_pTableWidget->setColumnCount(9);
 m_TableHeader<<"X"<<"Y"<<"max Intensity"<<"Area"<<"MaxDiff"<<"Frame"<<"Sec"<<"MSec"<<"USec";
 m_pTableWidget->setHorizontalHeaderLabels(m_TableHeader);
 m_pTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::fillTable(unsigned long n)
{
 MosquitoesInFrame* currentFrame;
 MosquitoPosition* currentMosquito;
 currentFrame=currMosqPos->mosqInFrameI(n);
 if(currentFrame)
 {
 m_pTableWidget->setSortingEnabled(false);
 for(int i=0; i<currentFrame->numOfMosquitoes(); i++)
 {
     currentMosquito=currentFrame->mosqi(i);
     if(currentMosquito)
     {
     m_pTableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(currentMosquito->getX())));
     m_pTableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(currentMosquito->getY())));
     m_pTableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(currentMosquito->getI())));
     m_pTableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(currentMosquito->getA())));
     m_pTableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(currentFrame->getMaxDiff())));
     m_pTableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(currentFrame->getF())));
     m_pTableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(currentFrame->getTsSec())));
     m_pTableWidget->setItem(i, 7, new QTableWidgetItem(QString::number(currentFrame->getTsMSec())));
     m_pTableWidget->setItem(i, 8, new QTableWidgetItem(QString::number(currentFrame->getTsUSec())));
     }
 }
 }
 m_pTableWidget->setSortingEnabled(true);
}

void MainWindow::displayTableOfAll()
{
 if(!m_pTableWidget)
 {
  m_pTableWidget = new QTableWidget();
  connect(m_pTableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(pTableClicked(int,int)));
  connect(m_pTableWidget,SIGNAL(cellActivated(int,int)),this,SLOT(pTableClicked(int,int)));
  connect(m_pTableWidget,SIGNAL(customContextMenuRequested(const QPoint &) ), this, SLOT( on_pTable_ContextMenuRequest(const QPoint &) ) );
  m_pTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
 }
 initTableOfAll(currMosqPos->getNumOfAllPos());
 cerr<<"Initalize table for "<<currMosqPos->getNumOfAllPos()<<" Positions...\n";
 fillTableOfAll();
}

void MainWindow::initTableOfAll(int n)
{
 m_pTableWidget->setRowCount(n);
 m_pTableWidget->setColumnCount(9);
 m_TableHeader<<"X"<<"Y"<<"max Intensity"<<"Area"<<"MaxDiff"<<"Frame"<<"Sec"<<"MSec"<<"USec";
 m_pTableWidget->setHorizontalHeaderLabels(m_TableHeader);
 m_pTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::fillTableOfAll()
{
 MosquitoesInFrame* currentFrame;
 MosquitoPosition* currentMosquito;
 m_pTableWidget->setSortingEnabled(false);
 int ii=0;
 for(unsigned long n=0; n<currMosqPos->getNoOfFrames(); n++)
 {
 currentFrame=currMosqPos->mosqInFrameI(n,true);
 if(currentFrame)
 {
  for(int i=0; i<currentFrame->numOfMosquitoes(); i++)
  {
     currentMosquito=currentFrame->mosqi(i);
     QTableWidgetItem *item = new QTableWidgetItem;
     item->setData(Qt::EditRole, currentMosquito->getX());
     m_pTableWidget->setItem(ii, 0, item);
     item = new QTableWidgetItem;
     item->setData(Qt::EditRole, currentMosquito->getY());
     m_pTableWidget->setItem(ii, 1, item);
     item = new QTableWidgetItem;
     item->setData(Qt::EditRole, currentMosquito->getI());
     m_pTableWidget->setItem(ii, 2, item);
     item = new QTableWidgetItem;
     item->setData(Qt::EditRole, currentMosquito->getA());
     m_pTableWidget->setItem(ii, 3, item);
     item = new QTableWidgetItem;
     item->setData(Qt::EditRole, currentFrame->getMaxDiff());
     m_pTableWidget->setItem(ii, 4, item);
     item = new QTableWidgetItem;
     item->setData(Qt::EditRole, (unsigned int)currentFrame->getF());
     m_pTableWidget->setItem(ii, 5, item);
     item = new QTableWidgetItem;
     item->setData(Qt::EditRole, currentFrame->getTsSec());
     m_pTableWidget->setItem(ii, 6, item);
     item = new QTableWidgetItem;
     item->setData(Qt::EditRole, currentFrame->getTsMSec());
     m_pTableWidget->setItem(ii, 7, item);
     item = new QTableWidgetItem;
     item->setData(Qt::EditRole, currentFrame->getTsUSec());
     m_pTableWidget->setItem(ii, 8, item);
     ii++;
  }
 }
 }
 m_pTableWidget->setSortingEnabled(true);
}

void MainWindow::displayTableOfTracks()
{
 if(!m_tTableWidget)
 {
  m_tTableWidget = new QTableWidget();
  connect(m_tTableWidget,SIGNAL(cellActivated(int,int)),this,SLOT(tTableClicked(int,int)));
  connect(m_tTableWidget,SIGNAL(customContextMenuRequested(const QPoint &) ), this, SLOT( on_tTable_ContextMenuRequest(const QPoint &) ) );
  connect(m_tTableWidget,SIGNAL(itemSelectionChanged() ), this, SLOT( tTableSelectionChanged() ) );
  connect(m_tTableWidget,SIGNAL(currentCellChanged(int, int, int, int)),this,SLOT(tTableCellChanged(int,int,int,int)));
  m_tTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
 }
 initTableOfTracks(currReg->numOfTracks());
 fillTableOfTracks();
}

void MainWindow::initTableOfTracks(int n)
{
 m_tTableWidget->setRowCount(0);
 m_tTableWidget->setRowCount(n);
 m_tTableWidget->setColumnCount(5);
 m_tTableHeader<<"ID"<<"Length"<<"Start"<<"Stop"<<"Num Points";
 m_tTableWidget->setHorizontalHeaderLabels(m_tTableHeader);
 m_tTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::fillTableOfTracks()
{
 
 m_tTableWidget->setSortingEnabled(false);
 int ii=0;
 vector<vector<TrackPoint> >  tmpReg= currReg->getRegistry();
 for(unsigned long n=0; n<tmpReg.size(); n++)
 {
     int nPoints=tmpReg[n].size();
     int id=n;
     int frameStart=tmpReg[n][0].frame;
     int frameStop=tmpReg[n][nPoints-1].frame;
     int durration=frameStop-frameStart;
     QTableWidgetItem *item = new QTableWidgetItem;
     item->setData(Qt::EditRole, id);
     m_tTableWidget->setItem(ii, 0, item);
     item = new QTableWidgetItem;
     item->setData(Qt::EditRole, durration);
     m_tTableWidget->setItem(ii, 1, item);
     item = new QTableWidgetItem;
     item->setData(Qt::EditRole, frameStart);
     m_tTableWidget->setItem(ii, 2, item);
     item = new QTableWidgetItem;
     item->setData(Qt::EditRole, frameStop);
     m_tTableWidget->setItem(ii, 3, item);
     item = new QTableWidgetItem;
     item->setData(Qt::EditRole, nPoints);
     m_tTableWidget->setItem(ii, 4, item);
     ii++;
 }
 m_tTableWidget->setSortingEnabled(true);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if(event->timerId()==timerID2)
    {
    if(workFrame<=toFrame)
    {
        currSeq->loadImage(workFrame);
        MosquitoesInFrame* currentFrame=currMosqPos->mosqInFrameI(workFrame);
        int m1=segParam->getMedianBlur1Param();
        int m2=segParam->getMedianBlur2Param();
        int g1=segParam->getGaussianBlur1Param();
        int g2=segParam->getGaussianBlur2Param();
        double s1=segParam->getGaussianSigma1Param();
        double s2=segParam->getGaussianSigma2Param();
        int maskT=segParam->getMaskThreshold();

         if(currSeq->getRoi())
         {
             Rect tmp=*currSeq->getRoi();
             vector<QPointF> tmpPoly;
             tmpPoly.push_back(QPointF(tmp.x,tmp.y));
             tmpPoly.push_back(QPointF(tmp.x+tmp.width,tmp.y));
             tmpPoly.push_back(QPointF(tmp.x+tmp.width,tmp.y+tmp.height));
             tmpPoly.push_back(QPointF(tmp.x,tmp.y+tmp.height));            
             tmpPoly.push_back(QPointF(tmp.x,tmp.y));
             currentFrame->removePointsInsideOfPolygon(tmpPoly);
         }
         else if(currSeq->gotPolyRoi())
         {
             vector<cv::Point> tmp=currSeq->getPolyRoi();
             vector<QPointF> tmpPoly;
             for(int i=0; i<tmp.size(); i++)
              tmpPoly.push_back(QPointF(tmp[i].x,tmp[i].y));
             currentFrame->removePointsInsideOfPolygon(tmpPoly);
         }
         else
         {
          currentFrame->clear();
         }
        currentFrame->setMinArea(minA);
        currentFrame->setMaxArea(maxA);
        currentFrame->setThreshold(fracN);
        currentFrame->setMinThreshold(minThreshold);
        currentFrame->setFrameNo(workFrame);
        currentFrame->setTsSec(currSeq->sec(0));
        currentFrame->setTsMSec(currSeq->ms(0));
        currentFrame->setTsUSec(currSeq->us(0));
        if(currSeq->getRoi())
        {
            Rect tmp=*currSeq->getRoi();
            worker->process(currSeq->getImagePointer(0), currSeq->getImagePointer(useIth), minA, maxA,fracN,minThreshold,erode,dilute,blackOnWhite,tmp,m1,m2,g1,s1,g2,s2,vector<cv::Point>(),maskT);
        }
        else if(currSeq->gotPolyRoi())
        {
            vector<cv::Point> polyRoi=currSeq->getPolyRoi();
            vector<cv::Point> BB=polyBB(polyRoi, cv::Point2f(0,0),1.0);
            cv::Rect tmp=cv::Rect(BB[0].x,BB[0].y,BB[1].x-BB[0].x,BB[1].y-BB[0].y);
            worker->process(currSeq->getImagePointer(0), currSeq->getImagePointer(useIth), minA, maxA,fracN,minThreshold,erode,dilute,blackOnWhite,tmp,m1,m2,g1,s1,g2,s2,polyRoi,maskT);
        }
        else 
        {            
            worker->process(currSeq->getImagePointer(0), currSeq->getImagePointer(useIth), minA, maxA,fracN,minThreshold,erode,dilute,blackOnWhite,m1,m2,g1,s1,g2,s2,vector<cv::Point>(),maskT);
        }
        DisplayProgress->setValue(workFrame-fromFrame);
        if (DisplayProgress->wasCanceled())
        {
            killTimer(timerID2);
            timerID2=-1;
            DisplayProgress->setValue(toFrame-fromFrame+1);
            delete DisplayProgress;
        }
        workFrame+=FrameStep;
    }
    else
    {
        killTimer(event->timerId());
        DisplayProgress->setValue(toFrame-fromFrame+1);
        timerID2=-1;
        delete DisplayProgress;
        emit readyToRunNext();
    }
    }
    else
    {
     cerr << "Unknown timer ID"<<endl;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
  if(seq.size()>0)
  {
   if (event->key() == Qt::Key_Left) {
       m_player->showImageWithPoints(m_player->getCurrentFrameNo()-1);
   }
   else if (event->key() == Qt::Key_Right) {
       m_player->showImageWithPoints(m_player->getCurrentFrameNo()+1);
   }
   else if (event->key() == Qt::Key_Comma) {
       if(currSeq)
       {
        int currentI=frameSelect->getFrom();
        int currentJ=frameSelect->getTo();
        int diff=currentJ-currentI;
        int newI=currentI-diff;
        if(m_player->useMetaMode())
        {
         if(newI<m_player->metaMin()) newI=m_player->metaMin();        
        }
        else
        {        
         if(newI<0) newI=0;
        }
        int newJ=newI+diff;
        frameSelect->setFrom(newI);
        frameSelect->setTo(newJ);
       }
   }
   else if (event->key() == Qt::Key_Period) {
       if(currSeq)
       {
        int currentI=frameSelect->getFrom();
        int currentJ=frameSelect->getTo();
        int diff=currentJ-currentI;
        int newI=currentI+diff;
        if(m_player->useMetaMode())
        {
         if(newI>=m_player->metaMax()) newI=m_player->metaMax()-1;        
        }
        else
        {
         if(newI>=currSeq->allocatedFrames()) newI=currSeq->allocatedFrames()-1;
        }
        int newJ=newI+diff;
        if(m_player->useMetaMode())
        {
         if(newJ>=m_player->metaMax()) newJ=m_player->metaMax()-1;                
        }
        else
        {
         if(newJ>=currSeq->allocatedFrames()) newJ=currSeq->allocatedFrames()-1;        
        }
        frameSelect->setFrom(newI);
        frameSelect->setTo(newJ);
       }
   }
   else if (event->key() == Qt::Key_K) {
       if(currSeq)
       {
        int currentI=frameSelect->getFrom();
        int currentJ=frameSelect->getTo();
        int diff=currentJ-currentI;
        int newJ=currentI+diff*0.5;
        if(m_player->useMetaMode())
        {
         if(newJ>=m_player->metaMax()) newJ=m_player->metaMax()-1;        
        }
        else
        {
         if(newJ>=currSeq->allocatedFrames()) newJ=currSeq->allocatedFrames()-1;        
        }
        frameSelect->setTo(newJ);
       }
   }
   else if (event->key() == Qt::Key_L) {
       if(currSeq)
       {
        int currentI=frameSelect->getFrom();
        int currentJ=frameSelect->getTo();
        int diff=currentJ-currentI;
        int newJ=currentI+diff*2;
        if(m_player->useMetaMode())
        {
         if(newJ>=m_player->metaMax()) newJ=m_player->metaMax()-1;        
        }
        else
        {
         if(newJ>=currSeq->allocatedFrames()) newJ=currSeq->allocatedFrames()-1;        
        }
        frameSelect->setTo(newJ);
       }
   }
   else if (event->key() == Qt::Key_J) {
       if(currSeq)
       {
        int currentI=frameSelect->getFrom();
        int currentJ=frameSelect->getTo();
        int newJ=currentJ-10;
        if(newJ<=currentI) newJ=currentI+1;
        if(m_player->useMetaMode())
        {
         if(newJ>=m_player->metaMax()) newJ=m_player->metaMax()-1;        
        }
        else
        {
         if(newJ>=currSeq->allocatedFrames()) newJ=currSeq->allocatedFrames()-1;        
        }
        frameSelect->setTo(newJ);
       }
   }
   else if (event->key() == Qt::Key_Semicolon) {
       if(currSeq)
       {
        int currentI=frameSelect->getFrom();
        int currentJ=frameSelect->getTo();
        int newJ=currentJ+10;
        if(newJ<=currentI) newJ=currentI+1;
        if(m_player->useMetaMode())
        {
        if(newJ>=m_player->metaMax()) newJ=m_player->metaMax()-1;
        }
        else
        {
        if(newJ>=currSeq->allocatedFrames()) newJ=currSeq->allocatedFrames()-1;
        }
        frameSelect->setTo(newJ);
       }
   }
   else if (event->key() == Qt::Key_O) {
       if(currSeq)
       {
        int currentI=frameSelect->getFrom();
        int currentJ=frameSelect->getTo();
        int diff=currentJ-currentI;
        int newI=currentJ-diff*0.5;
        if(m_player->useMetaMode())
        {
         if(newI>=m_player->metaMax()) newI=m_player->metaMax()-1;        
        }
        else
        {
        if(newI>=currSeq->allocatedFrames()) newI=currSeq->allocatedFrames()-1;        
        }
        if(m_player->useMetaMode())
        {
        if(newI<m_player->metaMin()) newI=m_player->metaMin();
        }
        else
        {
        if(newI<0) newI=0;
        }
        frameSelect->setFrom(newI);
       }
   }
   else if (event->key() == Qt::Key_I) {
       if(currSeq)
       {
        int currentI=frameSelect->getFrom();
        int currentJ=frameSelect->getTo();
        int diff=currentJ-currentI;
        int newI=currentJ-diff*2;
        if(m_player->useMetaMode())
        {
         if(newI>=m_player->metaMax()) newI=m_player->metaMax()-1;        
        }
        else
        {
        if(newI>=currSeq->allocatedFrames()) newI=currSeq->allocatedFrames()-1;        
        }
        if(m_player->useMetaMode())
        {
        if(newI<m_player->metaMin()) newI=m_player->metaMin();
        }
        else
        {
        if(newI<0) newI=0;
        }
        frameSelect->setFrom(newI);
       }
   }
   else if (event->key() == Qt::Key_U) {
       if(currSeq)
       {
        int currentI=frameSelect->getFrom();
        int currentJ=frameSelect->getTo();
        int newI=currentI-10;
        if(newI>=currentJ) newI=currentJ-1;
        
        if(m_player->useMetaMode())
        {
        if(newI>=m_player->metaMax()) newI=m_player->metaMax()-1;        
        }
        else
        {
        if(newI>=currSeq->allocatedFrames()) newI=currSeq->allocatedFrames()-1;        
        }
        frameSelect->setFrom(newI);
       }
   }
   else if (event->key() == Qt::Key_P) {
       if(currSeq)
       {
        int currentI=frameSelect->getFrom();
        int currentJ=frameSelect->getTo();
        int newI=currentI+10;
        if(newI>=currentJ) newI=currentJ-1;
        if(m_player->useMetaMode())
        {
        if(newI>=m_player->metaMax()) newI=m_player->metaMax()-1;
        }
        else
        {        
        if(newI>=currSeq->allocatedFrames()) newI=currSeq->allocatedFrames()-1;
        }  
        frameSelect->setFrom(newI);
       }
   }   
   else if (event->key() == Qt::Key_Delete) {
       if(currSeq)
       {
        int currentI=frameSelect->getFrom();
        int currentJ=frameSelect->getTo();
        currMosqPos->removePoints(currentI,currentJ,m_player->useMetaMode());
        m_player->showImageWithPoints(m_player->getCurrentFrameNo());
        if(showAllPoints)
         m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
       }
   }   
   else if (event->key() == Qt::Key_Insert) {
       if(currSeq)
       {
        backupPos(seqPointerToSeqNum(currSeq),true);
       }
   }   
  }
}

void MainWindow::on_mouseMove(QPoint m, int c, int r, int dx, int dy, int ddx, int ddy)
{
  if(mouseMode==PAN)
   m_player->moveZoom(m,c,r,dx,dy);
  else if(mouseMode==ZOOM)
   m_player->setSelection(m,c,r,ddx,ddy);
  else if(mouseMode==SELECTROI)
   m_player->setSelection(m,c,r,ddx,ddy);
  if(mouseMode==EDITPOLYGON)
  {
   m_player->drawLine(m,c,r,ddx,ddy);
  }
}

void MainWindow::on_mouseClicked(QPoint m, int c, int r)
{
  if(mouseMode==ADDPOINT)
  {
   backupPos(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
   m_player->addMosquito(m,c,r);
  }
  if(mouseMode==SELECTPOINTFORLOCALSEARCH)
  {
   backupPos(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
   QPointF p=m_player->imageCoordinates(m,c,r);
   double radius=15;
   int x0=p.x()-radius; if(x0<0)x0=0;if(x0>=m_player->getSeqPointer(c,r)->pointerToHeader()->width())x0=m_player->getSeqPointer(c,r)->pointerToHeader()->width()-1;
   int y0=p.y()-radius; if(y0<0)y0=0;if(y0>=m_player->getSeqPointer(c,r)->pointerToHeader()->height())y0=m_player->getSeqPointer(c,r)->pointerToHeader()->height()-1;
   int x1=p.x()+radius; if(x1<0)x1=0;if(x1>=m_player->getSeqPointer(c,r)->pointerToHeader()->width())x1=m_player->getSeqPointer(c,r)->pointerToHeader()->width()-1;
   int y1=p.y()+radius; if(y1<0)y1=0;if(y1>=m_player->getSeqPointer(c,r)->pointerToHeader()->height())y1=m_player->getSeqPointer(c,r)->pointerToHeader()->height()-1;   
   cv::Rect croi=cv::Rect((int)x0,(int)y0, (int)(x1-x0+1),(int) (y1-y0+1));
   cerr << "Searching ROI:"<<croi<< "for frame "<<m_player->getSeqPointer(c,r)->currentFrame() <<"\n";
   double maxdiff;
   vector<vector<double> > centers=segmentROI(m_player->getSeqPointer(c,r),croi, m_player->getSeqPointer(c,r)->currentFrame(),maxdiff);
   if(maxdiff>0)
   {
    MosquitoesInFrame* currentFrame=m_player->getMosqPointer(c,r)->mosqInFrameI(m_player->getSeqPointer(c,r)->currentFrame());
    if(currentFrame)
    {
    cerr<<"Adding "<<centers.size()<<" Mosquitoes...\n";
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
    }
    else
    {
     cerr<<"CurrentFrame not valid...\n";
    }
   } 
  }
  if(mouseMode==SELECTTRACKFORLOCALSEARCH)
  {
   backupPos(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
   QPointF p=m_player->imageCoordinates(m,c,r);
   vector<int> currT=m_player->getTrackPointer(c,r)->findTrackID((double) p.x(), (double) p.y(), 10.0, frameSelect->getFrom(),frameSelect->getTo());
   if(currT.size()==2)
   {
    double ox;
    double oy;
    unsigned long of;
    bool ret=m_player->getTrackPointer(c,r)->estimateNextPosition(currT[0],ox,oy,of);
    if(ret)
    {
     double radius=15;
     int x0=ox-radius; if(x0<0)x0=0;if(x0>=m_player->getSeqPointer(c,r)->pointerToHeader()->width())x0=m_player->getSeqPointer(c,r)->pointerToHeader()->width()-1;
     int y0=oy-radius; if(y0<0)y0=0;if(y0>=m_player->getSeqPointer(c,r)->pointerToHeader()->height())y0=m_player->getSeqPointer(c,r)->pointerToHeader()->height()-1;
     int x1=ox+radius; if(x1<0)x1=0;if(x1>=m_player->getSeqPointer(c,r)->pointerToHeader()->width())x1=m_player->getSeqPointer(c,r)->pointerToHeader()->width()-1;
     int y1=oy+radius; if(y1<0)y1=0;if(y1>=m_player->getSeqPointer(c,r)->pointerToHeader()->height())y1=m_player->getSeqPointer(c,r)->pointerToHeader()->height()-1;   
     cv::Rect croi=cv::Rect((int)x0,(int)y0, (int)(x1-x0+1),(int) (y1-y0+1));
     cerr << "Searching ROI:"<<croi<< "for frame "<<of <<"\n";
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
         m_player->getTrackPointer(c,r)->addPointToTrack(currT[0], NewTrackPoint);
         m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
         updatePointerOfTracks(c,r);
         updateTableOfTracks();
         on_mouseClicked(m,c,r);
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
  if(mouseMode==SELECTTRACKFORLOCALBACKWARDSSEARCH)
  {
   backupPos(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
   QPointF p=m_player->imageCoordinates(m,c,r);
   vector<int> currT=m_player->getTrackPointer(c,r)->findTrackID((double) p.x(), (double) p.y(), 10.0, frameSelect->getFrom(),frameSelect->getTo());
   if(currT.size()==2)
   {
    double ox;
    double oy;
    unsigned long of;
    bool ret=m_player->getTrackPointer(c,r)->estimatePrevPosition(currT[0],ox,oy,of);
    if(ret)
    {
     double radius=15;
     int x0=ox-radius; if(x0<0)x0=0;if(x0>=m_player->getSeqPointer(c,r)->pointerToHeader()->width())x0=m_player->getSeqPointer(c,r)->pointerToHeader()->width()-1;
     int y0=oy-radius; if(y0<0)y0=0;if(y0>=m_player->getSeqPointer(c,r)->pointerToHeader()->height())y0=m_player->getSeqPointer(c,r)->pointerToHeader()->height()-1;
     int x1=ox+radius; if(x1<0)x1=0;if(x1>=m_player->getSeqPointer(c,r)->pointerToHeader()->width())x1=m_player->getSeqPointer(c,r)->pointerToHeader()->width()-1;
     int y1=oy+radius; if(y1<0)y1=0;if(y1>=m_player->getSeqPointer(c,r)->pointerToHeader()->height())y1=m_player->getSeqPointer(c,r)->pointerToHeader()->height()-1;   
     cv::Rect croi=cv::Rect((int)x0,(int)y0, (int)(x1-x0+1),(int) (y1-y0+1));
     cerr << "Searching ROI:"<<croi<< "for frame "<<of <<"\n";
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
         m_player->getTrackPointer(c,r)->addPointToTrack(currT[0], NewTrackPoint);
         m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
         updatePointerOfTracks(c,r);
         updateTableOfTracks();
         on_mouseClicked(m,c,r);
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
  if(mouseMode==SELECTTRACKFORLOCALINBETWEENSEARCH)
  {
   backupPos(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
   QPointF p=m_player->imageCoordinates(m,c,r);
   vector<int> currT=m_player->getTrackPointer(c,r)->findTrackID((double) p.x(), (double) p.y(), 10.0, frameSelect->getFrom(),frameSelect->getTo());
   if(currT.size()==2)
   {
    double ox;
    double oy;
    unsigned long of;
    vector<vector<TrackPoint> > pieces=m_player->getTrackPointer(c,r)->getMissingPieces(currT[0]);
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
           m_player->getTrackPointer(c,r)->addPointToTrack(currT[0], NewTrackPoint);
           m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
           updatePointerOfTracks(c,r);
           updateTableOfTracks();
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
  if(mouseMode==REMOVEPOINT)
  {
   backupPos(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
   if(showAllPoints)
    m_player->removeMosquito(m,c,r, fromFrame, toFrame,displayParam->getSearchRadius(),displayParam->getOnlyOne());
   else
    m_player->removeMosquito(m,c,r,displayParam->getSearchRadius(),displayParam->getOnlyOne());
  }
  if(mouseMode==PAN)
   m_player->setCursorForWidgets(3);
  if(mouseMode==EDITPOLYGON)
  {
   if(thingToAccept==1)
   {
   if(polygon.size()>0)
   {
    selectedVert=-1;
    double dist=16;
    QPointF p=m_player->imageCoordinates(m,c,r);
    for(unsigned int i=0;i <polygon.size(); i++)
    {
     double d=sqrt(pow(polygon[i].x()-p.x(),2)+pow(polygon[i].y()-p.y(),2));
     if(d<dist)
     {
      dist=d;
      selectedVert=i;
     }
    }
    if(selectedVert==-1)
     cerr<<"No vertex found!\n";
   }
   m_player->setCursorForWidgets(3);
   }
   else if(thingToAccept==2 || thingToAccept==3)
   {
   if(polyROI.size()>0)
   {
    selectedVert=-1;
    double dist=16;
    QPointF p=m_player->imageCoordinates(m,c,r);
    for(unsigned int i=0;i <polyROI.size(); i++)
    {
     double d=sqrt(pow(polyROI[i].x()-p.x(),2)+pow(polyROI[i].y()-p.y(),2));
     if(d<dist)
     {
      dist=d;
      selectedVert=i;
     }
    }
    if(selectedVert==-1)
     cerr<<"No vertex found!\n";
   }
   m_player->setCursorForWidgets(3);
   }
  }
  if(mouseMode==POLYGONSELECTION)
  {
   QPointF p=m_player->imageCoordinates(m,c,r);
   if(polygon.size()>1){
    if(sqrt(pow(polygon[0].x()-p.x(),2)+pow(polygon[0].y()-p.y(),2))<50 ||
       sqrt(pow(polygon[polygon.size()-1].x()-p.x(),2)+pow(polygon[polygon.size()-1].y()-p.y(),2))<10)
    {
     p=polygon[0];
     polygon.push_back(p);
     MosquitoesInSeq* pin=m_player->getMosqPointer(c,r);
     backupPos(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
     int rem=pin->removePointsInsideOfPolygon(polygon,frameSelect->getFrom(),frameSelect->getTo(),false,m_player->useMetaMode());
     polygon.clear();
     m_player->clearPolygon(c,r);
     m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
    }
    else
    {
     polygon.push_back(p);
     m_player->addToPolygon(m,c,r);
    }
   }
   else
   {
   polygon.push_back(p);
   m_player->addToPolygon(m,c,r);
   }
  }
  if(mouseMode==INVERSEPOLYGONSELECTION)
  {
   QPointF p=m_player->imageCoordinates(m,c,r);
   if(polygon.size()>1){
    if(sqrt(pow(polygon[0].x()-p.x(),2)+pow(polygon[0].y()-p.y(),2))<50 ||
       sqrt(pow(polygon[polygon.size()-1].x()-p.x(),2)+pow(polygon[polygon.size()-1].y()-p.y(),2))<10)
    {
     p=polygon[0];
     polygon.push_back(p);
     MosquitoesInSeq* pin=m_player->getMosqPointer(c,r);
     backupPos(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
     int rem=pin->removePointsInsideOfPolygon(polygon,frameSelect->getFrom(),frameSelect->getTo(),true,m_player->useMetaMode());
     polygon.clear();
     m_player->clearPolygon(c,r);
     m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
    }
    else
    {
     polygon.push_back(p);
     m_player->addToPolygon(m,c,r);
    }
   }
   else
   {
   polygon.push_back(p);
   m_player->addToPolygon(m,c,r);
   }
  }
  if(mouseMode==SELECTMOSQUITO)
  {
     QPointF p=m_player->imageCoordinates(m,c,r);
     MosquitoesInSeq* pin=m_player->getMosqPointer(c,r);
     vector<unsigned long> tmp=pin->findMosquito(p,frameSelect->getFrom(),frameSelect->getTo(),m_player->useMetaMode());
     if(tmp.size()==2)
     {
         m_player->setCurrentFrameNo(tmp[0]);
         m_player->ForceUpdateFrame(tmp[0]);
     }
  }
  if(mouseMode==SELECTPOLYROI)
  {
   QPointF p=m_player->imageCoordinates(m,c,r);
   polyROI.push_back(p);
   m_player->addToPolygon(m,c,r);

   if(polyROI.size()>1 && sqrt(pow(p.x()-polyROI[0].x(),2) +pow(p.y()-polyROI[0].y(),2))<10)
   {   
    mouseMode=EDITPOLYGON;
    accept->setIcon(QPixmap(":/icons/ok.png"));
    thingToAccept=2;
    currentRow=r;
    currentCol=c;
   }
  }
  if(mouseMode==REMOVETRACK)
  {
     QPointF p=m_player->imageCoordinates(m,c,r);
     vector<int> currT=m_player->getTrackPointer(c,r)->findTrackID((double) p.x(), (double) p.y(), 10.0, frameSelect->getFrom(),frameSelect->getTo());
     if(currT.size()==2)
     {
      backupTracks(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
      m_player->getTrackPointer(c,r)->deleteTrack(currT[0]);
      m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
      updatePointerOfTracks(c,r);
      updateTableOfTracks();
     }
  }

  if(mouseMode==SELECTTRACKTOESTIMATE)
  {
     QPointF p=m_player->imageCoordinates(m,c,r);
     vector<int> currT=m_player->getTrackPointer(c,r)->findTrackID((double) p.x(), (double) p.y(), 10.0, frameSelect->getFrom(),frameSelect->getTo());
     if(currT.size()==2)
     {
      double ox;
      double oy;
      unsigned long of;
      bool ret=m_player->getTrackPointer(c,r)->estimateNextPosition(currT[0],ox,oy,of,2.0);
      if(ret)
      {
       m_player->setHighlight(ox, oy, c, r);
       cerr<<"Got Estimate for frame "<<of<< "...\n";      
      }
      else
       cerr<<"Didn't got Estimate...\n";
     }
  }

  if(mouseMode==SELECTTRACKFROMESTIMATE)
  {
     QPointF p=m_player->imageCoordinates(m,c,r);
     vector<int> currT=m_player->getTrackPointer(c,r)->findTrackID((double) p.x(), (double) p.y(), 10.0, frameSelect->getFrom(),frameSelect->getTo());
     if(currT.size()==2)
     {
      double ox;
      double oy;
      unsigned long of;
      bool ret=m_player->getTrackPointer(c,r)->estimatePrevPosition(currT[0],ox,oy,of,2.0);
      if(ret)
      {
       m_player->setHighlight(ox, oy, c, r);
       cerr<<"Got Estimate for frame "<<of<< "...\n";      
      }
      else
       cerr<<"Didn't got Estimate...\n";
     }
  }

  if(mouseMode==REMOVEPOINTFROMTRACK)
  {
     QPointF p=m_player->imageCoordinates(m,c,r);
     vector<int> currT=m_player->getTrackPointer(c,r)->findTrackID((double) p.x(), (double) p.y(), 10.0, frameSelect->getFrom(),frameSelect->getTo());
     if(currT.size()==2)
     {
      backupTracks(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
      m_player->getTrackPointer(c,r)->removePointFromTrack(currT[0], currT[1]);
      m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
      updatePointerOfTracks(c,r);
      updateTableOfTracks();
     }
  }

  if(mouseMode==ADDPOINTTOTRACK)
  {
     if(selectedTrack==-1)
     {
     QPointF p=m_player->imageCoordinates(m,c,r);
     vector<int> currT=m_player->getTrackPointer(c,r)->findTrackID((double) p.x(), (double) p.y(), 10.0, frameSelect->getFrom(),frameSelect->getTo());
     if(currT.size()==2)
     {
      selectedTrack=currT[0];
      m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
      m_player->setTrackToHighlight(m_player->getTrackPointer(c,r)->getSingleTrack(currT[0]),c,r);
     }
     }
     else
     {
      QPointF p=m_player->imageCoordinates(m,c,r);
      MosquitoesInSeq* pin=m_player->getMosqPointer(c,r);
      vector<unsigned long> tmp=pin->findMosquito(p,frameSelect->getFrom(),frameSelect->getTo());
      if(tmp.size()==2)
      {
      backupTracks(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
         cerr<<"This is Mosquito "<<tmp[1]<<" in frame "<<tmp[0]<<"\n";
         MosquitoesInFrame* cF=pin->mosqInFrameI(tmp[0]);
         MosquitoPosition* cM=cF->mosqi(tmp[1]);
         TrackPoint NewTrackPoint(cM->getX(),cM->getY(),tmp[0],cF->getTsSec(),cF->getTsMSec(),cF->getTsUSec(),
                          cM->getI(),cM->getA(),cF->getMaxDiff(),cF->getMinArea(),cF->getMaxArea(),
                          cF->getThreshold(),cF->getMinThreshold(),cF->getIth());
         m_player->getTrackPointer(c,r)->addPointToTrack(selectedTrack, NewTrackPoint);
         m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
         m_player->setTrackToHighlight(m_player->getTrackPointer(c,r)->getSingleTrack(selectedTrack),c,r);
         updatePointerOfTracks(c,r);
         updateTableOfTracks();
      }       
     }
  }

  if(mouseMode==JOINTRACKS)
  {
     if(selectedTrack==-1)
     {
     QPointF p=m_player->imageCoordinates(m,c,r);
     vector<int> currT=m_player->getTrackPointer(c,r)->findTrackID((double) p.x(), (double) p.y(), 10.0, frameSelect->getFrom(),frameSelect->getTo());
     if(currT.size()==2)
     {
      selectedTrack=currT[0];
      m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
      m_player->setTrackToHighlight(m_player->getTrackPointer(c,r)->getSingleTrack(currT[0]),c,r);
     }
     }
     else
     {
      QPointF p=m_player->imageCoordinates(m,c,r);
      vector<int> currT=m_player->getTrackPointer(c,r)->findTrackID((double) p.x(), (double) p.y(), 10.0, frameSelect->getFrom(),frameSelect->getTo());
      if(currT.size()==2)
      {
         if(currT[0]!=selectedTrack)
         {
      backupTracks(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
             bool ret=m_player->getTrackPointer(c,r)->joinTracks(selectedTrack, currT[0]);
             if(currT[0]<selectedTrack && ret)
               selectedTrack--;
             m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
             m_player->setTrackToHighlight(m_player->getTrackPointer(c,r)->getSingleTrack(selectedTrack),c,r);
         }
      }       
      selectedTrack=-1;
      m_player->clearTrackToHighlight(c,r); 
      if(showAllPoints)
       m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
      updatePointerOfTracks(c,r);
      updateTableOfTracks();
     }
  }
  if(mouseMode==SPLITTRACK)
  {
     QPointF p=m_player->imageCoordinates(m,c,r);
     vector<int> currT=m_player->getTrackPointer(c,r)->findTrackID((double) p.x(), (double) p.y(), 10.0, frameSelect->getFrom(),frameSelect->getTo());
     if(currT.size()==2)
     {
      backupTracks(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
      m_player->getTrackPointer(c,r)->splitTrack(currT[0], currT[1]);
      m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
      updatePointerOfTracks(c,r);
      updateTableOfTracks();
     }
  }
  if(mouseMode==POLYGONSELECTIONFORTRACKS)
  {
   QPointF p=m_player->imageCoordinates(m,c,r);
   if(polygon.size()>1){
    if(sqrt(pow(polygon[0].x()-p.x(),2)+pow(polygon[0].y()-p.y(),2))<50 ||
       sqrt(pow(polygon[polygon.size()-1].x()-p.x(),2)+pow(polygon[polygon.size()-1].y()-p.y(),2))<10)
    {
     p=polygon[0];
     polygon.push_back(p);
     MosquitoesInSeq* pin=m_player->getMosqPointer(c,r);
     vector<Point2f> points=pin->pointsInsideOfPolygon(polygon,frameSelect->getFrom(),frameSelect->getTo());
     cerr<<"Removing "<<points.size()<<" TrackPoints\n";
     cerr<<"points="<<points<<"\n";
     backupTracks(seqPointerToSeqNum(m_player->getSeqPointer(c,r)));
     for(unsigned int j=0; j<points.size();j++)
     {
      vector<int> currT=m_player->getTrackPointer(c,r)->findTrackID((double) points[j].x, (double) points[j].y, 10.0, frameSelect->getFrom(),frameSelect->getTo());
      if(currT.size()==2)
      {
       cerr<<"Removing Point "<<currT[1]<<" form Track "<<currT[0]<<"\n";
       m_player->getTrackPointer(c,r)->removePointFromTrack(currT[0], currT[1]);
       updatePointerOfTracks(c,r);
       updateTableOfTracks();      
      }
      else
      {
       cerr<<"No track found for Point "<<j<<"("<<points[j]<<")\n";
      }
     }
     polygon.clear();
     m_player->clearPolygon(c,r);
     m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
    }
    else
    {
     polygon.push_back(p);
     m_player->addToPolygon(m,c,r);
    }
   }
   else
   {
   polygon.push_back(p);
   m_player->addToPolygon(m,c,r);
   }
  }
  if(mouseMode==DISPLAYPIXELVALUE)
  {
   if(seq.size()>0)
   {
   QPointF p=m_player->imageCoordinates(m,c,r);
   int value=m_player->getSeqPointer(c,r)->getImagePointer(0)->getValue((unsigned int)(p.x()+0.5), (unsigned int) (p.y()+0.5));
   cerr<<"("<<p.x()<<","<<p.y()<<") -> "<<value<<"\n";
   QMessageBox msgBox;
   msgBox.setText(QString("I(%1,%2) = %3 \n").arg(int(p.x()+0.5)).arg(int(p.y()+0.5)).arg(value));
   msgBox.exec();

   }
  } 
  if(mouseMode==POLYGONSELECTIONTOTRACK)
  {
   QPointF p=m_player->imageCoordinates(m,c,r);
   if(polygon.size()>1){
    if(sqrt(pow(polygon[0].x()-p.x(),2)+pow(polygon[0].y()-p.y(),2))<50 ||
       sqrt(pow(polygon[polygon.size()-1].x()-p.x(),2)+pow(polygon[polygon.size()-1].y()-p.y(),2))<10)
    {
     p=polygon[0];
     polygon.push_back(p);
     if(m_player->getMosqPointer(c,r) && m_player->getTrackPointer(c,r))
     {
       vector<vector<double> > a=m_player->getMosqPointer(c,r)->pointsInsideOfPolygonAsVector(polygon,frameSelect->getFrom(),frameSelect->getTo(),m_player->useMetaMode());
       if(a.size()>0)
       {
         int currID=m_player->getTrackPointer(c,r)->registerNewMosquito();
         vector<int> tracksToJoin;
         for(int ii=0; ii<a.size(); ii++)
         {
           MosquitoesInFrame* mif=m_player->getMosqPointer(c,r)->mosqInFrameI((int)a[ii][2]);
           if(mif)
           {
            MosquitoPosition* mp=mif->mosqi(a[ii][0],a[ii][1], 1.0);
            if(mp)
            {
              vector<int> alreadyOnTrack=m_player->getTrackPointer(c,r)->findTrackID(a[ii][0],a[ii][1], 1, (int)a[ii][2], (int) a[ii][2]+1);
              if(alreadyOnTrack.size()==2)
              {
               tracksToJoin.push_back(alreadyOnTrack[0]);
              }
              else
              {
               TrackPoint NewTrackPoint(a[ii][0],a[ii][1],a[ii][2],a[ii][3],a[ii][4],a[ii][5],a[ii][6],a[ii][7],a[ii][8],a[ii][9],a[ii][10],a[ii][11],a[ii][12],a[ii][13]);
               m_player->getTrackPointer(c,r)->updateTrack(currID,NewTrackPoint);
              }
            }
           }
         }
         if(tracksToJoin.size()>0)
         {
          tracksToJoin.push_back(currID);
          sort( tracksToJoin.begin(), tracksToJoin.end() );
          tracksToJoin.erase( unique( tracksToJoin.begin(), tracksToJoin.end() ), tracksToJoin.end() );
          if(tracksToJoin.size()>1)
          {
           cerr<<"Joining Tracks";
           for(int i=0; i<tracksToJoin.size(); i++) cerr<<" "<<tracksToJoin[i];
           cerr<<"\n";
           for(int i=tracksToJoin.size()-1; i>0; i--)
           {
            bool ret=currReg->joinTracks(tracksToJoin[0],tracksToJoin[i]);
            if(!ret) cerr<<"Error: Could not join Tracks...\n";
           }
          }
         }
       }
     }
     polygon.clear();
     m_player->clearPolygon(c,r);
     m_player->drawAllPoints(c, r, frameSelect->getFrom(),frameSelect->getTo());
    }
    else
    {
     polygon.push_back(p);
     m_player->addToPolygon(m,c,r);
    }
   }
   else
   {
   polygon.push_back(p);
   m_player->addToPolygon(m,c,r);
   }
  }
}

void MainWindow::on_mouseRelease(QPoint m, int c, int r,int dx, int dy)
{
  if(mouseMode==ZOOM)
   m_player->setZoom(m,c,r,dx,dy);
  if(mouseMode==SELECTROI)
   m_player->setROI(m,c,r,dx,dy);
  if(mouseMode==PAN)
   m_player->setCursorForWidgets(2);
  if(mouseMode==EDITPOLYGON)
  {
   QPointF p=m_player->imageCoordinates(m,c,r);
   if(thingToAccept==1 || thingToAccept==4 )
   {
   if(selectedVert!=-1 && selectedVert<polygon.size())
   {
    polygon[selectedVert]=p;
    m_player->clearPolygon(c,r);
    for(int i=0; i<polygon.size(); i++)
     m_player->addToPolygon(polygon[i].x(),polygon[i].y(),c,r);
    m_player->addToPolygon(polygon[0].x(),polygon[0].y(),c,r);     
   }
   m_player->setCursorForWidgets(2);
   }
   else if(thingToAccept==2 || thingToAccept==3)
   {
   if(selectedVert!=-1 && selectedVert<polyROI.size())
   {
    polyROI[selectedVert]=p;
    m_player->clearPolygon(c,r);
    for(int i=0; i<polyROI.size(); i++)
     m_player->addToPolygon(polyROI[i].x(),polyROI[i].y(),c,r);
    m_player->addToPolygon(polyROI[0].x(),polyROI[0].y(),c,r);     
   }
   m_player->setCursorForWidgets(2);
   }
  }
}

void MainWindow::on_m_player_keyPressEvent(QKeyEvent* key)
{
 keyPressEvent(key);
}

void MainWindow::setNative(bool b)
{
     nativeFD=b;
}

void MainWindow::createToolBar()
{
 QToolBar *viewToolBar = addToolBar(tr("View"));
 viewToolBar->setObjectName("ViewToolBar");
 viewToolBar->setIconSize(QSize(24, 24)); 
 viewToolBar->addAction(QPixmap(":/icons/zoom.png"), tr("ZOOM"), this,[this](){mouseMode=ZOOM;m_player->setCursorForWidgets(5);});
 viewToolBar-> addAction(QPixmap(":/icons/pan.png"), tr("Pan"), this,[this](){mouseMode=PAN;m_player->setCursorForWidgets(2);});
 viewToolBar-> addAction(QPixmap(":/icons/None.png"),tr("None"), this,[this](){mouseMode=NONE;m_player->setCursorForWidgets(0);});
 viewToolBar-> addAction(QPixmap(":/icons/Reset.png"), tr("Reset"), this,[this](){
 for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j))m_player->setZoom(QPoint(0,0),i,j,0,0);});
 viewToolBar-> addAction(QPixmap(":/icons/toggle_points.png"), tr("Points"), this,[this](){
  if(showAllPoints==false)
  {
  if(seq.size()>0)m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
  showAllPoints=true;
  }
  else
  {
    if(seq.size()>0) {m_player->clearAllPoints(-1, -1);m_player->ForceUpdateFrame(currSeq->currentFrame());}
    showAllPoints=false;
  }
 });

 QToolBar *pointsToolBar = addToolBar(tr("Points"));
 pointsToolBar->setObjectName("PointsToolBar");
 pointsToolBar->setIconSize(QSize(24, 24)); 
 pointsToolBar-> addAction(QPixmap(":/icons/addPoint.png"), tr("Add"), this,[this](){mouseMode=ADDPOINT;m_player->setCursorForWidgets(1);});
 pointsToolBar-> addAction(QPixmap(":/icons/removePoint.png"),tr("Remove"), this,[this](){mouseMode=REMOVEPOINT;m_player->setCursorForWidgets(1);});
 pointsToolBar-> addAction(QPixmap(":/icons/polyRemovePoint.png"),tr("PolygonSelection"), this,[this](){
      if(seq.size()>0)
      {
      mouseMode=POLYGONSELECTION;
      m_player->setCursorForWidgets(4);
      for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j))m_player->drawAllPoints(i, j, frameSelect->getFrom(),frameSelect->getTo());
      showAllPoints=true;
      polygon.clear();
      for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j))m_player->clearPolygon(i,j);
      }
  });
  pointsToolBar-> addAction(QPixmap(":/icons/polyRemoveInvPoint.png"),tr("PolygonSelection(inv)"), this,[this](){
      if(seq.size()>0)
      {
      mouseMode=INVERSEPOLYGONSELECTION;
      m_player->setCursorForWidgets(4);
      for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j))m_player->drawAllPoints(i, j, frameSelect->getFrom(),frameSelect->getTo());
      showAllPoints=true;
      polygon.clear();
      for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j))m_player->clearPolygon(i,j);
      }
  });
  pointsToolBar-> addAction(QPixmap(":/icons/selectPoint.png"),tr("Select Mosquito"), this,[this](){
      if(seq.size()>0)
      {
      mouseMode=SELECTMOSQUITO;
      m_player->setCursorForWidgets(1);
      for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j)) m_player->drawAllPoints(i, j, frameSelect->getFrom(),frameSelect->getTo());
      }
  });
  pointsToolBar-> addAction(QPixmap(":/icons/undo.png"),tr("Undo"), this,[this](){
      if(seq.size()>0)
      {
         int sqn=seqPointerToSeqNum(currSeq);
         if(sqn!=-1)
         {
          restorePos(sqn);
          m_player->showImageWithPoints(m_player->getCurrentFrameNo());
          if(showAllPoints)
           m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
         }
      }
  });

 QToolBar *tracksToolBar = addToolBar(tr("Tracks"));
 tracksToolBar->setObjectName("TracksToolBar");
 tracksToolBar->setIconSize(QSize(24, 24)); 
  tracksToolBar-> addAction(QPixmap(":/icons/RemovePointFromTrack.png"),tr("Remove Point From Track"), this,[this](){if(seq.size()>0){
   for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j))
    m_player->drawAllPoints(i, j, frameSelect->getFrom(),frameSelect->getTo());
    showAllPoints=true;}
   mouseMode=REMOVEPOINTFROMTRACK;m_player->setCursorForWidgets(1);});
  tracksToolBar-> addAction(QPixmap(":/icons/AddPointToTrack.png"),tr("Add Point to track"), this,[this](){if(seq.size()>0){
   for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j))
    m_player->drawAllPoints(i, j, frameSelect->getFrom(),frameSelect->getTo(),true);
   showAllPoints=true;
   }
   mouseMode=ADDPOINTTOTRACK;m_player->setCursorForWidgets(1);selectedTrack=-1;});
  tracksToolBar-> addAction(QPixmap(":/icons/removeTrack.png"),tr("Remove Track"), this,[this](){if(seq.size()>0){
  for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j))m_player->drawAllPoints(0, 0, frameSelect->getFrom(),frameSelect->getTo(),true);showAllPoints=true;}
  mouseMode=REMOVETRACK;m_player->setCursorForWidgets(1);});
  tracksToolBar-> addAction(QPixmap(":/icons/joinTracks.png"),tr("Join tracks"), this,[this](){if(seq.size()>0){
  for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j))m_player->drawAllPoints(0, 0, frameSelect->getFrom(),frameSelect->getTo(),true);showAllPoints=true;}mouseMode=JOINTRACKS;m_player->setCursorForWidgets(1);selectedTrack=-1;});
  tracksToolBar-> addAction(QPixmap(":/icons/splitTracks.png"),tr("Split tracks"), this,[this](){if(seq.size()>0){
  for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j))m_player->drawAllPoints(0, 0, frameSelect->getFrom(),frameSelect->getTo());showAllPoints=true;}mouseMode=SPLITTRACK;m_player->setCursorForWidgets(1);selectedTrack=-1;});
  tracksToolBar-> addAction(QPixmap(":/icons/PolyRemoveTrack.png"),tr("Remove Point From Track(poly)"), this,[this](){if(seq.size()>0){
  for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j))m_player->drawAllPoints(0, 0, frameSelect->getFrom(),frameSelect->getTo());showAllPoints=true;}mouseMode=POLYGONSELECTIONFORTRACKS;m_player->setCursorForWidgets(1);});
  tracksToolBar-> addAction(QPixmap(":/icons/polyCreateTrackOfPoint.png"),tr("Create Track From Points(poly)"), this,[this](){if(seq.size()>0){
  for(int i=0; i<4; i++)for(int j=0; j<4;j++)if(m_player->getSeqPointer(i,j))m_player->drawAllPoints(0, 0, frameSelect->getFrom(),frameSelect->getTo());showAllPoints=true;}mouseMode=POLYGONSELECTIONTOTRACK;m_player->setCursorForWidgets(1);});
  tracksToolBar->addAction(QPixmap(":/icons/undo.png"),tr("Undo"), this,[this](){
     if(seq.size()>0)
     {
         int sqn=seqPointerToSeqNum(currSeq);
         if(sqn!=-1)
         {
          restoreTracks(sqn);
          m_player->showImageWithPoints(m_player->getCurrentFrameNo());
          if(showAllPoints)
           m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
         }
      }
     });

 QToolBar *okToolBar = addToolBar(tr("Accept"));
 okToolBar->setObjectName("OkToolBar");
 okToolBar->setIconSize(QSize(24, 24)); 
 accept=okToolBar-> addAction(QPixmap(":/icons/ok.png"),tr("Accept"), this,[this](){
  if(thingToAccept==2)
  {
   accept->setIcon(QPixmap(":/icons/ok_grayed.png"));
   thingToAccept=0;
   currentRow=-1;
   currentCol=-1;
   mouseMode=ZOOM;
   vector<cv::Point> pR;
   for(unsigned int i=0; i<polyROI.size(); i++)
    pR.push_back(cv::Point(int(polyROI[i].x()+0.5),int(polyROI[i].y()+0.5)));
   currSeq->setPolyRoi(pR);   
  }
  });
 accept->setIcon(QPixmap(":/icons/ok_grayed.png"));
}
