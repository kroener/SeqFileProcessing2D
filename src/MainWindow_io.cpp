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
#include "MyVideoWriter.hpp"

using namespace std;
using namespace cv;

bool MainWindow::loadFile(const QString &fileName)
{
    showAllPoints=false;
    m_player->clearPolygons(0,0); 
    seq.push_back(iSeq());
    MosqPos.push_back(MosquitoesInSeq());
    Reg.push_back(MosquitoRegistry());
    vector<MosquitoesInSeq> tmp;
    MosqPosHist.push_back(tmp);
    vector<MosquitoRegistry> tmp2;
    RegHist.push_back(tmp2);
    settings.push_back(defSettings);
    unsigned long lastCurrMosqPos=currSeqNum;
    currSeqNum=seq.size()-1;
    if(currSeqNum>0){
    saveSettings(lastCurrMosqPos);
    }
    currMosqPos = &MosqPos[currSeqNum];
    currReg = &Reg[currSeqNum];
    currSeq = &seq[currSeqNum];
    int status=currSeq->open(fileName.toUtf8().constData());
    if(status==1)
    {
      seq.pop_back();
      MosqPos.pop_back();
      Reg.pop_back();
      RegHist.pop_back();
      MosqPosHist.pop_back();  
      settings.pop_back();
      currSeq=nullptr;
      currMosqPos=nullptr;
      currReg=nullptr;
      QMessageBox msgBox;
      msgBox.setText("Error: cannot read file..");
      msgBox.exec();
      return false;
    }
    seqInfo->updateInfo(fileName.toUtf8().constData(),currSeq->pointerToHeader());
    seqInfo->addToList(fileName.toUtf8().constData());
    seqInfo->setCurrent(currSeqNum);
    frameSelect->update(currSeq->pointerToHeader());
    currMosqPos->setNoOfFrames(currSeq->allocatedFrames());
    currMosqPos->giveSeqPointer(currSeq);
    if(currSeqNum==0)
    {
        m_player->setHaveNoPositions();
        m_player->setHaveNoTracks();
        m_player->LoadSeq(currSeq);
    }
    else
    {
        m_player->setHaveNoPositions();
        m_player->setHaveNoTracks();
        m_player->LoadSeq(currSeq);
    }
    m_player->LoadMosqPos(currMosqPos);
    m_player->LoadReg(currReg);
    isOpen=true;
    ith_changed=true;
    if(currSeqNum==0)
    {
    loadPositionsAct->setEnabled(true);
    loadPositionsRangeAct->setEnabled(true);
    loadDynamicPositionsAct->setEnabled(true);
    savePositionsAct->setEnabled(true);
    savePositions4Act->setEnabled(true);
    saveProjectAct->setEnabled(true);
    loadTracksAct->setEnabled(true);
    saveTracksAct->setEnabled(true);
    closeAct->setEnabled(true);
    editMenu->setEnabled(true);
    }
    string sname=currSeq->settingsName();
    ifstream fs(sname.c_str());
    if(fs.good())
    {
        fs.close();
        QMessageBox msgBox;
        msgBox.setWindowTitle("loading Settings");
        msgBox.setText("Do you want to load the settings file?\nIf you select \"No\" file will be deleted!");
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if(msgBox.exec() == QMessageBox::Yes){
            loadSettingsFromFile(sname);
        }
        else
        {
            remove(sname.c_str());
        }
    }
    string bname=currSeq->backupName();
    ifstream fb(bname.c_str());
    if(fb.good())
    {
        fb.close();
        QMessageBox msgBox;
        msgBox.setWindowTitle("loading positions");
        msgBox.setText("Do you want to load the backup positions file?\nIf you select \"No\" file will be deleted!");
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if(msgBox.exec() == QMessageBox::Yes){
            fromFrame=currMosqPos->loadFromFile(bname,false,true);
            m_player->setHavePositions();
            frameSelect->setFromFrame(fromFrame);
            m_player->setCurrentFrameNo(fromFrame);
        }
        else
        {
            remove(bname.c_str());
        }
    }
    setCurrentFile(fileName);
    return true;
}

bool MainWindow::loadPositionsFile(const QString &fileName,bool dynamic /*=false */)
{
    currMosqPos->loadFromFile(fileName.toUtf8().constData(),dynamic,true);
    currSeq->setPosfilename(fileName.toUtf8().constData());
    m_player->setHavePositions();
    if(dynamic)
    {
     displayParam->setSelectOnMeta(true);
    }
    return true;
}

bool MainWindow::loadPositionsFileRange(const QString &fileName,bool dynamic /*=false */)
{
    currMosqPos->loadFromFile(fileName.toUtf8().constData(),fromFrame,toFrame,dynamic,true);
    currSeq->setPosfilename(fileName.toUtf8().constData());
    m_player->setHavePositions();
    if(dynamic)
    {
     displayParam->setSelectOnMeta(true);
    }
    return true;
}

bool MainWindow::loadTracksFile(const QString &fileName)
{
    currReg->loadFromFile(fileName.toUtf8().constData());
    m_player->setHaveTracks();
    currSeq->setTrackfilename(fileName.toUtf8().constData());
    return true;
}

bool MainWindow::savePositionsFile(const QString &fileName, bool selection /*=false*/)
{

    if(displayParam->getArrayWidth()==1 && displayParam->getArrayHeight()==1)
    {
        if(selection)
        {
         currMosqPos->saveToFile(fileName.toUtf8().constData(),frameSelect->getFrom(),frameSelect->getTo());
        }
        else
        {
         currMosqPos->saveToFile(fileName.toUtf8().constData());
         currSeq->closeBackupFile();
         remove(currSeq->backupName().c_str());
         string nsN=fileName.toUtf8().constData();
         nsN+=".yml";
         rename(currSeq->settingsName().c_str(),nsN.c_str());
         }
         currSeq->setPosfilename(fileName.toUtf8().constData());
    }
    else
    {
     composer.setC(displayParam->getArrayWidth());
     composer.setR(displayParam->getArrayHeight());
     int ncurr=-1;
     int mcurr=-1;
     for(int i=0; i<displayParam->getArrayWidth();i++)
      for(int j=0; j<displayParam->getArrayHeight();j++)
     {
       iSeq* in=m_player->getSeqPointer(i,j);
       if(in)
       {
        composer.setSeq(in,i,j);
       }
       if(in == currSeq)
       {
       ncurr=i;
       mcurr=j;
       }
       MosquitoesInSeq* pin=m_player->getMosqPointer(i,j);
       if(pin)
       {
        composer.setP(pin,i,j);
       }
     }
     int c,r;
     bool ret=m_player->findCurrSeq(currSeq,c,r);
     if(ret)
     {
      composer.setTimeBasedOnCurrentSeq(c,r);
     }

     if(selection)
      composer.currMosqPos()->saveToFile(fileName.toUtf8().constData(),frameSelect->getFrom(),frameSelect->getTo());
     else
      composer.currMosqPos()->saveToFile(fileName.toUtf8().constData());

    }
    return true;
}

bool MainWindow::saveTracksFile(const QString &fileName)
{
    if(displayParam->getArrayWidth()==1 && displayParam->getArrayHeight()==1)
    {
        currReg->saveToFile(fileName.toUtf8().constData());
        currSeq->setTrackfilename(fileName.toUtf8().constData());
    }
    else
    {
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
        composer.currTracks()->saveToFile(fileName.toUtf8().constData());
    }
    return true;
}

bool MainWindow::savePGMFile(const QString &fileName)
{
    m_player->savePGM(fileName.toUtf8().constData());
    return true;
}

bool MainWindow::savePGMFileWithMetadata(const QString &fileName)
{
    m_player->savePGM(fileName.toUtf8().constData());
    if(!(!currSeq->hasMeta() || (displayParam->getArrayWidth()==1 && displayParam->getArrayHeight()==1)))
    {
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
     }
     int c,r;
     bool ret=m_player->findCurrSeq(currSeq,c,r);
     if(ret)composer.setTimeBasedOnCurrentSeq(c,r);
    }
    Metadata metaOut;
    string basen(fileName.toUtf8().constData());
    basen+= ".metadata";
    int ret=metaOut.create(basen,1);
    int n=0;
    for(int i=currSeq->metaData(frameSelect->getFrom());i<=currSeq->metaData(frameSelect->getTo());i+=FrameStep)
    {
     if(composer.haveImage(i))
     {
      vector<int> ts=composer.getTime(i);
      Metadata::timeValue tv;
      if(ts.size()==3)
      {   
       tv.sec=ts[0];
       tv.msec=ts[1];
       tv.usec=ts[2];
      }
      ret=metaOut.write(n,i,tv);
      n++;
      if(ret!=0)
      {
       cerr<<"Error: Can't write Metadata to file\n";
       break;
      }     

     }
    }

    return true;
}

bool MainWindow::savePGMSelectionFile(const QString &fileName)
{
    QProgressDialog* DisplayProgress;
    int tasks=frameSelect->getTo()-frameSelect->getFrom()+1;
    DisplayProgress=new QProgressDialog("Saving Images...", "Cancel",0, tasks);
    DisplayProgress->setWindowModality(Qt::WindowModal);
    bool haveDialog=true;
    vector<int> compression_params;
    compression_params.push_back(IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    QFileInfo original(fileName);
    for(unsigned int i=frameSelect->getFrom(); i<=frameSelect->getTo(); i++)
    {
    
     QString newPath = original.absolutePath() + QDir::separator() + original.baseName() + QString::number(i).rightJustified(7, '0');
     if (!original.completeSuffix().isEmpty())
      newPath += "." + original.completeSuffix();
     else
      newPath += ".png";
     currSeq->loadImage(i);
     Image<unsigned char>* timg = currSeq->getImagePointer(0);
     imwrite(newPath.toUtf8().constData(), timg->image(0),compression_params);
     int ci = i-frameSelect->getFrom();
     DisplayProgress->setValue(ci);
     if (DisplayProgress->wasCanceled())
     {
      DisplayProgress->setValue(tasks);
      delete DisplayProgress;
      haveDialog=false;
      break;
     }
    }
    if(haveDialog)
    {
      DisplayProgress->setValue(tasks);
      delete DisplayProgress;
      haveDialog=false;
    }
    return true;
}

bool MainWindow::saveHistogramToFile(const QString &fileName)
{
    string filename=fileName.toUtf8().constData();
    int maxi;
    vector<vector<int> > hist;
    Image<unsigned char>* timg = currSeq->getImagePointer(0);
    hist=timg->dHistogram(&maxi);
    FILE * pFile;
    if(!( access( filename.c_str(), F_OK ) != -1 ))
    {
        pFile = fopen (filename.c_str(),"w");
        if(pFile == NULL)
        {
            cerr << "Error file "<<filename<< " does exists!"<<endl;
        }
        else
        {
            for(unsigned long i=0; i<hist.size(); i++)
            {
              fprintf(pFile,"%d\t%d\n",hist[i][0],hist[i][1]);
            }
            fclose(pFile);
        }
    }
    return true;
}

void MainWindow::saveSettings(unsigned long i)
{
 if(i<settings.size())
 {
      settings[i].minA=minA;
      settings[i].maxA=maxA;
      settings[i].fracN=fracN;
      settings[i].minThreshold=minThreshold;
      settings[i].useIth=useIth;
      settings[i].fromFrame=fromFrame;
      settings[i].toFrame=toFrame;
      settings[i].FrameStep=FrameStep;
      settings[i].currFrame=m_player->getCurrentFrameNo();
      settings[i].erode=erode;
      settings[i].dilute=dilute;
      settings[i].cleanCluster=cleanCluster;
      settings[i].blackOnWhite=blackOnWhite;
      settings[i].displayEachN=displayEachN;
 }
}

void MainWindow::saveSettingsToFile(string outputFileName)
{
    FileStorage fs( outputFileName, FileStorage::WRITE );
    fs << "minA" << minA;
    fs << "maxA" << maxA;
    fs << "fracN" << fracN;
    fs << "minThreshold" << minThreshold;
    fs << "useIth" << useIth;
    double tmp=fromFrame;
    fs << "fromFrame" << tmp;
    tmp=toFrame;
    fs << "toFrame" << tmp;
    tmp=FrameStep;
    fs << "FrameStep" << tmp;
    fs << "erode" << erode;
    fs << "dilute" << dilute;
    fs << "cleanCluster" << cleanCluster;
    fs << "blackOnWhite" << blackOnWhite;
}

void MainWindow::loadSettingsFromFile(string inputFileName)
{
    FileStorage fs( inputFileName, FileStorage::READ );
    fs["minA"] >> minA;
    fs["maxA"] >> maxA;
    fs["fracN"] >> fracN;
    fs["minThreshold"] >> minThreshold;
    fs["useIth"] >> useIth;
    double tmp;
    fs["fromFrame"] >> tmp;
    fromFrame=tmp;
    fs["toFrame"] >> tmp;
    toFrame=tmp;
    fs["FrameStep"] >> tmp;
    FrameStep=tmp;
    fs["erode"] >> erode;
    fs["dilute"] >> dilute;
    fs["cleanCluster"] >> cleanCluster;
    fs["blackOnWhite"] >> blackOnWhite;
    segParam->setMinA(minA);
    segParam->setMaxA(maxA);
    segParam->setFracN(fracN);
    segParam->setMThreshold(minThreshold);
    segParam->setUseIth(useIth);
    segParam->setDilute(dilute);
    segParam->setErode(erode);
    segParam->setCleanCluster(cleanCluster);
    segParam->setBlackOnWhite(blackOnWhite);
    frameSelect->setFromFrame(fromFrame);
    frameSelect->setToFrame(toFrame);
    frameSelect->setFrameStep(FrameStep);
}

void MainWindow::saveSegSettings(unsigned long i)
{
 if(i<settings.size())
 {
      settings[i].minA=minA;
      settings[i].maxA=maxA;
      settings[i].fracN=fracN;
      settings[i].minThreshold=minThreshold;
      settings[i].useIth=useIth;
      settings[i].erode=erode;
      settings[i].dilute=dilute;
      settings[i].cleanCluster=cleanCluster;
      settings[i].blackOnWhite=blackOnWhite;
      settings[i].displayEachN=displayEachN;
 }
}

void MainWindow::loadSettings(unsigned long i)
{
 if(i<settings.size())
 {
    segParam->setMinA(settings[i].minA);
    segParam->setMaxA(settings[i].maxA);
    segParam->setFracN(settings[i].fracN);
    segParam->setMThreshold(settings[i].minThreshold);
    segParam->setUseIth(settings[i].useIth);
    segParam->setDilute(settings[i].dilute);
    segParam->setErode(settings[i].erode);
    segParam->setCleanCluster(settings[i].cleanCluster);
    segParam->setBlackOnWhite(settings[i].blackOnWhite);
    segParam->setDisplayEachNFrames(settings[i].displayEachN);
    
    frameSelect->setFromFrame(settings[i].fromFrame);
    frameSelect->setToFrame(settings[i].toFrame);
    frameSelect->setFrameStep(settings[i].FrameStep);
    if(syncOnMeta && currSeq->hasMeta())
    {
     long metaN=currSeq->lookUpTrigger(currMeta);
     if(metaN<0)
     {
      metaN=currSeq->lookUpTrigger(currMeta+metaN);
     }
     if(metaN>=0)
     {
        m_player->setCurrentFrameNo(metaN);
        m_player->ForceUpdateFrame(metaN);
     }
    }
    else
    {
        m_player->setCurrentFrameNo(settings[i].currFrame);
        m_player->ForceUpdateFrame(settings[i].currFrame);
    }
 }
}

void MainWindow::saveSegSettingsToAll()
{
  for(unsigned long i=0; i<settings.size();i++)
    saveSegSettings(i);
}

void MainWindow::save_results(vector<vector<double> > centers, double maxDiff)
{
    MosquitoesInFrame* currentFrame=currMosqPos->mosqInFrameI(workFrame);
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
    if(cleanCluster!=0)
    {
        int nr=currentFrame->clean(cleanCluster);
    }
    currentFrame->setMaxDiff(maxDiff);
    currentFrame->setProcessed(1);
    currSeq->backupCurrentPositions(currentFrame);

    if(displayEachN!=0 && fc%displayEachN==0)
    {
        m_player->showImageWithPoints(workFrame);
        fc=1;
    }
    else
        fc++;
}

void MainWindow::saveCustomSettings()
{
    QSettings qsettings;
    qsettings.setValue("mainWindowGeometry",saveGeometry());
    qsettings.setValue("mainWindowState",saveState());
}

void MainWindow::saveCustomGeometrySettingsToFile(QString f)
{
    QSettings qsettings(f, QSettings::IniFormat);
    qsettings.setValue("mainWindowGeometry",saveGeometry());
    qsettings.setValue("mainWindowState",saveState());
}

void MainWindow::loadCustomSettings()
{
  QSettings qsettings;
    defSettings.minA = qsettings.value("Segmentation/minA",8.0).toDouble();
    defSettings.maxA = qsettings.value("Segmentation/maxA",600.0).toDouble();
    defSettings.fracN = qsettings.value("Segmentation/fracN", 0.15).toDouble();
    defSettings.minThreshold = qsettings.value("Segmentation/minThreshold", 6.0).toDouble();
    defSettings.useIth = qsettings.value("Segmentation/useIth",1).toInt();
    defSettings.erode = qsettings.value("Segmentation/erode", 5).toInt();
    defSettings.dilute = qsettings.value("Segmentation/dilute", 5).toInt();
    defSettings.cleanCluster = qsettings.value("Segmentation/cleanCluster", 0.0).toDouble();
    defSettings.blackOnWhite = qsettings.value("Segmentation/blackonWhite", true).toBool();
    defSettings.displayEachN = qsettings.value("Segmentation/DisplayEachN", 1).toInt();

    trackParam->setMinA(qsettings.value("Tracking/minA",8).toDouble());
    trackParam->setMaxA(qsettings.value("Tracking/maxA",600).toDouble());
    trackParam->setMinD(qsettings.value("Tracking/minD",10).toDouble());
    trackParam->setMaxD(qsettings.value("Tracking/maxD", 100).toDouble());
    trackParam->setMaxG(qsettings.value("Tracking/maxG", 15).toInt());
    trackParam->setMaxN(qsettings.value("Tracking/maxN", 100 ).toInt());
    trackParam->setMaxCL(qsettings.value("Tracking/maxCL", 15).toInt());
    trackParam->setMinTD(qsettings.value("Tracking/minTD",3).toInt());
    trackParam->setAppendOrOverwrite(qsettings.value("Tracking/AppendOrOverwrite", 0).toInt());
    trackParam->setTrackDisplayLength(qsettings.value("Tracking/TDL", 100).toInt());
    trackParam->setMinTrackDisplayLength(qsettings.value("Tracking/minTDL",3).toInt()) ;

    Viewer::DisplaySettings dset=m_player->getDisplaySettings();
    dset.showPoints=qsettings.value("Display/ShowPoints",1).toInt();
    dset.showTracks=qsettings.value("Display/ShowTracks",1).toInt();
    dset.pointSize=(float)qsettings.value("Display/PointSize",10).toDouble();
    dset.lineWidth=(float)qsettings.value("Display/TrackWidth",10).toDouble();
    m_player->setDisplaySettings(dset);
    displayParam->setShowPoints(dset.showPoints);
    displayParam->setShowTracks(dset.showTracks);
    displayParam->setPointSize(dset.pointSize);
    displayParam->setTrackWidth(dset.lineWidth);
    displayAllPointsWidget->setRadius(qsettings.value("DisplayAllPoints/DisplayAllPointsRadius",1).toInt());
}

void MainWindow::loadCustomSettingsFromFile(QString f)
{
  QSettings qsettings(f, QSettings::IniFormat);
    defSettings.minA = qsettings.value("Segmentation/minA",8.0).toDouble();
    defSettings.maxA = qsettings.value("Segmentation/maxA",600.0).toDouble();
    defSettings.fracN = qsettings.value("Segmentation/fracN", 0.15).toDouble();
    defSettings.minThreshold = qsettings.value("Segmentation/minThreshold", 6.0).toDouble();
    defSettings.useIth = qsettings.value("Segmentation/useIth",1).toInt();
    defSettings.erode = qsettings.value("Segmentation/erode", 5).toInt();
    defSettings.dilute = qsettings.value("Segmentation/dilute", 5).toInt();
    defSettings.cleanCluster = qsettings.value("Segmentation/cleanCluster", 0.0).toDouble();
    defSettings.blackOnWhite = qsettings.value("Segmentation/blackonWhite", true).toBool();
    defSettings.displayEachN = qsettings.value("Segmentation/DisplayEachN", 1).toInt();

    trackParam->setMinA(qsettings.value("Tracking/minA",8).toDouble());
    trackParam->setMaxA(qsettings.value("Tracking/maxA",600).toDouble());
    trackParam->setMinD(qsettings.value("Tracking/minD",10).toDouble());
    trackParam->setMaxD(qsettings.value("Tracking/maxD", 100).toDouble());
    trackParam->setMaxG(qsettings.value("Tracking/maxG", 15).toInt());
    trackParam->setMaxN(qsettings.value("Tracking/maxN", 100 ).toInt());
    trackParam->setMaxCL(qsettings.value("Tracking/maxCL", 15).toInt());
    trackParam->setMinTD(qsettings.value("Tracking/minTD",3).toInt());
    trackParam->setAppendOrOverwrite(qsettings.value("Tracking/AppendOrOverwrite", 0).toInt());
    trackParam->setTrackDisplayLength(qsettings.value("Tracking/TDL", 100).toInt());
    trackParam->setMinTrackDisplayLength(qsettings.value("Tracking/minTDL",3).toInt()) ;

    Viewer::DisplaySettings dset=m_player->getDisplaySettings();
    dset.showPoints=qsettings.value("Display/ShowPoints",1).toInt();
    dset.showTracks=qsettings.value("Display/ShowTracks",1).toInt();
    dset.pointSize=(float)qsettings.value("Display/PointSize",10).toDouble();
    dset.lineWidth=(float)qsettings.value("Display/TrackWidth",10).toDouble();
    m_player->setDisplaySettings(dset);
    displayParam->setShowPoints(dset.showPoints);
    displayParam->setShowTracks(dset.showTracks);
    displayParam->setPointSize(dset.pointSize);
    displayParam->setTrackWidth(dset.lineWidth);
    displayAllPointsWidget->setRadius(qsettings.value("DisplayAllPoints/DisplayAllPointsRadius",1).toInt());
}

void MainWindow::saveSettingsAsDefault()
{
    QSettings qsettings;
    qsettings.setValue("Segmentation/minA", minA);
    qsettings.setValue("Segmentation/maxA", maxA);
    qsettings.setValue("Segmentation/fracN", fracN);
    qsettings.setValue("Segmentation/minThreshold", minThreshold);
    qsettings.setValue("Segmentation/useIth", useIth);
    qsettings.setValue("Segmentation/erode", erode);
    qsettings.setValue("Segmentation/dilute", dilute);
    qsettings.setValue("Segmentation/cleanCluster", cleanCluster);
    qsettings.setValue("Segmentation/blackonWhite", blackOnWhite);
    qsettings.setValue("Segmentation/DisplayEachN", displayEachN);

    qsettings.setValue("Tracking/minA", trackParam->getMinA());
    qsettings.setValue("Tracking/maxA", trackParam->getMaxA());
    qsettings.setValue("Tracking/minD", trackParam->getMinD());
    qsettings.setValue("Tracking/maxD", trackParam->getMaxD());
    qsettings.setValue("Tracking/maxG", trackParam->getMaxG());
    qsettings.setValue("Tracking/maxN", trackParam->getMaxN());
    qsettings.setValue("Tracking/maxCL", trackParam->getMaxCL());
    qsettings.setValue("Tracking/minTD", trackParam->getMinTD());
    qsettings.setValue("Tracking/AppendOrOverwrite", trackParam->getAppendOrOverwrite());
    qsettings.setValue("Tracking/TDL", trackParam->getTrackDisplayLength());
    qsettings.setValue("Tracking/minTDL", trackParam->getMinTrackDisplayLength());

    Viewer::DisplaySettings dset=m_player->getDisplaySettings();
    qsettings.setValue("Display/ShowPoints", dset.showPoints);
    qsettings.setValue("Display/ShowTracks", dset.showTracks);
    qsettings.setValue("Display/PointSize", (double)dset.pointSize);
    qsettings.setValue("Display/TrackWidth", (double)dset.lineWidth);
    qsettings.setValue("DisplayAllPoints/DisplayAllPointsRadius", displayAllPointsWidget->getRadius());
}

void MainWindow::saveCustomSettingsToFile(QString f)
{
    QSettings qsettings(f, QSettings::IniFormat);
    qsettings.setValue("Segmentation/minA", minA);
    qsettings.setValue("Segmentation/maxA", maxA);
    qsettings.setValue("Segmentation/fracN", fracN);
    qsettings.setValue("Segmentation/minThreshold", minThreshold);
    qsettings.setValue("Segmentation/useIth", useIth);
    qsettings.setValue("Segmentation/erode", erode);
    qsettings.setValue("Segmentation/dilute", dilute);
    qsettings.setValue("Segmentation/cleanCluster", cleanCluster);
    qsettings.setValue("Segmentation/blackonWhite", blackOnWhite);
    qsettings.setValue("Segmentation/DisplayEachN", displayEachN);

    qsettings.setValue("Tracking/minA", trackParam->getMinA());
    qsettings.setValue("Tracking/maxA", trackParam->getMaxA());
    qsettings.setValue("Tracking/minD", trackParam->getMinD());
    qsettings.setValue("Tracking/maxD", trackParam->getMaxD());
    qsettings.setValue("Tracking/maxG", trackParam->getMaxG());
    qsettings.setValue("Tracking/maxN", trackParam->getMaxN());
    qsettings.setValue("Tracking/maxCL", trackParam->getMaxCL());
    qsettings.setValue("Tracking/minTD", trackParam->getMinTD());
    qsettings.setValue("Tracking/AppendOrOverwrite", trackParam->getAppendOrOverwrite());
    qsettings.setValue("Tracking/TDL", trackParam->getTrackDisplayLength());
    qsettings.setValue("Tracking/minTDL", trackParam->getMinTrackDisplayLength());

    Viewer::DisplaySettings dset=m_player->getDisplaySettings();
    qsettings.setValue("Display/ShowPoints", dset.showPoints);
    qsettings.setValue("Display/ShowTracks", dset.showTracks);
    qsettings.setValue("Display/PointSize", (double)dset.pointSize);
    qsettings.setValue("Display/TrackWidth", (double)dset.lineWidth);
    qsettings.setValue("DisplayAllPoints/DisplayAllPointsRadius", displayAllPointsWidget->getRadius());
}

void MainWindow::openRecentFile()
{
   QAction *action = qobject_cast<QAction *>(sender());
   if (action)
   {
       loadFile(action->data().toString());
   }
}

bool MainWindow::saveProject(const QString &fileName)
{
    string filename = fileName.toUtf8().constData();
    FileStorage fs(filename,FileStorage::WRITE);
    fs <<"NumberOfSeqs" << (int)seq.size();
    fs<<"seqs"<<"[";
    for(unsigned int i=0; i<seq.size(); i++)
    {
        fs << "{";
        fs << "seqFile" << seq[i].filename();
        if(seq[i].posfilename() != "")
         fs << "positionFile" << seq[i].posfilename();
        if(seq[i].trackfilename() != "")
         fs << "trackFile" << seq[i].trackfilename();
        fs << "}";
    }
    fs<<"]";
    fs<<"currSeq"<<currSeq->filename();
    fs<<"currFrame"<<(int)currSeq->currentFrame();
    fs<<"dispW"<<m_player->getColumns();
    fs<<"dispH"<<m_player->getRows();
    fs<<"views"<<"[";
    for(int i=0; i<m_player->getColumns(); i++)
     for(int j=0; j<m_player->getRows(); j++)
    {
        if(m_player->getSeqPointer(i,j))
        {
        fs << "{";
        fs << "i" << i;
        fs << "j" << j;
        fs << "seqFile" << m_player->getSeqPointer(i,j)->filename();
        fs << "}";
        }
    }
    fs<<"]";
    fs.release();
 return true;
}

bool MainWindow::loadProject(const QString &fileName, bool append /* =false */)
{
    string filename = fileName.toUtf8().constData();
    FileStorage fs(filename,FileStorage::READ);
    int seqstoload;
    fs["NumberOfSeqs"] >> seqstoload;
    cerr << "I will try to load "<<seqstoload<<"seq files...\n";
    if(seqstoload>0)
    {
     FileNode fn = fs["seqs"];
     for (FileNodeIterator it = fn.begin(); it != fn.end(); it++)
     {
         FileNode item = *it;
         string s;
         item["seqFile"] >> s;
         if(s!="")
           if(loadFile(QString::fromStdString(s)))
           {
             item["positionFile"] >> s;
             if(s!="")
               loadPositionsFile(QString::fromStdString(s));
             item["trackFile"] >> s;
             if(s!="")
               loadTracksFile(QString::fromStdString(s));
           }
     }
     string s;
     fs["currSeq"] >> s;
     cerr<< "currSeq is "<< s<<"!\n";
     
     int n=0;
     if(s!="")
     for(unsigned int i=0;i<seq.size(); i++)
      if(seq[i].filename()==s)
       n=i;
     if(n>=0)
       switchSeq(n);

     fs["currFrame"] >> n;
     cerr<< "File wants to set frame to "<< n<<"!\n";
     
     if(n<currSeq->allocatedFrames() && n>=0)
     {
       m_player->updateFrame(n);
     }
     int w=0;
     int h=0;
     fs["dispW"]>>w;
     fs["dispH"]>>h;
     cerr<< "File wants "<< w<<"x"<< h<<"!\n";
     
     if(!(w==1 && h==1))
     {
       cerr <<" Arranging views...\n";
       bool b=true;
       if(currSeq->hasMeta())
       {
        m_player->setColumns(w);
        m_player->setRows(h);
        displayParam->setArrayWidth(w);
        displayParam->setArrayHeight(h);
        cerr<< " View set to be "<< w<<"x"<< h<<"!\n";
       }
       else
       {
        cerr<< " I can't set the view!\n";
        b=false;
       }
       if(b)
       {
       FileNode fn3 = fs["views"];
       for (FileNodeIterator it3 = fn3.begin(); it3 != fn3.end(); it3++)
             {
                 FileNode item3 = *it3;
                 string cname;
                 int i=-1;
                 int j=-1;
                 item3["i"] >> i;
                 item3["j"] >> j;
                 item3["seqFile"]>>cname;
                 if(i>=0 && j>=0 && i<(int)w && j<(int)h && cname!="")
                 {
                  int seqnum=-1;
                  for(unsigned int x=0; x<seq.size(); x++)
                   if(seq[x].filename()==cname)
                    seqnum=x;
                  if(seqnum>=0)
                  {
                    displaySeqIn(seqnum,i,j);
                  }
                 }
             }
       }
     }
    }
    fs.release();
    return true;
}

void MainWindow::loadGeometryFromFile(QString f)
{
    QSettings qsettings(f, QSettings::IniFormat);
    restoreGeometry(qsettings.value("mainWindowGeometry").toByteArray());
    restoreState(qsettings.value("mainWindowState").toByteArray());
}

void MainWindow::customSettingsFromFile()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(tr("Settings files (*.ini)"));
    QStringList fileNames;
    if (dialog.exec())
    	fileNames = dialog.selectedFiles();
    for (int i = 0; i < fileNames.size(); ++i)
    {
       loadCustomSettingsFromFile(fileNames.at(i));
       loadGeometryFromFile(fileNames.at(i));
    }
}

void MainWindow::customSettingsFromFile(QString f)
{
   loadCustomSettingsFromFile(f);
   loadGeometryFromFile(f);
}

void MainWindow::customSettingsToFile()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Settings files (*.ini)"));
    QStringList fileNames;
    if (dialog.exec())
    	fileNames = dialog.selectedFiles();
    for (int i = 0; i < fileNames.size(); ++i)
    {
       saveCustomSettingsToFile(fileNames.at(i));
       saveCustomGeometrySettingsToFile(fileNames.at(i));
    }
}

bool MainWindow::savePolyToFile(const QString &fileName, vector<cv::Point> poly)
{
    
    FileStorage fs( fileName.toUtf8().constData(), FileStorage::WRITE );
    fs << "polyRoi" << Mat(poly);
    return true;
}

vector<cv::Point> MainWindow::loadPolyFromFile(const QString &fileName)
{    
    FileStorage fs( fileName.toUtf8().constData(), FileStorage::READ );
    Mat tmp;
    fs["polyRoi"] >> tmp;
    vector<Point> outPoly;
    for(int i=0; i< tmp.rows;i++)
    {
     outPoly.push_back(tmp.at<Point>(i,0));
    }
    return outPoly;
}

bool MainWindow::save_animation(const QString &fileName)
{
    int drawTracks=aniParam->getDrawTracks();
    int drawPoints=aniParam->getDrawPoints();
    int addTimeStamp=aniParam->getDrawTime();
    int scaleImage=aniParam->getScaleOutput();
    int colorBy=aniParam->getTrackColorMode();

    Size oSize(currSeq->pointerToHeader()->width(),currSeq->pointerToHeader()->height());
    if(!(!currSeq->hasMeta() || (displayParam->getArrayWidth()==1 && displayParam->getArrayHeight()==1))){
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
     oSize.width=composer.getWidth();
     oSize.height=composer.getHeight();
    }
    if(scaleImage)
    {
     int orig=oSize.width;
     oSize.width=aniParam->getOutputWidth();
     oSize.height*=(double)oSize.width/(double)orig;
    }
    Scalar pointColor=Scalar(aniParam->getColorPointB(),aniParam->getColorPointG(),aniParam->getColorPointR());
    Scalar trackColor=Scalar(aniParam->getColorTrackB(),aniParam->getColorTrackG(),aniParam->getColorTrackR());
    int pRadius=aniParam->getPointRadius();
    int trackThickness=aniParam->getTrackThickness();
    int trackL=aniParam->getTrackLength();
    int minTrackL=aniParam->getTrackMinLength();
    int numTasks = toFrame-fromFrame+1;
    QProgressDialog progress("Saving animation file...", "Cancel", 0, numTasks, this);
    progress.setMinimumDuration(100);
    progress.setWindowModality(Qt::WindowModal);

    MyVideoWriter::Settings vset;
    vset.w=oSize.width;
    vset.h=oSize.height;
    vset.bit_rate=codecParam->getBitrate();
    vset.timebasenum=codecParam->getFrameNum();
    vset.timebaseden=codecParam->getFrameDen();
    vset.useCrf=codecParam->getUseCrf();
    vset.crf=codecParam->getCrf();
    int codecID=codecParam->getCodec();
    int hardwareCodec=codecParam->getHardware();
    vset.by_name=0;
    if(codecID==0)
    {
        vset.video_codec=AV_CODEC_ID_H264;
        if(hardwareCodec==1)
        {
            vset.codec_by_name="h264_nvenc";
            vset.by_name=1;
        }
    }
    else if(codecID==1)
    {
        vset.video_codec=AV_CODEC_ID_HEVC;
        if(hardwareCodec==1)
        {
            vset.codec_by_name="hevc_nvenc";
            vset.by_name=1;
        }
    }
    MyVideoWriter video(fileName.toUtf8().constData(),vset);

    if(!currSeq->hasMeta() || (displayParam->getArrayWidth()==1 && displayParam->getArrayHeight()==1)){
    for(unsigned long i=fromFrame;i<=toFrame;i+=FrameStep)
    {
        currSeq->loadImage(i);
        Image<unsigned char>* timg = currSeq->getImagePointer(0);
        Mat im=timg->image();
        cvtColor(im, im, COLOR_GRAY2RGB);
        if(drawTracks)
        {
             vector<vector<Point2f> > cvtracks=currReg->getAllTracks(i,trackL,minTrackL);
             vector<vector<Scalar> > colors=currReg->getTrackMetaForAni(i,trackL, minTrackL, colorBy, 0, currSeq->allocatedFrames());
             for (unsigned int j=0; j<cvtracks.size(); j++)
             {
                 for(unsigned int jj=0; jj<cvtracks[j].size()-1; jj++)
                 {
                     if(colorBy==0)
                     {
                      line( im, cvtracks[j][jj],  cvtracks[j][jj+1], trackColor, trackThickness, 8 );
                     }
                     else
                     {
                      line( im, cvtracks[j][jj],  cvtracks[j][jj+1], colors[j][jj], trackThickness, 8 );
                     }                     
                 }
             }
        }
        if(drawPoints)
        {
            vector<Point2f> mp=currMosqPos->getAllPos(i);
            for(unsigned j=0; j<mp.size(); j++)
            {
                circle(im,mp[j],pRadius,pointColor,-1,8);
            }
        }
        if(addTimeStamp)
        {
            time_t rawtime=timg->getTsec();
            int msecs=timg->getTmsec();
            int usecs=timg->getTusec();
            struct tm * ptm;
            ptm = gmtime ( &rawtime );
            char tbuf [80];
            strftime (tbuf,80,"%A, %B %d, %Y %H:%M:%S",ptm);
            char buff[100];
            snprintf(buff, sizeof(buff), "%s:%03d:%03d", tbuf,msecs,usecs);
            string buffAsStdStr = buff;
            int fontFace = FONT_HERSHEY_SIMPLEX;
            double fontScale = 1.5;
            int thickness = 1;
            int baseline=0;
            Size textSize = getTextSize(buffAsStdStr, fontFace,
                            fontScale, thickness, &baseline);
            baseline += thickness;
            Point textOrg((im.cols-textSize.width)/2, im.rows-16);
            putText(im, buffAsStdStr, textOrg, fontFace, fontScale, Scalar::all(255), thickness,8);
        }
        if(scaleImage)
        {
            Mat tmp=im.clone();
            cv::resize(tmp,im,oSize);
        }
        video.add(im);

        progress.setValue(i-fromFrame);
        if (progress.wasCanceled())
           break;
    }
    }
    else{
       for(int i=currSeq->metaData(fromFrame);i<=currSeq->metaData(toFrame);i+=FrameStep)
       {
         if(composer.haveImage(i)){
            Image<unsigned char>* timg=composer.getImage(i);
            Mat im=timg->image();
            cvtColor(im, im, COLOR_GRAY2RGB);
            if(drawTracks)
            {
                 vector<vector<Point2f> > cvtracks=composer.currTracks()->getAllTracks(i,trackL,minTrackL);
                 vector<vector<Scalar> > colors=composer.currTracks()->getTrackMetaForAni(i,trackL, minTrackL, colorBy, 0.0, (double) currSeq->allocatedFrames());
                 for (unsigned int j=0; j<cvtracks.size(); j++)
                 {
                     for(unsigned int jj=0; jj<cvtracks[j].size()-1; jj++)
                     {
                         if(colorBy==0)
                         {
                          line( im, cvtracks[j][jj],  cvtracks[j][jj+1], trackColor, trackThickness, 8 );
                         }
                         else
                         {
                          line( im, cvtracks[j][jj],  cvtracks[j][jj+1], colors[j][jj], trackThickness, 8 );
                         }                     
                     }
                 }
            }
            if(drawPoints)
            {
                vector<Point2f> mp=composer.getAllPos(i);
                for(unsigned j=0; j<mp.size(); j++)
                {
                   circle(im,mp[j],pRadius,pointColor,-1,8);
                }
            }
            if(addTimeStamp)
            {
                time_t rawtime=timg->getTsec();
                int msecs=timg->getTmsec();
                int usecs=timg->getTusec();
                struct tm * ptm;
                ptm = gmtime ( &rawtime );
                char tbuf [80];
                strftime (tbuf,80,"%A, %B %d, %Y %H:%M:%S",ptm);
                char buff[100];
                snprintf(buff, sizeof(buff), "%s:%03d:%03d", tbuf,msecs,usecs);
                string buffAsStdStr = buff;
                int fontFace = FONT_HERSHEY_SIMPLEX;
                //FONT_HERSHEY_SCRIPT_SIMPLEX;
                double fontScale = 1.5;
                int thickness = 1;
                int baseline=0;
                Size textSize = getTextSize(buffAsStdStr, fontFace,
                                fontScale, thickness, &baseline);
                baseline += thickness;
                Point textOrg((im.cols-textSize.width)/2, im.rows-16);
                putText(im, buffAsStdStr, textOrg, fontFace, fontScale, Scalar::all(255), thickness,8);
            }
            if(scaleImage)
            {
                Mat tmp=im.clone();
                cv::resize(tmp,im,oSize);
            }
            video.add(im);
            progress.setValue(currSeq->lookUpTrigger(i)-fromFrame);
            if (progress.wasCanceled())
               break;
         }
       }
    }
    progress.setValue(numTasks);
    return true;
}
