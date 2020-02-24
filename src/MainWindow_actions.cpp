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
extern "C" {
#include <libavcodec/avcodec.h>
}
#include <sstream>
#include <iomanip>

using namespace std;
using namespace cv;

/** \brief storage of Connection costs
  */
struct costOfConnection {
    double cost;
    int source;
    int target;
};

bool sortCostOfConnectionByCost(const costOfConnection &a, const costOfConnection &b)
{
    return a.cost < b.cost;
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(tr("Sequence files (*.seq *.mp4 *.png *.jpg)"));
    QStringList fileNames;
    if (dialog.exec())
    	fileNames = dialog.selectedFiles();
    for (int i = 0; i < fileNames.size(); ++i)
       loadFile(fileNames.at(i));
}

void MainWindow::on_about_triggered()
{
    //QString compilationHost = QString("%1").arg(BUILDHOST);
    //QString compilationUser = QString("%1").arg(BUILDUSER);
    //QString build = QString("%1%2").arg(QLocale(QLocale::C).toDate(QString(__DATE__).simplified(), QLatin1String("MMM d yyyy")).toString("yyyyMMdd"))
    //                .arg(QString("%1%2%3%4%5%6").arg(__TIME__[0]).arg(__TIME__[1]).arg(__TIME__[3]).arg(__TIME__[4]).arg(__TIME__[6]).arg(__TIME__[7]));
    QString build(GITVERSION);
    QString builddate = QString("%1 %2").arg(QLocale(QLocale::C).toDate(QString(__DATE__).simplified(), QLatin1String("MMM d yyyy")).toString("dd.MM.yyyy"))
                    .arg(QString("%1%2:%3%4:%5%6").arg(__TIME__[0]).arg(__TIME__[1]).arg(__TIME__[3]).arg(__TIME__[4]).arg(__TIME__[6]).arg(__TIME__[7]));
    QMessageBox::about(this,tr("SeqFileProcessing2D"), QString("<b>SeqFileProcessing2D</b>:<br><a href='https://github.com/kroener/SeqFileProcessing2D'>https://github.com/kroener/SeqFileProcessing2D</a><br>") +QString("<b>Version</b>:<br>") +build+QString("<br><b>Build time</b>:<br>") +builddate+QString("<br>"));
}

void MainWindow::on_actionClose_triggered()
{
    if(seq.size()>1)
    {
        int toRemove=currSeqNum;
        if(currSeqNum == int(seq.size()-1))
        {
         switchSeq(currSeqNum-1);
        }
        else
        {
         switchSeq(currSeqNum+1);
        }
        seq.erase (seq.begin()+toRemove);
        MosqPos.erase (MosqPos.begin()+toRemove);
        Reg.erase (Reg.begin()+toRemove);
        RegHist.erase (RegHist.begin()+toRemove);
        MosqPosHist.erase (MosqPosHist.begin()+toRemove);
        settings.erase (settings.begin()+toRemove);
        seqFileChanged=true;
        seqInfo->removeFromList(toRemove);
    }
}

void MainWindow::on_actionOpenPositions_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(tr("Positions files (*.txt *.dat)"));
    QFileInfo fN(QString::fromStdString(currSeq->filename()));
    dialog.setDirectory(fN.absolutePath());
    QStringList fileNames;
    if (dialog.exec())
    	fileNames = dialog.selectedFiles();
    for (int i = 0; i < fileNames.size(); ++i)
     loadPositionsFile(fileNames.at(i));
}

void MainWindow::on_actionOpenPositionsRange_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(tr("Positions files (*.txt *.dat)"));
    QFileInfo fN(QString::fromStdString(currSeq->filename()));
    dialog.setDirectory(fN.absolutePath());
    QStringList fileNames;
    if (dialog.exec())
    	fileNames = dialog.selectedFiles();
    for (int i = 0; i < fileNames.size(); ++i)
     loadPositionsFileRange(fileNames.at(i));
}

void MainWindow::on_actionOpenDynamicPositions_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Positions files (*.txt *.dat)"));
    QFileInfo fN(QString::fromStdString(currSeq->filename()));
    dialog.setDirectory(fN.absolutePath());
    while (dialog.exec() == QDialog::Accepted && !loadPositionsFile(dialog.selectedFiles().first(),true)) {}
}

void MainWindow::on_actionOpenProject_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Project files (*.xml *.yml *.yml.gz *.xml.gz)"));
    if(seq.size()>0)
    {
     QFileInfo fN(QString::fromStdString(currSeq->filename()));
     dialog.setDirectory(fN.absolutePath());
    }
    while (dialog.exec() == QDialog::Accepted && !loadProject(dialog.selectedFiles().first())) {}
}

void MainWindow::on_actionOpenTracks_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Tracks files (*.txt *.dat)"));
    QFileInfo fN(QString::fromStdString(currSeq->filename()));
    dialog.setDirectory(fN.absolutePath());
    while (dialog.exec() == QDialog::Accepted && !loadTracksFile(dialog.selectedFiles().first())) {}
}

void MainWindow::on_actionSavePositions_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Positions files (*.txt *.dat)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QFileInfo fN(QString::fromStdString(currSeq->filename()));
    dialog.setDirectory(fN.absolutePath());
    dialog.selectFile(QString(fN.baseName()+"_pos.txt"));
    while (dialog.exec() == QDialog::Accepted && !savePositionsFile(dialog.selectedFiles().first())) {}
}

void MainWindow::on_actionSavePositionsSelection_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Positions files (*.txt *.dat)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QFileInfo fN(QString::fromStdString(currSeq->filename()));
    dialog.setDirectory(fN.absolutePath());
    dialog.selectFile(QString(fN.baseName()+"_pos.txt"));
    while (dialog.exec() == QDialog::Accepted && !savePositionsFile(dialog.selectedFiles().first(),true)) {}
}

void MainWindow::on_actionSaveProject_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Project files (*.yml *.xml *.yml.gz *.xml.gz)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if(seq.size()>0)
    {
     QFileInfo fN(QString::fromStdString(currSeq->filename()));
     dialog.setDirectory(fN.absolutePath());
     dialog.selectFile(QString(fN.baseName()+"_project.yml"));
    }
    while (dialog.exec() == QDialog::Accepted && !saveProject(dialog.selectedFiles().first())) {}
}

void MainWindow::on_actionSaveHistOfImage_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Positions files (*.txt *.dat)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QFileInfo fN(QString::fromStdString(currSeq->filename()));
    dialog.setDirectory(fN.absolutePath());
    dialog.selectFile(QString(fN.baseName()+"_"+QString::number(currSeq->currentFrame())+"_hist.txt"));
    while (dialog.exec() == QDialog::Accepted && !saveHistogramToFile(dialog.selectedFiles().first())) {}
}

void MainWindow::on_actionSaveTracks_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Tracks files (*.txt *.dat)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QFileInfo fN(QString::fromStdString(currSeq->filename()));
    dialog.setDirectory(fN.absolutePath());
    dialog.selectFile(QString(fN.baseName()+"_trk.txt"));
    while (dialog.exec() == QDialog::Accepted && !saveTracksFile(dialog.selectedFiles().first())) {}
}

void MainWindow::on_actionSavePGM_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("PNG Image File (*.png)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QFileInfo fN(QString::fromStdString(currSeq->filename()));
    dialog.setDirectory(fN.absolutePath());
    while (dialog.exec() == QDialog::Accepted && !savePGMFile(dialog.selectedFiles().first())) {}
}

void MainWindow::on_actionSavePGMMetadata_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("PNG Image File (*.png)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QFileInfo fN(QString::fromStdString(currSeq->filename()));
    dialog.setDirectory(fN.absolutePath());
    while (dialog.exec() == QDialog::Accepted && !savePGMFileWithMetadata(dialog.selectedFiles().first())) {}
}

void MainWindow::on_actionSavePGMSelection_triggered()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("PNG Image File (*.png)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QFileInfo fN(QString::fromStdString(currSeq->filename()));
    dialog.setDirectory(fN.absolutePath());
    while (dialog.exec() == QDialog::Accepted && !savePGMSelectionFile(dialog.selectedFiles().first())) {}
}

void MainWindow::on_actionFrom_triggered()
{
    frameSelect->setFrom(m_player->getCurrentFrameNo());
}

void MainWindow::on_fromButton_clicked(int i)
{
    if(m_player->useMetaMode())
    {
      frameSelect->metaMode(m_player->useMetaMode());
      frameSelect->setMin(m_player->metaMin());
      frameSelect->setMax(m_player->metaMax());
      frameSelect->setFrom(i);
    }
    else
    {
    if(frameSelect->getMetaMode())
      frameSelect->metaMode(m_player->useMetaMode());
    frameSelect->setFrom(i);
    }
}

void MainWindow::on_toButton_clicked(int i)
{
    if(m_player->useMetaMode())
    {
      frameSelect->metaMode(m_player->useMetaMode());
      frameSelect->setMin(m_player->metaMin());
      frameSelect->setMax(m_player->metaMax());
      frameSelect->setTo(i);
    }
    else
    {
    if(frameSelect->getMetaMode())
      frameSelect->metaMode(m_player->useMetaMode());
    frameSelect->setTo(i);
    }
}

void MainWindow::on_actionShowHist_triggered()
{
   int maxi;
   vector<vector<int> > hist;
   if(ith_changed || currSeq->getRequired()!=useIth+1)
   {
     currSeq->setRequired(useIth+1);
     ith_changed=0;
   }
   int n1=0;
   int n2=useIth;
   if(blackOnWhite)
   {
    n1=n2;
    n2=0;
   }
   Image<short int> tmp=currSeq->diffImage(n1,n2);
   hist=tmp.dHistogram(&maxi);
   Histogram->SetP(fracN,(double)(minThreshold+hist[maxi][0]));
   Histogram->setData(hist,maxi);
   HistoDock->show();
}

void MainWindow::on_actionShowHistOfImage_triggered()
{
   int maxi;
   vector<vector<int> > hist;
   Image<unsigned char>* timg = currSeq->getImagePointer(0);
   hist=timg->dHistogram(&maxi);
   Histogram->setData(hist,maxi);
   HistoDock->show();
   int aw=7;
   double minh=2000;
   int *h0;h0=new int[256];for(int i=0; i<256; i++)h0[i]=0;
   double *h1;h1=new double[256];for(int i=0; i<256; i++)h1[i]=0;
   for(int i=0; i<hist.size(); i++) h0[hist[i][0]]=hist[i][1];
   for(int i=0; i<256;i++)
   {
    int avg=0;
    int navg=0;
    for(int j=-aw; j<=aw; j++)
     if(i+j>=0 && i+j<256)
    {
     avg+=h0[i+j];
     navg++;
    }
    if(navg>0)
     if(double(avg)/double(navg)>minh)
      h1[i]=double(avg)/double(navg);
   }
   vector<int> maxima;
   for(int i=1; i<255;i++)
   {
    if(h1[i-1]<h1[i] && h1[i]>=h1[i+1])
     maxima.push_back(i);
   }

   vector<int> minima;
   for(int i=0; i<maxima.size()-1; i++)
   {
    int min=h0[maxima[i]];
    int nmin=maxima[i];
    for(int j=maxima[i]; j<maxima[i+1]; j++)
     if(h0[j]<min)
     {
      min=h0[j];
      nmin=j;
     }
    minima.push_back(nmin);
   }
   cerr<<"I found "<<maxima.size()<<" local maxima:\n";
   for(int i=0; i<maxima.size(); i++) cerr<<i <<"->"<<maxima[i]<<"\n";
   cerr<<"I found "<<minima.size()<<" in between minima:\n";
   for(int i=0; i<minima.size(); i++) cerr<<i <<"->"<<minima[i]<<"\n";
   
   vector<int> localMinima;
   if(maxima.size()>0)
   {
    for(int i=1; i<255; i++)
     if(h0[i]<h0[i-1] && h0[i]<h0[i+1])
    {
     localMinima.push_back(i);
    }
   }
   if(localMinima.size()>0)
   {
    if(localMinima.size()>=4)
    {
     for(int i=0; i<4; i++) cerr<<i <<"->"<<localMinima[i]<<"\n";
    }
    else
    {
     cerr<<"First minima located at "<< localMinima[0]<<"\n";
    }
   }
   else
    cerr<<"Eror: No local minimum found\n";
   delete[] h0;
   delete[] h1;  
}

void MainWindow::on_actionCleanCluster_triggered()
{
    MosquitoesInFrame* currentFrame=currMosqPos->mosqInFrameI(m_player->getCurrentFrameNo());
    if(cleanCluster!=0)
    {
        backupPos(currSeqNum);
        int nr=currentFrame->clean(cleanCluster);
    }
    m_player->showImageWithPoints(m_player->getCurrentFrameNo());
}

void MainWindow::on_actionCleanAllCluster_triggered()
{
  backupPos(currSeqNum);
  if(cleanCluster!=0)
    for(unsigned long i=0; i<currMosqPos->getNoOfFrames();i++)
    {
       MosquitoesInFrame* currentFrame=currMosqPos->mosqInFrameI(i,true);
       int nr=currentFrame->clean(cleanCluster);
    }
  m_player->showImageWithPoints(m_player->getCurrentFrameNo());
}

void MainWindow::on_actionResetPositions_triggered()
{
  currMosqPos->removeAll();
  currSeq->setPosfilename(string());
}

void MainWindow::on_actionResetTracks_triggered()
{
  currReg->reset();
  currSeq->setTrackfilename(string());
}

void MainWindow::on_actionShowDiffImage_triggered()
{

   int maxi;
   vector<vector<int> > hist;
   if(ith_changed)
   {
     currSeq->setRequired(useIth+1);
     ith_changed=0;
   }

   int n1=0;
   int n2=useIth;
   if(blackOnWhite)
   {
    n1=n2;
    n2=0;
   }
   Image<short int> tmp=currSeq->diffImage(n1,n2);
   hist=tmp.dHistogram(&maxi);
   if(hist.size())
   {
        double threshold=hist[maxi][0]+fracN*(hist[hist.size()-1][0]-hist[maxi][0]);
        double maxdiff=hist[hist.size()-1][0];

        if(threshold<minThreshold)threshold=minThreshold;
        tmp.Threshold(threshold);
        tmp.erodeAndDilate(erode,dilute);
        Image<unsigned char> display=*currSeq->getImagePointer(0);
        vector<std::vector<Point> > centers=tmp.removeSmallBlobsContours(minA,maxA,threshold,maxdiff);
        Rect* roi=currSeq->getRoi();
        if(roi)
        {
        for(unsigned int i=0; i<centers.size(); i++)
        {
         for(unsigned int j=0; j<centers[i].size(); j++)
         {
         centers[i][j].x+=roi->x;
         centers[i][j].y+=roi->y;
         }
        }
        }
        Mat tmpM=display.image();
        for (unsigned long i = 0; i < centers.size(); i++)
        {
            drawContours(tmpM, centers, i, Scalar(255), 1);
        }
        m_player->showImage(tmpM, currSeq->currentFrame(),0,0);
   }
}

void MainWindow::on_actionShowPoints_triggered()
  {
   if(!currSeq->hasMeta() || (displayParam->getArrayWidth()==1 && displayParam->getArrayHeight()==1))
   {
    Image<unsigned char>* tmpi=currSeq->getImagePointer(0);
    unsigned char* buffer=tmpi->bufferPointer();
    Mat tmp = Mat(tmpi->getY(), tmpi->getX(), CV_8UC1, &buffer[0]);
    if(!currSeq->hasMeta()) 
     displayAllPointsWidget->initialize(currMosqPos,tmp,fromFrame,toFrame,FrameStep,currSeq,nativeFD);
    else
     displayAllPointsWidget->initialize(currMosqPos,tmp,fromFrame,toFrame,FrameStep,currSeq,nativeFD,false,currMosqPos->getFirstFrameWithMosquito(),currMosqPos->getLastFrameWithMosquito());
    if(currReg)
     displayAllPointsWidget->giveRegistryPointer(currReg);
    displayAllPointsWidget->show();
   }
   else{
   composer.setC(displayParam->getArrayWidth());
   composer.setR(displayParam->getArrayHeight());

   for(int i=0; i<displayParam->getArrayWidth();i++)
    for(int j=0; j<displayParam->getArrayHeight();j++)
   {
    iSeq* in=m_player->getSeqPointer(i,j);
    if(in)
    {
     composer.setSeq(in,i,j);
    }
    MosquitoesInSeq* pin=m_player->getMosqPointer(i,j);
    if(pin)
    {
     composer.setP(pin,i,j);
    }
    MosquitoRegistry* tin=m_player->getTrackPointer(i,j);
    if(tin)
    {
     composer.setT(tin,i,j);
    }
   }
   int trigger=currSeq->metaData(m_player->getCurrentFrameNo());
   Image<unsigned char>* tmpi=composer.getImage(trigger);
   unsigned char* buffer=tmpi->bufferPointer();
   Mat tmp = Mat(tmpi->getY(), tmpi->getX(), CV_8UC1, &buffer[0]);
   composer.setTimeBasedOnCurrentSeq(0,0,1000);
   cerr<<"initializing displayAllPointsWidget with trigger range of "<<composer.getMinTrigger()<<" to "<<composer.getMaxTrigger()<<"\n";
   displayAllPointsWidget->initialize(composer.currMosqPos(),tmp,currSeq->metaData(fromFrame),currSeq->metaData(toFrame),FrameStep,currSeq,nativeFD,true,composer.getMinTrigger(),composer.getMaxTrigger());
   displayAllPointsWidget->giveRegistryPointer(composer.currTracks());
   displayAllPointsWidget->show();
   }
}

void MainWindow::on_actionShowTable_triggered()
{
        displayTable();
        m_pTableWidget->show();
}

void MainWindow::on_actionShowTableOfAll_triggered()
{
        displayTableOfAll();
        m_pTableWidget->show();
}

void MainWindow::on_actionShowTableOfTracks_triggered()
{
        displayTableOfTracks();
        m_tTableWidget->show();
}

void MainWindow::on_actionTo_triggered()
{
    frameSelect->setTo(m_player->getCurrentFrameNo());
}

void MainWindow::on_actionCopySettings_triggered()
{
    saveSegSettingsToAll();
}

void MainWindow::on_actionProcessAll_triggered()
{
    if(seq.size()>0)
    {
       process_all=1;
       processSeqI(0);
    }
}

void MainWindow::update_minA(double v)
{
    minA=v;
}

void MainWindow::update_maxA(double v)
{
    maxA=v;
}

void MainWindow::update_fracN(double v)
{
    fracN=v;
}

void MainWindow::update_minThreshold(double v)
{
    minThreshold=v;
}

void MainWindow::update_useIth(int i)
{
    useIth=i;
    ith_changed=1;
}
void MainWindow::update_erode(int i)
{
    erode=i;
}

void MainWindow::update_cleanCluster(double v)
{
    cleanCluster=v;
}

void MainWindow::update_displayEachNFrames(int i)
{
    displayEachN=i;
}

void MainWindow::update_dilute(int i)
{
    dilute=i;
}

void MainWindow::on_testParams_clicked()
{
if(seq.size()>0)
{
    backupPos(currSeqNum);
    if(ith_changed)
    {
        currSeq->setRequired(useIth+1);
        ith_changed=0;
    }
    workFrame=currSeq->currentFrame();
    MosquitoesInFrame* currentFrame=currMosqPos->mosqInFrameI(workFrame);
    currentFrame->clear();
    currentFrame->setMinArea(minA);
    currentFrame->setMaxArea(maxA);
    currentFrame->setThreshold(fracN);
    currentFrame->setMinThreshold(minThreshold);
    currentFrame->setFrameNo(workFrame);
    currentFrame->setTsSec(currSeq->sec(0));
    currentFrame->setTsMSec(currSeq->ms(0));
    currentFrame->setTsUSec(currSeq->us(0));
    int m1=segParam->getMedianBlur1Param();
    int m2=segParam->getMedianBlur2Param();
    int g1=segParam->getGaussianBlur1Param();
    double s1=segParam->getGaussianSigma1Param();
    int g2=segParam->getGaussianBlur2Param();
    double s2=segParam->getGaussianSigma2Param();
    if(currSeq->getRoi())
    {
    Rect tmp=*currSeq->getRoi();
    worker->process(currSeq->getImagePointer(0), currSeq->getImagePointer(useIth), minA, maxA,fracN,minThreshold,erode,dilute,blackOnWhite,tmp,m1,m2,g1,s1,g2,s2);    
    }
    else
    {
    worker->process(currSeq->getImagePointer(0), currSeq->getImagePointer(useIth), minA, maxA,fracN,minThreshold,erode,dilute,blackOnWhite,m1,m2,g1,s1,g2,s2);
    }

}
}

void MainWindow::on_startParams_clicked()
{
if(seq.size()>0)
{
    saveSettingsToFile(currSeq->settingsName());
    currSeq->openBackupFile();
    if(ith_changed)
    {
        currSeq->setRequired(useIth+1);
        ith_changed=0;
    }
    if(fromFrame<(unsigned)useIth) fromFrame=useIth;
    workFrame=fromFrame;
    int numTasks = toFrame-fromFrame+1 ;
    DisplayProgress=new QProgressDialog("Processing Sequence...", "Cancel", 0, numTasks, this);
    DisplayProgress->setWindowModality(Qt::WindowModal);
    backupPos(currSeqNum);
    timerID2=startTimer(0);
}
}

void MainWindow::on_blackOnWhite_checked(int i)
{
 blackOnWhite=i;
}

void MainWindow::update_TrackMinA(double v)
{
    TrackMinA=v;
}

void MainWindow::update_TrackMaxA(double v)
{
    TrackMaxA=v;
}

void MainWindow::update_TrackMinD(double v)
{
    TrackMinD=v;
}

void MainWindow::update_TrackMaxD(double v)
{
    TrackMaxD=v;
}

void MainWindow::update_TrackMaxG(int i)
{
    TrackMaxG=i;
}

void MainWindow::update_TrackMaxN(int i)
{
    TrackMaxN=i;
}

void MainWindow::update_TrackMaxCL(int i)
{
    TrackMaxCL=i;
}

void MainWindow::update_TrackMinTD(int i)
{
    TrackMinTD=i;
}

void MainWindow::update_TrackAppendOrOverwrite(int i)
{
    appendTracks=i;
}

void MainWindow::on_TrackStartParams_clicked()
{
if(seq.size()>0)
{
backupTracks(currSeqNum);
vector<int> ids;
vector<vector<double> > LostMosquitoes;
vector<int> LostMosquitoesIds;

if(!appendTracks)
  currReg->reset();
  
MosquitoesInSeq tmpP=*currMosqPos;

DisplayProgress=new QProgressDialog("Tracking Sequence...", "Cancel", 0, tmpP.getNoOfFrames(), this);
DisplayProgress->setWindowModality(Qt::WindowModal);

for(unsigned long i=0; i<tmpP.getNoOfFrames();i++)
{
DisplayProgress->setValue(i);
if (DisplayProgress->wasCanceled())
{
 DisplayProgress->setValue(tmpP.getNoOfFrames());
 delete DisplayProgress;
 break;
}
unsigned long trigger=i;
if(tmpP.isDynamic())
 trigger=tmpP.triggerOfIdx(i);
unsigned int n=tmpP.MosqsInFrame(trigger,tmpP.isTriggerBased());

if(i==0)
{
 int cleaned=tmpP.clean(trigger,TrackMinD,tmpP.isTriggerBased());
 cleaned+=tmpP.clean(trigger,TrackMinA,TrackMaxA,tmpP.isTriggerBased());
 n=tmpP.MosqsInFrame(trigger,tmpP.isTriggerBased());
 for(unsigned int ii=0;ii<n;ii++)
  ids.push_back(-1);
}

unsigned long trigger2=i+1;
if(tmpP.isDynamic())
 trigger2=tmpP.triggerOfIdx(i+1);


int cleaned=tmpP.clean(trigger2,TrackMinD,tmpP.isTriggerBased());
cleaned+=tmpP.clean(trigger2,TrackMinA,TrackMaxA,tmpP.isTriggerBased());
vector<vector<double> > a=tmpP.getAllPosAsVector(trigger,tmpP.isTriggerBased());
n=a.size();

int starts=0;
int added=0;
for(unsigned long ii=0; ii<n;ii++)
{
 if(ids[ii]==-1)
 {
  ids[ii]=currReg->registerNewMosquito();
  starts++;
 }
  
 TrackPoint NewTrackPoint(a[ii][0],a[ii][1],a[ii][2],a[ii][3],a[ii][4],a[ii][5],
                          a[ii][6],a[ii][7],a[ii][8],a[ii][9],a[ii][10],a[ii][11],a[ii][12],a[ii][13]);
 currReg->updateTrack(ids[ii],NewTrackPoint);
 added++;
}

if(trigger2!=trigger+1)
{
LostMosquitoes.insert(end(LostMosquitoes), begin(a), end(a));
LostMosquitoesIds.insert(end(LostMosquitoesIds), begin(ids), end(ids));
 vector<int>().swap(ids);
 vector<vector<double> >().swap(a);
}

if(i<tmpP.getNoOfFrames()-1)
{
int m=tmpP.MosqsInFrame(trigger2,tmpP.isTriggerBased());
vector<vector<double> > tmpLostMosquitoes;
vector<int> tmpLostMosquitoesIds;
int stillLost=0;
int completelyLost=0;
for(unsigned int iii=0; iii<LostMosquitoes.size(); iii++)
{
 if(trigger2-LostMosquitoes[iii][2]-1<TrackMaxG)
 {
 tmpLostMosquitoes.push_back(LostMosquitoes[iii]);
 tmpLostMosquitoesIds.push_back(LostMosquitoesIds[iii]);
 stillLost++;
 }
 else
 {
 completelyLost++;
 }
}

a.insert(end(a), begin(tmpLostMosquitoes), end(tmpLostMosquitoes));
ids.insert(end(ids), begin(tmpLostMosquitoesIds), end(tmpLostMosquitoesIds));
n=a.size();
LostMosquitoes=tmpLostMosquitoes;
LostMosquitoesIds=tmpLostMosquitoesIds;
if(n>0 && m>0)
{
	unsigned int nrows = n;
	unsigned int ncols = m;
	vector<vector<double> >  matrix(n,vector<double>(m));
	vector<vector<size_t> >  idx(n,vector<size_t>(m));
        vector<vector<double> > b=tmpP.getAllPosAsVector(trigger2,tmpP.isTriggerBased());
        
        for ( unsigned int row = 0 ; row < nrows ; row++ ) {
		for ( unsigned int col = 0 ; col < ncols ; col++ ) {
		        idx[row][col] = col;
		        double dt=b[col][2]-a[row][2];
			matrix[row][col] = (double)sqrt(pow(b[col][0]-a[row][0],2)+pow(b[col][1]-a[row][1],2)/dt);
		}
	}
        double maxTD=TrackMaxD;
        for ( unsigned int row = 0 ; row < nrows ; row++ ) {
            idx[row].erase(std::remove_if(idx[row].begin(), idx[row].end(),
                [&matrix,&row,&maxTD](unsigned long const& x){
                    if (matrix[row][x]<maxTD)
                        return false;
                    else
                        return true;
                }), idx[row].end());
            sort(idx[row].begin(), idx[row].end(),
                [&matrix,&row](size_t i1, size_t i2) {return matrix[row][i1] < matrix[row][i2];});
            if(idx[row].size()>TrackMaxN)
                idx[row].resize(TrackMaxN);
        }
	vector<vector<double> >  cost;
        for ( unsigned int row = 0 ; row < nrows ; row++ ) {
        vector<double> tmpc;
        vector<vector<double> > move_vector=currReg->TrackMotionVector(row,TrackMaxCL);
         for ( unsigned int col = 0; col < idx[row].size(); col++)
         {
          double dt=b[idx[row][col]][2]-a[row][2];
          if(dt>1)
           dt=1.1*dt;
          if(move_vector.size()>TrackMinTD)
          {
           double dx=(b[idx[row][col]][0]-a[row][0])/dt;
           double dy=(b[idx[row][col]][1]-a[row][1])/dt;
           double tmpval=0;
           for(unsigned int nn=0; nn<move_vector.size();nn++)
            tmpval+=sqrt(pow(move_vector[nn][0]-dx,2)+pow(move_vector[nn][1]-dy,2));
           if(move_vector.size()>0)
            tmpval/=move_vector.size();
           tmpc.push_back(tmpval);
          }
          else
          {
           tmpc.push_back(sqrt(pow(b[idx[row][col]][0]-a[row][0],2)
                              +pow(b[idx[row][col]][1]-a[row][1],2))/dt);
          }
         }
         cost.push_back(tmpc);
        }
        vector<costOfConnection> coc;
        for (unsigned int row = 0 ; row < nrows ; row++ ) {
            for ( unsigned int col = 0 ; col < cost[row].size() ; col++ ) {
                costOfConnection tmpCost;
                tmpCost.cost=cost[row][col];
                tmpCost.source=row;
                tmpCost.target=idx[row][col];
                coc.push_back(tmpCost);
            }
        }
        sort(coc.begin(), coc.end(), sortCostOfConnectionByCost);
        vector<costOfConnection> selectedConnections;
        for(unsigned int nn=0; nn< coc.size(); nn++)
        {
            int valid=0;
            for(unsigned int mm=0;mm<selectedConnections.size();mm++)
            {
                if (coc[nn].source == selectedConnections[mm].source ||
                    coc[nn].target == selectedConnections[mm].target)
                {
                    valid=1;
                    break;
                }
            }
            if(valid==0)
                selectedConnections.push_back(coc[nn]);
        }
        vector<int> tmpVec;
        for (unsigned int col = 0 ; col < ncols ; col++ )
          tmpVec.push_back(-1);
        vector<int> tmprows;
        for (unsigned int row = 0 ; row < nrows ; row++ )
          tmprows.push_back(-1);
        
        for ( unsigned int nn = 0 ; nn < selectedConnections.size() ; nn++ )
        {
         tmpVec[selectedConnections[nn].target]=ids[selectedConnections[nn].source];
         tmprows[selectedConnections[nn].source]=selectedConnections[nn].target;
        }
        vector<vector<double> > lost;
        vector<int> lostIds;
        for (unsigned int row = 0 ; row < nrows ; row++ )
        {
         if(tmprows[row]==-1)
         {
          lost.push_back(a[row]);
          lostIds.push_back(ids[row]);
         }
        }
        LostMosquitoes=lost;
        LostMosquitoesIds=lostIds;
        ids=tmpVec;
}
else{
LostMosquitoes=a;
LostMosquitoesIds=ids;
ids.clear();
for(int jj=0;jj<m;jj++)
 ids.push_back(-1);
}
}
}
    DisplayProgress->setValue(tmpP.getNoOfFrames());
    delete DisplayProgress;
    m_player->showImageWithPoints(m_player->getCurrentFrameNo());
}
if(showAllPoints)
           m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
}

void MainWindow::on_selectCurrent(int i)
{
    if(currSeqNum!=i && i<(int)seq.size())
        switchSeq(i);
    else if(seqFileChanged && i<(int)seq.size())
    {
        switchSeq(i);
        seqFileChanged=false;
    }
}

void MainWindow::on_readyToRunNext_triggered()
{
 if(process_all)
 {
 if(currSeqNum+1<int(seq.size()))
  processSeqI(currSeqNum+1);
 else
  process_all=0;
 }
}

void MainWindow::removePointClicked(double x, double y, int c, int r)
{
    if(seq.size()>0)
    {
        if(showAllPoints)
        {
         cerr<<"Removing point from range "<< fromFrame <<"->"<<toFrame<<"\n";
         backupPos(currSeqNum);
         currMosqPos->removePoints(x,y,fromFrame,toFrame);
        }
        else
        {
         MosquitoesInFrame* currentFrame=currMosqPos->mosqInFrameI(m_player->getCurrentFrameNo());
         backupPos(currSeqNum);
         currentFrame->removeMosquito(x, y);
        }
    }
}

void MainWindow::addPointClicked(double x, double y)
{
    if(seq.size()>0)
    {
        MosquitoesInFrame* currentFrame=currMosqPos->mosqInFrameI(m_player->getCurrentFrameNo());
        backupPos(currSeqNum);
        currentFrame->addMosquito(x, y, -1.0,-1.0);
    }
}

void MainWindow::on_displayShowPointsClicked(int i)
{
 if(seq.size()>0)
 {  
  Viewer::DisplaySettings dset=m_player->getDisplaySettings();
  dset.showPoints=i;
  m_player->setDisplaySettings(dset);
  m_player->ForceUpdateFrame(currSeq->currentFrame());
  if(showAllPoints)
   m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
 }
}

void MainWindow::on_displayShowTracksClicked(int i)
{
 if(seq.size()>0)
 {  
  Viewer::DisplaySettings dset=m_player->getDisplaySettings();
  dset.showTracks=i;
  m_player->setDisplaySettings(dset);
  m_player->ForceUpdateFrame(currSeq->currentFrame());
  if(showAllPoints)
   m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
 }
}

void MainWindow::on_displayPointSizeChanged(double v)
{
 if(seq.size()>0)
 {  
  Viewer::DisplaySettings dset=m_player->getDisplaySettings();
  dset.pointSize=v;
  m_player->setDisplaySettings(dset);
  m_player->ForceUpdateFrame(currSeq->currentFrame());
  if(showAllPoints)
   m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
 }
}

void MainWindow::on_displayTrackWidthChanged(double v)
{
 if(seq.size()>0)
 {  
  Viewer::DisplaySettings dset=m_player->getDisplaySettings();
  dset.lineWidth=v;
  m_player->setDisplaySettings(dset);
  m_player->ForceUpdateFrame(currSeq->currentFrame());
  if(showAllPoints)
   m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
 }
}

void MainWindow::on_displayArrayWidthChanged(int i)
{
  if(seq.size()>0)
  {
  if(currSeq->hasMeta())
      m_player->setColumns(i);
  else
     displayParam->setArrayWidth(1);
  }
}

void MainWindow::on_displayArrayHeightChanged(int i)
{
  if(seq.size()>0)
  {
  if(currSeq->hasMeta())
      m_player->setRows(i);
  else
     displayParam->setArrayHeight(1);
  }
}

void MainWindow::on_displayContoursChanged(int i)
{
 if(seq.size()>0)
 {
  Viewer::DisplaySettings dset=m_player->getDisplaySettings();
  dset.showContours=i;
  m_player->setDisplaySettings(dset);
  m_player->ForceUpdateFrame(currSeq->currentFrame());
  if(showAllPoints)
   m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
 }
}

void MainWindow::on_varPointRadiusChanged(double v)
{
 if(seq.size()>0)
 {
  Viewer::DisplaySettings dset=m_player->getDisplaySettings();
  dset.showPointsVar=v;
  m_player->setDisplaySettings(dset);
  m_player->ForceUpdateFrame(currSeq->currentFrame());
  if(showAllPoints)
   m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
 }
}


void MainWindow::on_displayMinGChanged(int i)
{
 if(seq.size()>0)
 {  
  Viewer::DisplaySettings dset=m_player->getDisplaySettings();
  dset.minG=i;
  m_player->setDisplaySettings(dset);
  m_player->ForceUpdateFrame(currSeq->currentFrame());
  if(showAllPoints)
   m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
 }
}

void MainWindow::on_displayMaxGChanged(int i)
{
 if(seq.size()>0)
 {  
  Viewer::DisplaySettings dset=m_player->getDisplaySettings();
  dset.maxG=i;
  m_player->setDisplaySettings(dset);
  m_player->ForceUpdateFrame(currSeq->currentFrame());
  if(showAllPoints)
   m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
 }
}

void MainWindow::pTableClicked(int row,int col)
{
 unsigned long tmpI=m_pTableWidget->item(row, 5)->text().toULong();
 double tmpX=m_pTableWidget->item(row, 0)->text().toDouble();
 double tmpY=m_pTableWidget->item(row, 1)->text().toDouble();
 if(tmpI<currSeq->allocatedFrames())
 {
   m_player->setCurrentFrameNo(tmpI);
   m_player->ForceUpdateFrame(tmpI); 
   m_player->setHighlight(tmpX,tmpY);
 }  
}

void MainWindow::on_pTable_ContextMenuRequest(const QPoint & P)
{
  if(m_pTableWidget)
  {
  QTableWidgetItem *tmp=m_pTableWidget->itemAt(P);
  if(tmp)
  {     
      unsigned long tmpI=m_pTableWidget->item(tmp->row(), 5)->text().toULong();
      double tmpX=m_pTableWidget->item(tmp->row(), 0)->text().toDouble();
      double tmpY=m_pTableWidget->item(tmp->row(), 1)->text().toDouble();
      MosquitoesInFrame* currentFrame=currMosqPos->mosqInFrameI(tmpI);

      int mosqIndex=currentFrame->mosqIndex(tmpX,tmpY);
      if(mosqIndex>=0 && mosqIndex<currentFrame->numOfMosquitoes())
      {
       QMenu* contextMenu=new QMenu();
       contextMenu-> addAction(tr("Delete"), this,[this,mosqIndex,tmp,currentFrame](){
       backupPos(currSeqNum);
       currentFrame->removeMosquito(mosqIndex);
       m_pTableWidget->removeRow(tmp->row());
       m_player->ForceUpdateFrame(currSeq->currentFrame());
       });
       if(m_pTableWidget)
           contextMenu->exec(m_pTableWidget->mapToGlobal(P));
      }
  }
  }
}

void MainWindow::tTableClicked(int row,int col)
{
 unsigned long tmpI=m_tTableWidget->item(row, 0)->text().toULong();
 m_player->setTrackToHighlight(currReg->getSingleTrack(tmpI));
}

void MainWindow::tTableSelectionChanged()
{
      QItemSelectionModel *select = m_tTableWidget->selectionModel();
      if(select->hasSelection())
      {
       QModelIndexList selected=select->selectedIndexes();
       qSort(selected);
       vector<int> toDisplay;
       QList<QModelIndex>::iterator i;
       for (i = selected.begin(); i != selected.end(); ++i)
       {
        unsigned long tmpII=m_tTableWidget->item(i->row(), 0)->text().toULong();
        toDisplay.push_back((int)tmpII);
       }
       sort( toDisplay.begin(), toDisplay.end() );
       toDisplay.erase( unique( toDisplay.begin(), toDisplay.end() ), toDisplay.end() );
       if(toDisplay.size()>1)
       {
        int c,r;
        bool ret=m_player->findCurrSeq(currSeq,c,r);
        if(ret)
        {
         m_player->drawSelectedTracks(c,r, toDisplay);
         m_player->redraw();
        }
       }
      }
}

void MainWindow::tTableCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
      int c,r;
      bool ret=m_player->findCurrSeq(currSeq,c,r);
      if(ret)m_player->clearTrackToHighlight(c,r);
      QItemSelectionModel *select = m_tTableWidget->selectionModel();
      if(currentRow!=-1)
      {
      if(!select->hasSelection())
      {
       unsigned long tmpI=m_tTableWidget->item(currentRow, 0)->text().toULong();
       m_player->setTrackToHighlight(currReg->getSingleTrack(tmpI));
       if(ret)m_player->drawSelectedTracks(c,r,vector<int>());
      }
      else
      {
       QModelIndexList selected=select->selectedIndexes();
       if(selected.count()==1)      
       {
        unsigned long tmpI=m_tTableWidget->item(currentRow, 0)->text().toULong();
        if(ret)m_player->setTrackToHighlight(currReg->getSingleTrack(tmpI),c,r);       
        if(ret)m_player->drawSelectedTracks(c,r,vector<int>());
       }
      }
      }
}

void MainWindow::on_tTable_ContextMenuRequest(const QPoint & P)
{
  if(m_tTableWidget)
  {
  QTableWidgetItem *tmp=m_tTableWidget->itemAt(P);
  if(tmp)
  {     
      unsigned long tmpI=m_tTableWidget->item(tmp->row(), 0)->text().toULong();
      unsigned long fromF=m_tTableWidget->item(tmp->row(), 2)->text().toULong();
      unsigned long toF=m_tTableWidget->item(tmp->row(), 3)->text().toULong();
      QMenu* contextMenu=new QMenu();
      
      contextMenu-> addAction(tr("Go To Start"), this,[this,fromF](){
       m_player->setCurrentFrameNo(fromF);
      });
      contextMenu-> addAction(tr("Go To End"), this,[this,toF](){
       m_player->setCurrentFrameNo(toF);
      });
      
      contextMenu-> addAction(tr("SetFrom"), this,[this,fromF](){
       frameSelect->setFromFrame(fromF);
      });
      contextMenu-> addAction(tr("SetTo"), this,[this,toF](){
       frameSelect->setToFrame(toF);
      });
      
      contextMenu-> addAction(tr("Set Range"), this,[this,toF,fromF](){
       frameSelect->setFromFrame(fromF);
       frameSelect->setToFrame(toF);
      });

      contextMenu-> addAction(tr("Select around Start"), this,[this,toF,fromF](){
       frameSelect->setFromFrame(fromF-1000);
       frameSelect->setToFrame(fromF+1000);
      });

      contextMenu-> addAction(tr("Select around End"), this,[this,toF,fromF](){
       frameSelect->setFromFrame(toF-1000);
       frameSelect->setToFrame(toF+1000);
      });
      
      contextMenu-> addAction(tr("Delete"), this,[this,tmpI](){
      QItemSelectionModel *select = m_tTableWidget->selectionModel();
      backupTracks(currSeqNum);
      if(select->hasSelection())
      {
       QModelIndexList selected=select->selectedIndexes();
       qSort(selected);
       vector<unsigned long> toJoin;
       QList<QModelIndex>::iterator i;
       for (i = selected.begin(); i != selected.end(); ++i)
       {
        unsigned long tmpII=m_tTableWidget->item(i->row(), 0)->text().toULong();
        toJoin.push_back(tmpII);
       }
       sort( toJoin.begin(), toJoin.end() );
       toJoin.erase( unique( toJoin.begin(), toJoin.end() ), toJoin.end() );
       if(toJoin.size()>1)
       {
       cerr<<"Deleting Tracks Tracks";
       for(int i=0; i<toJoin.size(); i++) cerr<<" "<<toJoin[i];
       cerr<<"\n";
       for(int i=toJoin.size()-1; i>=0; i--)
        currReg->deleteTrack(toJoin[i]);
       displayTableOfTracks();       
       }
      }
      else
      {
       cerr<<"Removing Track "<<tmpI<<"\n";
       currReg->deleteTrack(tmpI);
      }
      displayTableOfTracks();
      });
      contextMenu-> addAction(tr("Draw Velocity graph"), this,[this,tmpI](){
      Graph->setData(toVectorOfQPointF(currReg->getDataForTrack(tmpI, 0)));
      GraphDock->show();
      });
      contextMenu-> addAction(tr("Join selected"), this,[this,tmpI](){
      backupTracks(currSeqNum);      
      QItemSelectionModel *select = m_tTableWidget->selectionModel();
      if(select->hasSelection())
      {
       QModelIndexList selected=select->selectedIndexes();
       qSort(selected);
       vector<unsigned long> toJoin;
       QList<QModelIndex>::iterator i;
       for (i = selected.begin(); i != selected.end(); ++i)
       {
        unsigned long tmpII=m_tTableWidget->item(i->row(), 0)->text().toULong();
        toJoin.push_back(tmpII);
       }
       sort( toJoin.begin(), toJoin.end() );
       toJoin.erase( unique( toJoin.begin(), toJoin.end() ), toJoin.end() );
       if(toJoin.size()>1)
       {
       cerr<<"Joining Tracks";
       for(int i=0; i<toJoin.size(); i++) cerr<<" "<<toJoin[i];
       cerr<<"\n";
       for(int i=toJoin.size()-1; i>0; i--)
       {
        bool ret=currReg->joinTracks(toJoin[0],toJoin[i]);
        if(!ret) cerr<<"Error: Could not join Tracks...\n";
       }
       displayTableOfTracks();       
       }
      } 
      });
      contextMenu-> addAction(tr("Show selected"), this,[this,tmpI](){
      QItemSelectionModel *select = m_tTableWidget->selectionModel();
      if(select->hasSelection())
      {
       QModelIndexList selected=select->selectedIndexes();
       qSort(selected);
       vector<int> toDisplay;
       QList<QModelIndex>::iterator i;
       for (i = selected.begin(); i != selected.end(); ++i)
       {
        unsigned long tmpII=m_tTableWidget->item(i->row(), 0)->text().toULong();
        toDisplay.push_back((int)tmpII);
       }
       sort( toDisplay.begin(), toDisplay.end() );
       toDisplay.erase( unique( toDisplay.begin(), toDisplay.end() ), toDisplay.end() );
       if(toDisplay.size()>1)
       {
        cerr<<"Displaying Tracks";
        int c,r;
        bool ret=m_player->findCurrSeq(currSeq,c,r);
        cerr<<"Did I found the SeqFile: "<<ret<<"\n";
        cerr<<"at: "<<c<<","<<r<<"\n";
        if(ret)
        {
         m_player->drawSelectedTracks(c,r, toDisplay);
         m_player->redraw();
        }
       }
      } 
      });
      contextMenu-> addAction(tr("Print Statistics"), this,[this,tmpI](){
      QItemSelectionModel *select = m_tTableWidget->selectionModel();
      if(select->hasSelection())
      {
       QModelIndexList selected=select->selectedIndexes();
       qSort(selected);
       vector<int> toDisplay;
       QList<QModelIndex>::iterator i;
       for (i = selected.begin(); i != selected.end(); ++i)
       {
        unsigned long tmpII=m_tTableWidget->item(i->row(), 0)->text().toULong();
        toDisplay.push_back((int)tmpII);
       }
       sort( toDisplay.begin(), toDisplay.end() );
       toDisplay.erase( unique( toDisplay.begin(), toDisplay.end() ), toDisplay.end() );
       if(toDisplay.size()>0)
       {
        cerr<<"Track Statistics:\n";
        int c,r;
        bool ret=m_player->findCurrSeq(currSeq,c,r);
        if(ret)
        {
         for(int n=0; n<toDisplay.size(); n++)
         {
          vector<vector<double> > a=currReg->trackStatistics(currReg->getVectorOfTrackPoints(toDisplay[n]));
          if(a.size()==0)
          {
           cerr<<toDisplay[n]<<" can't caluclate statistics\n";
          }
          else if(a.size()==2)
          {
           cerr<<toDisplay[n]<<" only got 2 points...\n";
           cerr<<"meanSpeed="<<a[0][0]<<"\n";
           cerr<<"meanVelX="<<a[1][0]<<"\n";
           cerr<<"meanVelY="<<a[1][1]<<"\n";
          }
          else if(a.size()==7)
          {
           cerr<<toDisplay[n]<<" got full statistics...\n";
           cerr<<"mS="<<a[0][0]<<"\n";
           cerr<<"avgS="<<a[1][0]<<"\n";
           cerr<<"stdAvgS="<<a[2][0]<<"\n";
           cerr<<"minS="<<a[3][0]<<"\n";
           cerr<<"maxS="<<a[3][1]<<"\n";
           cerr<<"vx="<<a[4][0]<<"\n";
           cerr<<"vy="<<a[4][1]<<"\n";
           cerr<<"stdVx="<<a[5][0]<<"\n";
           cerr<<"stdVy="<<a[5][1]<<"\n";
           cerr<<"minvx="<<a[6][0]<<"\n";
           cerr<<"maxvx="<<a[6][1]<<"\n";
           cerr<<"minvy="<<a[6][2]<<"\n";
           cerr<<"maxvy="<<a[6][3]<<"\n";  
          }
         }
         for(int n=0; n<toDisplay.size(); n++)
         {
          for(int m=n+1; m<toDisplay.size(); m++)
          {
           bool join=currReg->joinBasedOnStatistics(toDisplay[n],toDisplay[m]);
           if(join)
           {
            cerr<<toDisplay[n] <<" and "<<toDisplay[m]<<"could be joined...\n";
           }
          }         
         }
         
        }
       }
      }
      else
      {
          vector<vector<double> > a=currReg->trackStatistics(currReg->getVectorOfTrackPoints(tmpI));
          if(a.size()==0)
          {
           cerr<<tmpI<<" can't caluclate statistics\n";
          }
          else if(a.size()==2)
          {
           cerr<<tmpI<<" only got 2 points...\n";
           cerr<<"meanSpeed="<<a[0][0]<<"\n";
           cerr<<"meanVelX="<<a[1][0]<<"\n";
           cerr<<"meanVelY="<<a[1][1]<<"\n";
          }
          else if(a.size()==7)
          {
           cerr<<tmpI<<" got full statistics...\n";
           cerr<<"mS="<<a[0][0]<<"\n";
           cerr<<"avgS="<<a[1][0]<<"\n";
           cerr<<"stdAvgS="<<a[2][0]<<"\n";
           cerr<<"minS="<<a[3][0]<<"\n";
           cerr<<"maxS="<<a[3][1]<<"\n";
           cerr<<"vx="<<a[4][0]<<"\n";
           cerr<<"vy="<<a[4][1]<<"\n";
           cerr<<"stdVx="<<a[5][0]<<"\n";
           cerr<<"stdVy="<<a[5][1]<<"\n";
           cerr<<"minvx="<<a[6][0]<<"\n";
           cerr<<"maxvx="<<a[6][1]<<"\n";
           cerr<<"minvy="<<a[6][2]<<"\n";
           cerr<<"maxvy="<<a[6][3]<<"\n";  
          }
      
      } 
      });
      if(m_tTableWidget)
         contextMenu->exec(m_tTableWidget->mapToGlobal(P));
  }
  }
}

void MainWindow::update_fromFrame(int i)
{
    fromFrame=i;
    if(m_player)
     m_player->setFromFrame(i);
    if(showAllPoints)
    {
      if(seq.size()>0)m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
    }
}

void MainWindow::update_toFrame(int i)
{
    toFrame=i;
    if(m_player)
     m_player->setToFrame(i);
    if(showAllPoints)
    {
      if(seq.size()>0)m_player->drawAllPoints(-1, -1, frameSelect->getFrom(),frameSelect->getTo());
    }
}

void MainWindow::update_FrameStep(int i)
{
    FrameStep=i;
}

bool MainWindow::switchSeq(int CSeq)
{
    if(syncOnMeta && currSeq->hasMeta())
        currMeta=currSeq->metaData(m_player->getCurrentFrameNo());
    if(CSeq>=0 && seq.size()>(unsigned) CSeq)
    {
        saveSettings(currSeqNum);
        m_player->setRows(1);
        m_player->setColumns(1);
        displayParam->setArrayWidth(1);
        displayParam->setArrayHeight(1);
        m_player->setHaveNoPositions();
        m_player->setHaveNoTracks();
        currSeqNum=CSeq;
        currMosqPos = &MosqPos[currSeqNum];
        currReg = &Reg[currSeqNum];
        currSeq = &seq[currSeqNum];
        seqInfo->setCurrent(currSeqNum);
        frameSelect->update(currSeq->pointerToHeader());
        m_player->reLoadSeq(currSeq);
        m_player->LoadMosqPos(currMosqPos);
        m_player->LoadReg(currReg);
        loadSettings(currSeqNum);
        ith_changed=true;
        m_player->clearPolygons(0,0); 
        return true;
    }
    else
        return false;
}

void MainWindow::setDefaultSegmentationParameter()
{
    segParam->setMinA(defSettings.minA);
    segParam->setMaxA(defSettings.maxA);
    segParam->setFracN(defSettings.fracN);
    segParam->setMThreshold(defSettings.minThreshold);
    segParam->setUseIth(defSettings.useIth);
    segParam->setDilute(defSettings.dilute);
    segParam->setErode(defSettings.erode);
    segParam->setCleanCluster(defSettings.cleanCluster);
    segParam->setBlackOnWhite(defSettings.blackOnWhite);
    segParam->setDisplayEachNFrames(defSettings.displayEachN);
}

void MainWindow::clearCustomSettings()
{
  QSettings qsettings;
  qsettings.clear();
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();
    settings.setValue("recentFileList", files);
    updateRecentFileActions();
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);
    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);
}

void MainWindow::on_displaySyncOnMetaClicked(int i)
{
    syncOnMeta=i;
}

void MainWindow::on_displaySelectOnMetaClicked(int i)
{
  if(seq.size()>0)
  {
    m_player->activateMetaMode(i);
    if(m_player->useMetaMode()!=(bool)i)
    {
     cerr<<"Could not switch MetaDataMode\n";
     displayParam->setSelectOnMeta(m_player->useMetaMode());
    }
    else
    {
     unsigned long cF=frameSelect->getFrom();
     unsigned long cT=frameSelect->getTo();
     frameSelect->metaMode(m_player->useMetaMode());
     frameSelect->setMin(m_player->metaMin());
     frameSelect->setMax(m_player->metaMax());
     frameSelect->setFrom(currSeq->metaData(cF));
     frameSelect->setTo(currSeq->metaData(cT));
    }
  }
}

void MainWindow::displaySeqIn(int i, int c, int r)
{
    m_player->reLoadSeq(&seq[i],c,r);
    m_player->LoadMosqPos(&MosqPos[i],c,r);
    m_player->LoadReg(&Reg[i],c,r);
    m_player->updateFrame(currSeq->currentFrame());
}

void MainWindow::processSeqI(unsigned long i)
{
    if (i<seq.size() && seq.size()>0)
    {
    switchSeq(i);
    saveSettingsToFile(currSeq->settingsName());
    currSeq->openBackupFile();
    ith_changed=1;
    if(ith_changed)
    {
        currSeq->setRequired(useIth+1);
        ith_changed=0;
    }
    if(fromFrame<(unsigned)useIth) fromFrame=useIth;
    workFrame=fromFrame;
    int numTasks = toFrame-fromFrame+1 ;
    DisplayProgress=new QProgressDialog("Processing Sequence...", "Cancel", 0, numTasks, this);
    DisplayProgress->setWindowModality(Qt::WindowModal);

    timerID2=startTimer(0);
    }
}


void MainWindow::updateTableOfTracks()
{
 if(m_tTableWidget)
 {
  initTableOfTracks(currReg->numOfTracks());
  fillTableOfTracks();
 }
}

void MainWindow::on_disablePosBackup(int i)
{
  if(i==0)
   disablePosBackup=true;
  else
   disablePosBackup=false;
}

void MainWindow::on_CreateAnimationParams_clicked()
{
  if(seq.size()>0)
  {
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("MP4 Video File (*.mp4)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QFileInfo fN(QString::fromStdString(currSeq->filename()));
    dialog.setDirectory(fN.absolutePath());
    while (dialog.exec() == QDialog::Accepted && !save_animation(dialog.selectedFiles().first())) {}
  }
}
