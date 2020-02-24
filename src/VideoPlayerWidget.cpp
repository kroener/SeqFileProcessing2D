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
#include "VideoPlayerWidget.h"
#include "Viewer.h"
#include "iSeq.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoRegistry.hpp"
#include "ImageComposer.hpp"
#include <time.h>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QTimerEvent>
#include <QGridLayout>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QApplication>

using namespace std;
using namespace cv;

template<> int Image<unsigned char>::haveWindow=0;
template<> int Image<double>::haveWindow=0;
template<> int Image<short>::haveWindow=0;

VideoPlayerWidget::VideoPlayerWidget()
    : firstButton( new QPushButton ("|<") ),
      lastButton( new QPushButton (">|") ),
      prevButton( new QPushButton ("<") ),
      nextButton( new QPushButton (">") ),
      playButton( new QPushButton ("|>") ),
      loopButton( new QPushButton ("><") ),
      stopButton( new QPushButton ("||") ),
      pslider( new QSlider(this) ),
      fselect( new QSpinBox(this) ),
      fskipselect( new QSpinBox(this) ),
      tdisplay( new QLabel("") ),
      mdisplay( new QLabel("") ),
      msdisplay( new QLabel("") ),
      usdisplay( new QLabel("") ),
      sm_d( new QLabel(":") ),
      mu_d( new QLabel(":") ),
      dmdisplay( new QLabel("#Mosqs:") ),
      fromButton( new QPushButton ("[") ),
      toButton( new QPushButton ("]") ),
      dmddisplay( new QLabel("#Trig:") ),
      mddisplay( new QLabel("") ) 
{
    metaMode=false;
    OpenCVViewer[0][0]=new Viewer(this);
    vw=1;
    vh=1;
    timerID=-1;
    for(int n=0; n<4; n++)
     for(int m=0; m<4; m++)
      have_positions[n][m]=false;
    for(int n=0; n<4; n++)
     for(int m=0; m<4; m++)
      have_tracks[n][m]=false;
    for(int n=0; n<4; n++)
     for(int m=0; m<4; m++)
      isOpen[n][m]=false;
    pslider->setObjectName(QStringLiteral("pslider"));
    pslider->setOrientation(Qt::Horizontal);

    QWidget *pc1 = new QWidget;
    QWidget *pc2 = new QWidget;
    QWidget *pc3 = new QWidget;
    QVBoxLayout *vLayout = new QVBoxLayout();
    QHBoxLayout *h1Layout = new QHBoxLayout(pc1);
    QHBoxLayout *h2Layout = new QHBoxLayout(pc2);
    QHBoxLayout *h3Layout = new QHBoxLayout(pc3);
    h1Layout->addWidget(pslider);
    h1Layout->addWidget(fselect);
    h1Layout->addWidget(fskipselect);
    h2Layout->addWidget(firstButton);
    h2Layout->addWidget(prevButton);
    h2Layout->addWidget(playButton);
    h2Layout->addWidget(loopButton);
    h2Layout->addWidget(stopButton);
    h2Layout->addWidget(nextButton);
    h2Layout->addWidget(lastButton);
    h2Layout->addWidget(fromButton);
    h2Layout->addWidget(toButton);
    h2Layout->addStretch();
    h3Layout->addWidget(dmdisplay);
    h3Layout->addWidget(mdisplay);
    h3Layout->addWidget(dmddisplay);
    h3Layout->addWidget(mddisplay);
    h3Layout->addStretch();
    h3Layout->addWidget(tdisplay);
    h3Layout->addWidget(sm_d);
    h3Layout->addWidget(msdisplay);
    h3Layout->addWidget(mu_d);
    h3Layout->addWidget(usdisplay);
    QFontMetrics fm(toButton->fontMetrics());;
    int fontHigh = fm.height();
    cerr<< "Font hight= "<<fontHigh<<"\n";
    QSize tmp=stopButton->minimumSizeHint();
    cerr<< "button hight= "<<tmp.height()<<"\n";
    QRect rec = QApplication::desktop()->screenGeometry();
    int sheight = rec.height();
    int swidth = rec.width();
    cerr<<"Screen Size: "<<sheight<<"\n"; 
    int rowHeight=tmp.height();
    if(2*fontHigh>rowHeight) rowHeight=2*fontHigh+1;
    if(sheight/32>rowHeight) rowHeight=sheight/32;
    pc1->setFixedHeight(rowHeight);
    pc2->setFixedHeight(rowHeight);
    pc3->setFixedHeight(rowHeight);
    
    ocvv = new QGridLayout();
    ocvv->setSpacing(0);
    ocvv->addWidget(OpenCVViewer[0][0],0,0); 
    vLayout->addLayout(ocvv);
    vLayout->addWidget(pc1);
    vLayout->addWidget(pc2);
    vLayout->addWidget(pc3);
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->setContentsMargins(0,0,0,0);
    h1Layout->setSpacing(0);
    h1Layout->setMargin(0);
    h1Layout->setContentsMargins(0,0,0,0);
    h2Layout->setSpacing(0);
    h2Layout->setMargin(0);
    h2Layout->setContentsMargins(0,0,0,0);
    h3Layout->setSpacing(0);
    h3Layout->setMargin(0);
    h3Layout->setContentsMargins(0,0,0,0);
    setWindowTitle( "VideoPlayer" );
    setLayout( vLayout);
    resize(1024,786);
    connect(firstButton,SIGNAL (clicked()),this,SLOT(on_FirstButton_clicked()));
    connect(lastButton,SIGNAL (clicked()),this,SLOT(on_LastButton_clicked()));
    connect(prevButton,SIGNAL (clicked()),this,SLOT(on_PrevButton_clicked()));
    connect(nextButton,SIGNAL (clicked()),this,SLOT(on_NextButton_clicked()));
    connect(playButton,SIGNAL (clicked()),this,SLOT(on_PlayButton_clicked()));
    connect(loopButton,SIGNAL (clicked()),this,SLOT(on_LoopButton_clicked()));
    connect(stopButton,SIGNAL (clicked()),this,SLOT(on_StopButton_clicked()));
    connect(firstButton,SIGNAL (clicked()),this,SLOT(on_FirstButton_clicked()));
    connect(toButton,SIGNAL (clicked()),this,SLOT(on_toButton_clicked()));
    connect(fromButton,SIGNAL (clicked()),this,SLOT(on_fromButton_clicked()));
    connect(pslider,SIGNAL(valueChanged(int)),this,SLOT(if_pslider_changed(int)));
    connect(fselect,SIGNAL(valueChanged(int)),this,SLOT(if_fselect_changed(int)));
    connect(fskipselect,SIGNAL(valueChanged(int)),this,SLOT(if_fskipselect_changed(int)));
    trackL=100;
    minTrackL=5;
    fskipselect->setMinimum(0);
    fskipselect->setMaximum(65536);
    fskipselect->setValue(0);
    pskip=0;
    syncByMeta=0;
    forceLoad=0;
    for(int i=0; i<4; i++)
     for(int j=0; j<4; j++)
    {
     seq[i][j]=NULL;
     MosqInSeq[i][j]=NULL;
     currReg[i][j]=NULL;
    }
    loopMode=false;
    fromFrame=0;
    toFrame=0;
    this->setFocusPolicy(Qt::StrongFocus);
}

void VideoPlayerWidget::on_FirstButton_clicked()
{
if(isOpen[0][0])
{
    frame[0][0]=0;
    if(metaMode)
     trigger=metaMin();
    if(!metaMode)
    {
     fselect->setValue((int)frame[0][0]);
     pslider->setValue((int)frame[0][0]);
    }
    else
    {
     fselect->setValue(trigger);
     pslider->setValue(trigger);
    }
    showFrame(0, 0);
    for(int c=0;c<vw;c++)
     for(int r=0;r<vh; r++)
      if( c+r>0 && isOpen[c][r])
    {
        frame[c][r]=0;
        showFrame(c,r); 
    }
}
}

void VideoPlayerWidget::on_LastButton_clicked()
{
if(isOpen[0][0])
{
    frame[0][0]=seq[0][0]->allocatedFrames()-1;
    fselect->blockSignals(true);
    pslider->blockSignals(true);
    if(!metaMode)
    {
    fselect->setValue((int)frame[0][0]);
    pslider->setValue((int)frame[0][0]);
    }
    else
    {
     trigger=metaMax();
     fselect->setValue(trigger);
     pslider->setValue(trigger);
    }
        fselect->blockSignals(false);
        pslider->blockSignals(false);
    showFrame(0,0);
    for(int c=0;c<vw;c++)
     for(int r=0;r<vh; r++)
      if(!(c==0 && r==0)&& isOpen[c][r])
    {
        frame[c][r]=seq[c][r]->allocatedFrames()-1;
        showFrame(c,r); 
    }
}
}

void VideoPlayerWidget::on_NextButton_clicked()
{
if(isOpen[0][0])
{
if(!metaMode)
{
    if(frame[0][0]+pskip<seq[0][0]->allocatedFrames()-1)
    {
        frame[0][0]++;
        frame[0][0]+=pskip;
        showFrame(0,0);
        fselect->blockSignals(true);
        pslider->blockSignals(true);
        fselect->setValue((int)frame[0][0]);
        pslider->setValue((int)frame[0][0]);
        fselect->blockSignals(false);
        pslider->blockSignals(false);
    }
}
else
{
    trigger=getNextTrigger();
    frame[0][0]=seq[0][0]->lookUpValidTrigger(trigger);
        showFrame(0,0);
    fselect->blockSignals(true);
    pslider->blockSignals(true);
    fselect->setValue(trigger);
    pslider->setValue(trigger);
    fselect->blockSignals(false);
    pslider->blockSignals(false);
}
    unsigned int t=frame[0][0];
    if(metaMode) t=trigger;
    else if(seq[0][0]->hasMeta())
        t=seq[0][0]->metaData(frame[0][0]);
    for(int c=0;c<vw;c++)
     for(int r=0;r<vh; r++)
      if(!(c==0 && r==0) && isOpen[c][r])
    {
        long tmp=0;
        if(seq[c][r]->hasMeta())
         tmp=seq[c][r]->lookUpTrigger(t);
        if(tmp<0)
         tmp=seq[c][r]->lookUpTrigger(t+tmp);
        if(tmp>0)
         frame[c][r]=tmp;
        else
         frame[c][r]=frame[0][0];
        cerr<<"For frame "<<frame[0][0]<<" show "<<frame[c][r]<< "on view ("<<c<<","<<r<<")\n";
        showFrame(c,r); 
    }
}
}

void VideoPlayerWidget::on_PrevButton_clicked()
{
if(isOpen[0][0])
{
 if(!metaMode)
 {
    if(frame[0][0]-pskip>0)
    {
        frame[0][0]--;
        frame[0][0]-=pskip;
        showFrame(0,0);
        fselect->blockSignals(true);
        pslider->blockSignals(true);
        fselect->setValue((int)frame[0][0]);
        pslider->setValue((int)frame[0][0]);
        fselect->blockSignals(false);
        pslider->blockSignals(false);
    }
 }
 else
 { 
    trigger=getPreviousTrigger();
    frame[0][0]=seq[0][0]->lookUpValidTrigger(trigger);
        showFrame(0,0);
    fselect->blockSignals(true);
    pslider->blockSignals(true);
    fselect->setValue(trigger);
    pslider->setValue(trigger);
    fselect->blockSignals(false);
    pslider->blockSignals(false); 
}
    unsigned int t=frame[0][0];
    if(metaMode) t=trigger;
    else if(seq[0][0]->hasMeta())
        t=seq[0][0]->metaData(frame[0][0]);
    for(int c=0;c<vw;c++)
     for(int r=0;r<vh; r++)
      if(!(c==0 && r==0)&& isOpen[c][r])
    {
        long tmp=0;
        if(seq[c][r]->hasMeta())
         tmp=seq[c][r]->lookUpTrigger(t);
        if(tmp<0)
         tmp=seq[c][r]->lookUpTrigger(t+tmp);
        if(tmp>0)
         frame[c][r]=tmp;
        else
         frame[c][r]=frame[0][0];
        showFrame(c,r); 
    }
}
}

void VideoPlayerWidget::on_PlayButton_clicked()
{
if(isOpen[0][0])
{
    if(pskip!=0)
    {
     originalEachN=seq[0][0]->getRequired();
     seq[0][0]->setRequired(1);
    }
    if(timerID==-1)
        timerID=startTimer(0);
}
}

void VideoPlayerWidget::on_LoopButton_clicked()
{
if(isOpen[0][0])
{
    if(pskip!=0)
    {
     originalEachN=seq[0][0]->getRequired();
     seq[0][0]->setRequired(1);
    }
    if(timerID==-1)
    {
        if(fromFrame<seq[0][0]->allocatedFrames())
         frame[0][0]=fromFrame;
        loopMode=true;
        timerID=startTimer(0);
    }
}
}

void VideoPlayerWidget::on_StopButton_clicked()
{
if(isOpen[0][0])
{
    if(timerID!=-1)
    {
      killTimer(timerID);
      timerID=-1;
      loopMode=false;
      if(pskip!=0)
      {
       seq[0][0]->setRequired(originalEachN);
      }
    }
}
}

void VideoPlayerWidget::on_fromButton_clicked()
{
if(isOpen[0][0]){
   if(!metaMode)
    emit set_from_value(frame[0][0]);
   else
    emit set_from_value(trigger);
   }
}

void VideoPlayerWidget::on_toButton_clicked()
{
  if(isOpen[0][0]){
   if(!metaMode)
    emit set_to_value(frame[0][0]);
   else
    emit set_to_value(trigger);
  }
}

void VideoPlayerWidget::timerEvent(QTimerEvent *event)
{
    if(frame[0][0]<seq[0][0]->allocatedFrames() && trigger!=metaMax())
    {
        if(loopMode && frame[0][0]>=toFrame)
        {
         if(fromFrame<seq[0][0]->allocatedFrames())
          frame[0][0]=fromFrame;
        }
        showFrame(0,0);
        fselect->blockSignals(true);
        pslider->blockSignals(true);
        if(!metaMode)
        {
        fselect->setValue((int)frame[0][0]);
        pslider->setValue((int)frame[0][0]);
        }
        else
        {
        fselect->setValue(trigger);
        pslider->setValue(trigger);
        }
        fselect->blockSignals(false);
        pslider->blockSignals(false);
        if(!metaMode)
        {
        frame[0][0]++;
        frame[0][0]+=pskip;
        }
        else
        {
        trigger=getNextTrigger();
        frame[0][0]=seq[0][0]->lookUpValidTrigger(trigger);
        }
        unsigned int t=frame[0][0];
        if(metaMode)t=trigger;
        else if(seq[0][0]->hasMeta())
           t=seq[0][0]->metaData(frame[0][0]);
        for(int c=0;c<vw;c++)
         for(int r=0;r<vh; r++)
          if(!(c==0 && r==0)&& isOpen[c][r])
        {
            long tmp=0;
            if(seq[c][r]->hasMeta())
             tmp=seq[c][r]->lookUpTrigger(t);
            if(tmp<0)
             tmp=seq[c][r]->lookUpTrigger(t+tmp);
            if(tmp>0)
             frame[c][r]=tmp;
            else
             frame[c][r]=frame[0][0];
            showFrame(c,r); 
        }
    }
    else
    {
        killTimer(event->timerId());
        timerID=-1;
        if(pskip!=0) 
         seq[0][0]->setRequired(originalEachN);
    }
}

void VideoPlayerWidget::if_pslider_changed(int value)
{
if(isOpen[0][0])
{
    if(metaMode)
    {
        trigger=value;
        value=seq[0][0]->lookUpValidTrigger(trigger);
    }
    if(frame[0][0]!=(unsigned)value)
    {
        frame[0][0]=value;
        fselect->blockSignals(true);
        pslider->blockSignals(true);
        if(!metaMode)
        {
         fselect->setValue((int)frame[0][0]);
         pslider->setValue((int)frame[0][0]);
        }
        else
        {
         fselect->setValue(trigger);
         pslider->setValue(trigger);
        }
        fselect->blockSignals(false);
        pslider->blockSignals(false);
        showFrame(0,0);        
    }
    unsigned int t=frame[0][0];
    if(metaMode) t=trigger;
    else if(seq[0][0]->hasMeta())
        t=seq[0][0]->metaData(frame[0][0]);
for(int c=0;c<vw;c++)
 for(int r=0;r<vh; r++)
  if(!(c==0 && r==0)&& isOpen[c][r])
{
        long tmp=0;
        if(seq[c][r]->hasMeta())
         tmp=seq[c][r]->lookUpTrigger(t);
        if(tmp<0)
         tmp=seq[c][r]->lookUpTrigger(t+tmp);
        if(tmp>0)
         frame[c][r]=tmp;
        else
         frame[c][r]=frame[0][0];
    showFrame(c,r); 
}
}
}

void VideoPlayerWidget::if_fselect_changed(int value)
{
if(isOpen[0][0])
{
    if(metaMode)
    {
        trigger=value;
        value=seq[0][0]->lookUpValidTrigger(trigger);
    }
    
    if(frame[0][0]!=(unsigned)value)
    {
        frame[0][0]=value;
        pslider->blockSignals(true);
        if(!metaMode)
         pslider->setValue((int)frame[0][0]);
        else
         pslider->setValue(trigger);
        pslider->blockSignals(false);
        showFrame(0,0);
    }
    unsigned int t=frame[0][0];
    if(metaMode) t=trigger;
    else if(seq[0][0]->hasMeta())
        trigger=seq[0][0]->metaData(frame[0][0]);
for(int c=0;c<vw;c++)
 for(int r=0;r<vh; r++)
  if(!(c==0 && r==0)&& isOpen[c][r])
{
    long tmp=0;
    if(seq[c][r]->hasMeta())
     tmp=seq[c][r]->lookUpTrigger(t);
    if(tmp<0)
     tmp=seq[c][r]->lookUpTrigger(t+tmp);
    if(tmp>0)
     frame[c][r]=tmp;
    else
     frame[c][r]=frame[0][0];
    showFrame(c,r); 
}
}
}

void VideoPlayerWidget::if_fskipselect_changed(int value)
{
pskip=value;
}

void VideoPlayerWidget::updateFrame(unsigned long value)
{
if(isOpen[0][0])
{
    if(metaMode)
    {
        trigger=value;
        value=seq[0][0]->lookUpValidTrigger(trigger);
    }
    if(value<seq[0][0]->allocatedFrames())
    {
        frame[0][0]=value;
        fselect->blockSignals(true);
        pslider->blockSignals(true);
        if(!metaMode)
        {
         fselect->setValue((int)frame[0][0]);
         pslider->setValue((int)frame[0][0]);
        }
        else
        {
         fselect->setValue(trigger);
         pslider->setValue(trigger);
        }
        fselect->blockSignals(false);
        pslider->blockSignals(false);
        showFrame(0,0);
    }
unsigned int t=frame[0][0];
if(metaMode) t=trigger;
else if(seq[0][0]->hasMeta())
    t=seq[0][0]->metaData(frame[0][0]);

for(int c=0;c<vw;c++)
 for(int r=0;r<vh; r++)
  if(!(c==0 && r==0)&& isOpen[c][r])
{
    long tmp=0;
    if(seq[c][r]->hasMeta())
     tmp=seq[c][r]->lookUpTrigger(t);
    if(tmp<0)
     tmp=seq[c][r]->lookUpTrigger(t+tmp);
    if(tmp>0)
     frame[c][r]=tmp;
    else
     frame[c][r]=frame[0][0];
    showFrame(c,r); 
}
}
}

void VideoPlayerWidget::ForceUpdateFrame(unsigned long value, bool forceImageReload /*=false*/)
{
 if(forceImageReload)
 {
  if(isOpen[0][0])
  {
   if(metaMode)
   {
    trigger=value;
    value=seq[0][0]->lookUpValidTrigger(trigger);
   }
   if(value<seq[0][0]->allocatedFrames())
   {
    frame[0][0]=value;
    fselect->blockSignals(true);
    pslider->blockSignals(true);
    if(!metaMode)
    {
     fselect->setValue((int)frame[0][0]);
     pslider->setValue((int)frame[0][0]);
    }
    else
    {
     fselect->setValue(trigger);
     pslider->setValue(trigger);
    }
    fselect->blockSignals(false);
    pslider->blockSignals(false);
    forceLoad=1;
    showFrame(0,0);
   }
   unsigned int t=frame[0][0];
   if(metaMode) t=trigger;
   else if(seq[0][0]->hasMeta())
   t=seq[0][0]->metaData(frame[0][0]);
   for(int c=0;c<vw;c++)
    for(int r=0;r<vh; r++)
     if(!(c==0 && r==0)&& isOpen[c][r])
   {
    forceLoad=1;
    long tmp=0;
    if(seq[c][r]->hasMeta())
     tmp=seq[c][r]->lookUpTrigger(t);
    if(tmp<0)
     tmp=seq[c][r]->lookUpTrigger(t+tmp);
    if(tmp>0)
     frame[c][r]=tmp;
    else
     frame[c][r]=frame[0][0];
    showFrame(c,r); 
   }
  }
 }
 else
  for(int c=0;c<vw;c++)
   for(int r=0;r<vh; r++)
    if(isOpen[c][r])
     OpenCVViewer[c][r]->reRenderImage();
}

void VideoPlayerWidget::LoadSeq(iSeq *iseq)
{
    seq[0][0]=iseq;
    isOpen[0][0]=true;
    unsigned long frames=iseq->allocatedFrames();
    if(metaMode){switchMeta();}
    on_FirstButton_clicked();
    have_positions[0][0]=false;
    have_tracks[0][0]=false;
    
    pslider->setMaximum(frames-1);
    fselect->setMaximum(frames-1);
    if(seq[0][0])
    {
     Rect* tmp=seq[0][0]->getRoi();
     if(tmp)
     {
      if(tmp->width!=0 && tmp->height!=0)
      {
      OpenCVViewer[0][0]->setROI(tmp->x,tmp->y,tmp->x+tmp->width,tmp->y+tmp->height);
      }
      else
      {
       OpenCVViewer[0][0]->clearROI();}
      }
     else
      OpenCVViewer[0][0]->clearROI();
    }
}

void VideoPlayerWidget::reLoadSeq(iSeq *iseq)
{
    seq[0][0]=iseq;
    isOpen[0][0]=true;
    unsigned long frames=iseq->allocatedFrames();
    if(metaMode){switchMeta();}
    pslider->setMaximum(frames-1);
    fselect->setMaximum(frames-1);
    frame[0][0]=iseq->currentFrame();
    updateFrame(frame[0][0]);
    have_positions[0][0]=false;
    have_tracks[0][0]=false;
    
    MosqInSeq[0][0]=NULL;
    currReg[0][0]=NULL;
    if(seq[0][0])
    {
     Rect* tmp=seq[0][0]->getRoi();
     if(tmp)
     {
      if(tmp->width!=0 && tmp->height!=0)
      {
      OpenCVViewer[0][0]->setROI(tmp->x,tmp->y,tmp->x+tmp->width,tmp->y+tmp->height);
      }
      else
      {
      OpenCVViewer[0][0]->clearROI();      
      }
     }  
     else
      OpenCVViewer[0][0]->clearROI();
    }
}

void VideoPlayerWidget::reLoadSeq(iSeq *iseq,int c, int r)
{
    seq[c][r]=iseq;
    isOpen[c][r]=true;
    MosqInSeq[c][r]=NULL;
    currReg[c][r]=NULL;
    have_positions[c][r]=false;
    have_tracks[c][r]=false;
    if(metaMode){switchMeta();switchMeta();}
    if(seq[c][r])
    {
     Rect* tmp=seq[c][r]->getRoi();
     if(tmp)
     {
     if(tmp->height!=0 && tmp->width!=0)
     {
      OpenCVViewer[c][r]->setROI(tmp->x,tmp->y,tmp->x+tmp->width,tmp->y+tmp->height);
     }
     else
     {
      OpenCVViewer[c][r]->clearROI();
     }
     }
     else
      OpenCVViewer[c][r]->clearROI();
    }
}

void VideoPlayerWidget::LoadMosqPos(MosquitoesInSeq *MosqPos)
{

    MosqInSeq[0][0]=MosqPos;
    have_positions[0][0]=true;
}

void VideoPlayerWidget::LoadMosqPos(MosquitoesInSeq *MosqPos,int c,int r)
{

    MosqInSeq[c][r]=MosqPos;
    have_positions[c][r]=true;
}

void VideoPlayerWidget::LoadReg(MosquitoRegistry *Reg)
{

    currReg[0][0]=Reg;
    have_tracks[0][0]=true;
}

void VideoPlayerWidget::LoadReg(MosquitoRegistry *Reg,int c,int r)
{
    currReg[c][r]=Reg;
    have_tracks[c][r]=true;
}

void VideoPlayerWidget::redraw()
{
if(isOpen[0][0])
    OpenCVViewer[0][0]->redraw();
for(int c=0;c<vw;c++)
 for(int r=0;r<vh; r++)
  if(!(c==0 && r==0)&& isOpen[c][r])
{
    OpenCVViewer[c][r]->redraw();
}
}

void VideoPlayerWidget::showImageWithPoints(unsigned long i, bool random /*=false*/, bool colorByFlag /*=false*/)
{
if(isOpen[0][0])
{
    int nFromRing=0;
    if(metaMode){
     if(i>=metaMin() && i<=metaMax())
      trigger=i;
     else
      cerr<<"Invalid trigger number\n";
     i=seq[0][0]->lookUpValidTrigger(trigger);
    }
    if(i<seq[0][0]->allocatedFrames())
    {
       nFromRing=seq[0][0]->currentFrame()-i;
       if(i<seq[0][0]->getRequired() ||(nFromRing<0 && nFromRing>seq[0][0]->getRequired()-1) || nFromRing<0)
       {
          seq[0][0]->loadImage(i);
          nFromRing=0;
       }
    }   
    if(have_positions[0][0])
    {
        if(have_tracks[0][0])
        {
            OpenCVViewer[0][0]->setMinTrackL(minTrackL);
            OpenCVViewer[0][0]->showImageWithPointsAndTracks(seq[0][0]->image(0,nFromRing),MosqInSeq[0][0]->getAllPos(i,metaMode),currReg[0][0]->getAllTracks(i,trackL,minTrackL),
                                                             MosqInSeq[0][0]->getPointMeta(i,metaMode),currReg[0][0]->getTrackMeta(i,trackL,minTrackL,random,colorByFlag),
                                                             MosqInSeq[0][0]->getContours(i,0,metaMode));
        }
        else
            OpenCVViewer[0][0]->showImageWithPoints(seq[0][0]->image(0,nFromRing),MosqInSeq[0][0]->getAllPos(i,metaMode),MosqInSeq[0][0]->getPointMeta(i,metaMode),
                                                    MosqInSeq[0][0]->getContours(i,0,metaMode));
        TSec=seq[0][0]->getImagePointer(nFromRing)->getTsec();
        TMSec=seq[0][0]->getImagePointer(nFromRing)->getTmsec();
        TUSec=seq[0][0]->getImagePointer(nFromRing)->getTusec();
        update_time();
        fselect->blockSignals(true);
        pslider->blockSignals(true);
        if(!metaMode)
        {
        fselect->setValue((int)i);
        pslider->setValue((int)i);
        }
        else
        {
        fselect->setValue((int)trigger);
        pslider->setValue((int)trigger);
        }
        fselect->blockSignals(false);
        pslider->blockSignals(false);
    }
    else
    {
        OpenCVViewer[0][0]->showImage(seq[0][0]->image(0,nFromRing) );
        TSec=seq[0][0]->getImagePointer(nFromRing)->getTsec();
        TMSec=seq[0][0]->getImagePointer(nFromRing)->getTmsec();
        TUSec=seq[0][0]->getImagePointer(nFromRing)->getTusec();
        update_time();
        fselect->blockSignals(true);
        pslider->blockSignals(true);
        if(!metaMode)
        {
        fselect->setValue((int)i);
        pslider->setValue((int)i);
        }
        else
        {
        fselect->setValue((int)trigger);
        pslider->setValue((int)trigger);
        }
        fselect->blockSignals(false);
        pslider->blockSignals(false);
    }

    unsigned int t=frame[0][0];
    if(metaMode) t=trigger;
    else if(seq[0][0]->hasMeta())
        t=seq[0][0]->metaData(frame[0][0]);

for(int c=0;c<vw;c++)
 for(int r=0;r<vh; r++)
  if(!(c==0 && r==0)&& isOpen[c][r])
{
if(isOpen[c][r])
{
    long tmp=0;
    if(seq[c][r]->hasMeta())
     tmp=seq[c][r]->lookUpTrigger(t);
    if(tmp<0)
     tmp=seq[c][r]->lookUpTrigger(t+tmp);
    if(tmp>0)
     frame[c][r]=tmp;
    else
     frame[c][r]=frame[0][0];
    int nFromRing=0;
    if(i<seq[c][r]->allocatedFrames())
    {
       nFromRing=seq[c][r]->currentFrame()-frame[c][r];
       if(i<seq[c][r]->getRequired() ||(nFromRing<0 && nFromRing>seq[c][r]->getRequired()-1))
       {
          seq[c][r]->loadImage(frame[c][r]);
          nFromRing=0;
       }
    }   
    if(have_positions[c][r])
    {
        if(have_tracks[c][r]) 
        {
            OpenCVViewer[c][r]->setMinTrackL(minTrackL);
            OpenCVViewer[c][r]->showImageWithPointsAndTracks(seq[c][r]->image(0,nFromRing),MosqInSeq[c][r]->getAllPos(frame[c][r],metaMode),currReg[c][r]->getAllTracks(frame[c][r],trackL,minTrackL),
                                                             MosqInSeq[c][r]->getPointMeta(i,metaMode),currReg[c][r]->getTrackMeta(frame[c][r],trackL,minTrackL,random,colorByFlag),
                                                             MosqInSeq[c][r]->getContours(frame[c][r],0,metaMode));
        }            
        else
            OpenCVViewer[c][r]->showImageWithPoints(seq[c][r]->image(0,nFromRing),MosqInSeq[c][r]->getAllPos(frame[c][r],metaMode),MosqInSeq[c][r]->getPointMeta(i,metaMode),
                                                    MosqInSeq[c][r]->getContours(frame[c][r],0,metaMode));
    }
    else
    {
        OpenCVViewer[c][r]->showImage(seq[c][r]->image(0,nFromRing) );
    }
}
}
}
}

void VideoPlayerWidget::drawAllPoints(int c, int r, int i, int j, bool random /*=false*/, bool colorByFlag /*=false*/)
{
 if(c==-1 && r==-1)
 {
  for(int ii=0;ii<vw;ii++)
   for(int jj=0;jj<vh; jj++)
    if(isOpen[ii][jj])
    {
        OpenCVViewer[ii][jj]->setPoints(MosqInSeq[ii][jj]->getAllPos(i,(unsigned long)j,metaMode),MosqInSeq[ii][jj]->getPointMeta(i,(unsigned long)j,metaMode),MosqInSeq[ii][jj]->getContours(i,j,metaMode));
        if(currReg[ii][jj]) OpenCVViewer[ii][jj]->setTracks(currReg[ii][jj]->getAllTracksInInterval(i,j),currReg[ii][jj]->getTrackMetaInInterval(i,j,0,random,colorByFlag));    
    }
 }
 else
 {
    if(isOpen[c][r])
    {
        OpenCVViewer[c][r]->setPoints(MosqInSeq[c][r]->getAllPos(i,(unsigned long)j,metaMode),MosqInSeq[c][r]->getPointMeta(i,(unsigned long)j,metaMode),MosqInSeq[c][r]->getContours(i,j,metaMode));
        if(currReg[c][r]) OpenCVViewer[c][r]->setTracks(currReg[c][r]->getAllTracksInInterval(i,j),currReg[c][r]->getTrackMetaInInterval(i,j,0,random,colorByFlag));
    }
 }
}

void VideoPlayerWidget::drawSelectedTracks(int c, int r, vector<int> TrackIDs, bool random /*=false*/, bool colorByFlag /*=false*/)
{
  if(isOpen[c][r])
  {
    if(currReg[c][r]) OpenCVViewer[c][r]->setTracks(currReg[c][r]->getSelectedTracks(TrackIDs),currReg[c][r]->getSelectedTracksMeta(TrackIDs, 0, random, colorByFlag));
  }
}

void VideoPlayerWidget::clearAllPoints(int c, int r)
{
 if(c==-1 && r==-1)
 {
  for(int ii=0;ii<vw;ii++)
   for(int jj=0;jj<vh; jj++)
    if(isOpen[ii][jj])
    {
        OpenCVViewer[ii][jj]->setPoints(std::vector<cv::Point2f>());
        if(currReg[ii][jj]) OpenCVViewer[ii][jj]->setTracks(std::vector<std::vector<cv::Point2f> >());    
    }
 }
 else
 {
    if(isOpen[c][r])
    {
        OpenCVViewer[c][r]->setPoints(std::vector<cv::Point2f>());
        if(currReg[c][r]) OpenCVViewer[c][r]->setTracks(std::vector<std::vector<cv::Point2f> >());
    }
 }
}

void VideoPlayerWidget::set_mdisplay(int mosqs)
{
if(isOpen[0][0])
{
    mdisplay->setNum(mosqs);
}
}

void VideoPlayerWidget::set_mddisplay(int num)
{
if(isOpen[0][0])
{
    dmddisplay->setText("#Trig:");
    mddisplay->setNum(num);
}
}

void VideoPlayerWidget::hide_mddisplay()
{
if(isOpen[0][0])
{
    mddisplay->clear();
    dmddisplay->clear();
}
}

void VideoPlayerWidget::set_usdisplay(short us)
{
if(isOpen[0][0])
{
    QString number = QString("%1").arg(us, 3, 10, QChar('0'));
    usdisplay->setText(number);
}
}

void VideoPlayerWidget::set_msdisplay(short ms)
{
if(isOpen[0][0])
{
    QString number = QString("%1").arg(ms, 3, 10, QChar('0'));
    msdisplay->setText(number);
}
}

void VideoPlayerWidget::set_tdisplay(int i)
{
if(isOpen[0][0])
{
    time_t rawtime=i;
    struct tm * ptm;
    ptm = gmtime ( &rawtime );
    char buffer [80];
    strftime (buffer,80,"%A, %B %d, %Y %H:%M:%S",ptm );
    tdisplay->setText(QString(buffer));
}
}

void VideoPlayerWidget::set_trackDisplayLength(int i)
{
    trackL=i;
}

void VideoPlayerWidget::set_MinTrackDisplayLength(int i)
{
    minTrackL=i;    
}

void VideoPlayerWidget::update_time()
{
if(isOpen[0][0])
{
    set_tdisplay(TSec);
    set_msdisplay(TMSec);
    set_usdisplay(TUSec);
    if(have_positions[0][0])
        set_mdisplay(MosqInSeq[0][0]->MosqsInFrame(frame[0][0]));
    if(seq[0][0]->hasMeta())
        set_mddisplay(seq[0][0]->metaData(frame[0][0]));
    else
        hide_mddisplay();
}
}

unsigned long VideoPlayerWidget::getCurrentFrameNo()
{
    if(!metaMode)
     return frame[0][0];
    else
     return trigger;
}

void VideoPlayerWidget::setCurrentFrameNo(unsigned long i)
{
if(isOpen[0][0])
    updateFrame(i);
}

void VideoPlayerWidget::savePGM(string filename)
{
if(vh+vw>2)
{
ImageComposer composer;
composer.setC(vw);
composer.setR(vh);
for(int i=0; i<vw;i++)
 for(int j=0; j<vh;j++)
  if(isOpen[i][j])
   composer.setSeq(seq[i][j],i,j);
   
int trigger=seq[0][0]->metaData(frame[0][0]);
Image<unsigned char>* tmpi=composer.getImage(trigger);
tmpi->save(filename);
}
else{
if(isOpen[0][0])
    seq[0][0]->getImagePointer(0)->save(filename);
}
}

void VideoPlayerWidget::setHavePositions()
{
if(isOpen[0][0])
    have_positions[0][0]=true;
}

void VideoPlayerWidget::setHaveTracks()
{
if(isOpen[0][0])
    have_tracks[0][0]=true;
}

void VideoPlayerWidget::setHaveNoPositions()
{
if(isOpen[0][0])
    have_positions[0][0]=false;
}

void VideoPlayerWidget::setHaveNoTracks()
{
if(isOpen[0][0])
    have_tracks[0][0]=false;
}

void VideoPlayerWidget::removePointClicked(double x, double y)
{
if(isOpen[0][0])
   emit removePoint(x,y,0,0);
}

void VideoPlayerWidget::addPointClicked(double x, double y)
{
if(isOpen[0][0])
   emit addPoint(x,y);
}

Viewer::DisplaySettings VideoPlayerWidget::getDisplaySettings()
{
    Viewer::DisplaySettings dset=OpenCVViewer[0][0]->getDisplaySettings();
    return dset;
}

bool  VideoPlayerWidget::setDisplaySettings(Viewer::DisplaySettings set)
{
    OpenCVViewer[0][0]->setDisplaySettings(set);
    for(int c=0;c<vw;c++)
     for(int r=0;r<vh; r++)
      OpenCVViewer[c][r]->setDisplaySettings(set);

    return true;
}

void VideoPlayerWidget::setColumns(int x)
{
    if(x>0 &&x<=4)
    {
        if(x>vw)
        {
        for(int xx=vw;xx<x;xx++)
        {
            
            for(int j=0; j<vh; j++)
            {
               OpenCVViewer[xx][j]=new Viewer(this);
               OpenCVViewer[xx][j]->setColRow(xx,j);
               ocvv->addWidget(OpenCVViewer[xx][j],j,xx); 
               OpenCVViewer[xx][j]->setDisplaySettings(OpenCVViewer[0][0]->getDisplaySettings());
            }
        }
        }
        if(x<vw)
        {
        for(int xx=vw-1;xx>=x;xx--)
        {
            for(int j=0; j<vh; j++)
            {
               ocvv->removeWidget(OpenCVViewer[xx][j]);
               delete OpenCVViewer[xx][j];
            }
        }
        }
        vw=x;
    }
}

void VideoPlayerWidget::setRows(int y)
{
    if(y>0 &&y<=4)
    {
        if(y>vh)
        {
        for(int yy=vh;yy<y;yy++)
        {
            for(int j=0; j<vw; j++)
            {
               OpenCVViewer[j][yy]=new Viewer(this);
               OpenCVViewer[j][yy]->setColRow(j,yy);
               ocvv->addWidget(OpenCVViewer[j][yy],yy,j); 
               OpenCVViewer[j][yy]->setDisplaySettings(OpenCVViewer[0][0]->getDisplaySettings());
            }
        }
        }
        if(y<vh)
        {
        for(int yy=vh-1;yy>=y;yy--)
        {
            for(int j=0; j<vw; j++)
            {
               ocvv->removeWidget(OpenCVViewer[j][yy]);
               delete OpenCVViewer[j][yy];
            }
        }
        }
        vh=y;
    }
}

int VideoPlayerWidget::getColumns()
{
    return vw;
}

int VideoPlayerWidget::getRows()
{
    return vh;
}

void VideoPlayerWidget::mousePressEvent(QMouseEvent *event)
{
    m = event->pos();
    mm = event->pos();
    Viewer *vidPointer = dynamic_cast<Viewer*>(childAt(m.x(),m.y()));
    if (vidPointer)
    {
      int c=vidPointer->getCol();
      int r=vidPointer->getRow();
      if(event->button()==Qt::RightButton)
          emit contextMenu(event->globalPos(),c,r);  
      if(event->button()==Qt::LeftButton)
          emit mouseClick(event->globalPos(),c,r);  
    }
}


void VideoPlayerWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m.x();
    int dy = event->y() - m.y();
    int ddx = event->x() - mm.x();
    int ddy = event->y() - mm.y();
    QPoint n = event->pos();    
    Viewer *vidPointer = dynamic_cast<Viewer*>(childAt(n.x(),n.y()));
    if (vidPointer)
    {
      int c=vidPointer->getCol();
      int r=vidPointer->getRow();
      if (event->buttons() & Qt::LeftButton) {
        emit mouseMove(event->globalPos(),c,r,ddx,ddy,dx,dy);
      }
    }
    mm = event->pos();
}

void VideoPlayerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint n = event->pos();
    int dx=n.x()-m.x();
    int dy=n.y()-m.y();
    Viewer *vidPointer = dynamic_cast<Viewer*>(childAt(n.x(),n.y()));
    if (vidPointer)
    {
      int c=vidPointer->getCol();
      int r=vidPointer->getRow();
      if(event->button()==Qt::LeftButton)
          emit mouseRelease(event->globalPos(),c,r,dx,dy); 
    }
}

void VideoPlayerWidget::wheelEvent(QWheelEvent* event)
{
    double z=0.2*(event->delta()/120);
    cerr<<"zoom factor: "<<z<<"\n";
    QPoint n = event->pos();
    Viewer *vidPointer = dynamic_cast<Viewer*>(childAt(n.x(),n.y()));
    if (vidPointer)
    {
      int c=vidPointer->getCol();
      int r=vidPointer->getRow();
      QPointF p=imageCoordinates(event->globalPos(),c,r);
      vidPointer->zoom(z,p);
    }
}

void VideoPlayerWidget::showFrame(int c, int r, bool random /*=false*/,bool colorByFlag /*=false*/)
{
        if(frame[c][r]<seq[c][r]->allocatedFrames())
        {
            int status=0;
            if(seq[c][r]->currentFrame()!=frame[c][r] || frame[c][r]==0 || frame[c][r]==seq[c][r]->allocatedFrames()-1 || forceLoad==1 || seq[c][r]->isImage() ){
                status=seq[c][r]->loadImage(frame[c][r]);
                forceLoad=0;
            }
            if(status==0)
            {
            if(have_positions[c][r])
            {
                if(have_tracks[c][r])
                {
                    OpenCVViewer[c][r]->setMinTrackL(minTrackL);
                    OpenCVViewer[c][r]->showImageWithPointsAndTracks(seq[c][r]->image(0,0),MosqInSeq[c][r]->getAllPos(frame[c][r],metaMode),currReg[c][r]->getAllTracks(frame[c][r],trackL,minTrackL),
                                                                     MosqInSeq[c][r]->getPointMeta(frame[c][r],metaMode),currReg[c][r]->getTrackMeta(frame[c][r],trackL,minTrackL,random,colorByFlag),MosqInSeq[c][r]->getContours(frame[c][r],0,metaMode));
                }
                else
                    OpenCVViewer[c][r]->showImageWithPoints(seq[c][r]->image(0,0),MosqInSeq[c][r]->getAllPos(frame[c][r],metaMode),MosqInSeq[c][r]->getPointMeta(frame[c][r],metaMode),MosqInSeq[c][r]->getContours(frame[c][r],0,metaMode));
                if(c==0 && r==0)
                {
                TSec=seq[c][r]->getImagePointer(0)->getTsec();
                TMSec=seq[c][r]->getImagePointer(0)->getTmsec();
                TUSec=seq[c][r]->getImagePointer(0)->getTusec();
                update_time();
                }
            }
            else
            {
                OpenCVViewer[c][r]->showImage(seq[c][r]->image(0,0) );
                if(c==0 && r==0)
                {
                TSec=seq[c][r]->getImagePointer(0)->getTsec();
                TMSec=seq[c][r]->getImagePointer(0)->getTmsec();
                TUSec=seq[c][r]->getImagePointer(0)->getTusec();
                update_time();
                }
            }
            }
            else
            {
                std::cerr<<"Error: Cannot load Frame"<<std::endl;
            }
        }
}

iSeq* VideoPlayerWidget::getSeqPointer(int c, int r)
{
   if(c>=0 && c<vw && r>=0 && r<vh)
    if(isOpen[c][r])
     return seq[c][r];
     
   return nullptr;
}

MosquitoesInSeq* VideoPlayerWidget::getMosqPointer(int c, int r)
{
   if(c>=0 && c<vw && r>=0 && r<vh)
    if(isOpen[c][r] && have_positions[c][r])
     return MosqInSeq[c][r];
     
   return nullptr;
}

MosquitoRegistry* VideoPlayerWidget::getTrackPointer(int c, int r)
{
   if(c>=0 && c<vw && r>=0 && r<vh)
    if(isOpen[c][r] && have_tracks[c][r])
     return currReg[c][r];
     
   return nullptr;
}

int VideoPlayerWidget::switchMeta()
{
if(!metaMode)
{
if(seq[0][0]->hasMeta())
{
    pslider->setMaximum(metaMax());
    fselect->setMaximum(metaMax());
    pslider->setMinimum(metaMin());
    fselect->setMinimum(metaMin());
    trigger=seq[0][0]->metaData(frame[0][0]);
    metaMode=!metaMode;
}
}
else{
    pslider->setMaximum(seq[0][0]->allocatedFrames()-1);
    fselect->setMaximum(seq[0][0]->allocatedFrames()-1);
    pslider->setMinimum(0);
    fselect->setMinimum(0);
    trigger=seq[0][0]->metaData(frame[0][0]);
    metaMode=!metaMode;
}
 return 0;
}


bool VideoPlayerWidget::haveImage(int t)
{
 bool haveOne=false;
 for(int i=0; i<vw; i++)
  for(int j=0; j<vh; j++)
   if(isOpen[i][j])
 {
   int n=seq[i][j]->lookUpTrigger(t);
   if(n>0){ haveOne=true;break;}
   if(have_positions[i][j])
    if(MosqInSeq[i][j]->isTriggerBased())
     if(MosqInSeq[i][j]->dataForTrigger(t))
     {haveOne=true;break;}
 }
 return haveOne;
}

int VideoPlayerWidget::metaMin()
{
 int mMin=-1;
 for(int i=0; i<vw; i++)
  for(int j=0; j<vh; j++)
   if(isOpen[i][j] && seq[i][j])
 {
   int n=seq[i][j]->metaData(0);
   if(n<mMin || mMin==-1 ){ mMin=n;}
   if(have_positions[i][j] && MosqInSeq[i][j])
   {
    if(MosqInSeq[i][j]->isTriggerBased())
    {
     vector<unsigned long> range=MosqInSeq[i][j]->getTriggerRange();
     if(range.size()==2)
     {
      if(range[0]<mMin){ mMin=range[0];}
     }
    }
   }  
  }
 return mMin;
}

int VideoPlayerWidget::metaMax()
{
 int mMax=-1;
 for(int i=0; i<vw; i++)
  for(int j=0; j<vh; j++)
   if(isOpen[i][j] && seq[i][j])
 {
   int n=seq[i][j]->metaData(seq[i][j]->allocatedFrames()-1);
   if(n>mMax){ mMax=n;}
   if(have_positions[i][j] && MosqInSeq[i][j])
   {
    if(MosqInSeq[i][j]->isTriggerBased())
    {
     vector<unsigned long> range=MosqInSeq[i][j]->getTriggerRange();
     if(range.size()==2)
     {
      if(range[1]>mMax){ mMax=range[1];}
     }
    }
   }  
 }
 return mMax;
}

int VideoPlayerWidget::getNextTrigger(){
 int nextTrigger=trigger+1+pskip;
 bool foundOne=false;
 while(foundOne==false && nextTrigger<=metaMax())
 {
  if(haveImage(nextTrigger))
  {foundOne=true;}
  else
  {
  nextTrigger++;
  }
 }
 return nextTrigger; 
}

int VideoPlayerWidget::getPreviousTrigger(){
 int prevTrigger=trigger-1-pskip;
 bool foundOne=false;
 while(foundOne==false && prevTrigger>=metaMin())
 {
  if(haveImage(prevTrigger))
  {foundOne=true;}
  else
  {
  prevTrigger--;
  }
 }
 return prevTrigger; 
}

bool VideoPlayerWidget::useMetaMode()
{
  return metaMode;
}

void VideoPlayerWidget::activateMetaMode(bool b)
{
  if(b!=metaMode)
   switchMeta();
}

void VideoPlayerWidget::setZoom(QPoint m, int c, int r, int dx, int dy)
{
  QPointF x=OpenCVViewer[c][r]->posToImgCoords(m);
  QPoint n;
  n.setX(m.x()-dx);
  n.setY(m.y()-dy);
  QPointF y=OpenCVViewer[c][r]->posToImgCoords(n);
  if(x.x()<y.x() && x.y()<y.y())
   OpenCVViewer[c][r]->setCrop((int)x.x(),(int)x.y(),(int)(y.x()-x.x()),(int)(y.y()-x.y()));
  else if(x.x()>=y.x() && x.y()<y.y())
   OpenCVViewer[c][r]->setCrop((int)y.x(),(int)x.y(),(int)(x.x()-y.x()),(int)(y.y()-x.y()));
  else if(x.x()<y.x() && x.y()>=y.y())
   OpenCVViewer[c][r]->setCrop((int)x.x(),(int)y.y(),(int)(y.x()-x.x()),(int)(x.y()-y.y()));
  else if(x.x()>=y.x() && x.y()>=y.y())
   OpenCVViewer[c][r]->setCrop((int)y.x(),(int)y.y(),(int)(x.x()-y.x()),(int)(x.y()-y.y()));
  OpenCVViewer[c][r]->clearSelection();
}

void VideoPlayerWidget::setZoom(int c, int r, int ix, int iy, int iw, int ih)
{
   OpenCVViewer[c][r]->setCrop(ix,iy,iw,ih);
}

void VideoPlayerWidget::moveZoom(QPoint m, int c, int r, int dx, int dy)
{
  QPointF x=OpenCVViewer[c][r]->posToImgCoords(m);
  QPoint n;
  n.setX(m.x()-dx);
  n.setY(m.y()-dy);
  QPointF y=OpenCVViewer[c][r]->posToImgCoords(n);
  OpenCVViewer[c][r]->moveCrop((int)(y.x()-x.x()),(int)(y.y()-x.y()));
}

void VideoPlayerWidget::addMosquito(QPoint m, int c, int r)
{
  if(have_positions[c][r] && isOpen[c][r])
  {
   QPointF x=OpenCVViewer[c][r]->posToImgCoords(m);
   MosquitoesInFrame* currentFrame=MosqInSeq[c][r]->mosqInFrameI(frame[c][r],false,metaMode);
   currentFrame->addMosquito(x.x(), x.y(), 1.0, 1.0);
   showFrame(c,r);
  }
}

void VideoPlayerWidget::removeMosquito(QPoint m, int c, int r, unsigned long nn /*=0*/, unsigned long mm /*=0*/, double searchRadius /*=8*/, bool onlyOne /*=false*/ )
{
  if(have_positions[c][r] && isOpen[c][r])
  {
   QPointF x=OpenCVViewer[c][r]->posToImgCoords(m);
   if(mm>nn)
   {
    if((nn<seq[c][r]->allocatedFrames()  && mm<seq[c][r]->allocatedFrames())|| (metaMode && (nn>=metaMin() && nn<metaMax() && mm>=metaMin() && mm<metaMax()) ) )
    {    
     MosqInSeq[c][r]->removePoints(x.x(), x.y(), nn, mm ,metaMode, searchRadius, onlyOne);
     drawAllPoints(-1, -1, nn, mm);
    }
    else
    {
     cerr<<"Error: Range not valid!\n";
    }
   }
   else
   {
   MosquitoesInFrame* currentFrame=MosqInSeq[c][r]->mosqInFrameI(frame[c][r],false,metaMode);
   currentFrame->removeMosquito(x.x(), x.y(),searchRadius,onlyOne);
   showFrame(c,r);
   }
  }
}

void VideoPlayerWidget::addToPolygon(QPoint m, int c, int r)
{
  if(isOpen[c][r])
  {
   QPointF x=OpenCVViewer[c][r]->posToImgCoords(m);
   OpenCVViewer[c][r]->addToPolygon(x.x(),x.y());
  }
}

void VideoPlayerWidget::addToPolygon(double x, double y, int c, int r)
{
  if(isOpen[c][r])
  {
   OpenCVViewer[c][r]->addToPolygon(x,y);
  }
}

void VideoPlayerWidget::clearPolygon(int c, int r)
{
  if(isOpen[c][r])
  {
   OpenCVViewer[c][r]->clearPolygon();
  }
}

void VideoPlayerWidget::setSelection(QPoint m, int c, int r, int dx, int dy)
{
  QPointF x=OpenCVViewer[c][r]->posToImgCoords(m);
  QPoint n;
  n.setX(m.x()-dx);
  n.setY(m.y()-dy);
  QPointF y=OpenCVViewer[c][r]->posToImgCoords(n);
  OpenCVViewer[c][r]->setSelection(x.x(),x.y(),y.x(),y.y());
}

void VideoPlayerWidget::drawLine(QPoint m, int c, int r, int dx, int dy)
{
  QPointF x=OpenCVViewer[c][r]->posToImgCoords(m);
  QPoint n;
  n.setX(m.x()-dx);
  n.setY(m.y()-dy);
  QPointF y=OpenCVViewer[c][r]->posToImgCoords(n);
  OpenCVViewer[c][r]->setLine(x.x(),x.y(),y.x(),y.y());
}

void VideoPlayerWidget::setCursorForWidgets(int n)
{   
 for(int i=0; i<vw; i++)
  for(int j=0; j<vh; j++)
 {
   if(n==0)
       OpenCVViewer[i][j]->setCursor(Qt::ArrowCursor);
   else if(n==1)
{
       QPixmap cursor_pixmap = QPixmap(":cursors/Pointer.png");
       QCursor cursor_default = QCursor(cursor_pixmap, 15, 24);
       OpenCVViewer[i][j]->setCursor(cursor_default);
}
   else if(n==2)
       OpenCVViewer[i][j]->setCursor(Qt::OpenHandCursor);
   else if(n==3)
       OpenCVViewer[i][j]->setCursor(Qt::ClosedHandCursor);
   else if(n==4)
       OpenCVViewer[i][j]->setCursor(Qt::PointingHandCursor);
   else if(n==5)
   {
       QPixmap cursor_pixmap = QPixmap(":cursors/cursor_zoom_32x32.png");
       QCursor cursor_default = QCursor(cursor_pixmap, 9, 10);
       OpenCVViewer[i][j]->setCursor(cursor_default);
   }
 }
}

void VideoPlayerWidget::showImage(Mat A, unsigned long i, int c, int r)
{
if(isOpen[c][r])
{
    if(i<seq[c][r]->allocatedFrames())
    {
       OpenCVViewer[c][r]->showImage(A);
       OpenCVViewer[c][r]->redraw();
       vector<int> t=seq[c][r]->timeOfFrameI(i);
       if(t.size()==3){
       TSec=t[0];
       TMSec=t[1];
       TUSec=t[2];
       update_time();
       }
       fselect->blockSignals(true);
       pslider->blockSignals(true);
       fselect->setValue((int)i);
       pslider->setValue((int)i);
       fselect->blockSignals(false);
       pslider->blockSignals(false);
    }
}
}

QPointF VideoPlayerWidget::imageCoordinates(QPoint m, int c, int r)
{
  QPointF x=OpenCVViewer[c][r]->posToImgCoords(m);
  return x;
}

void VideoPlayerWidget::keyPressEvent(QKeyEvent *event)
{
if(event->key() == Qt::Key_Left){
on_PrevButton_clicked();
} 
else if(event->key() == Qt::Key_Right)
{
on_NextButton_clicked();
}
else if(event->key() == Qt::Key_Space)
{
 if(timerID==-1)
  on_PlayButton_clicked();
 else
  on_StopButton_clicked();
} 
else if(event->key() == Qt::Key_Up)
{
unsigned long opskip=pskip;
pskip=99;
on_PrevButton_clicked();
pskip=opskip;
}
else if(event->key() == Qt::Key_Down)
{
unsigned long opskip=pskip;
pskip=99;
on_NextButton_clicked();
pskip=opskip;
}
else if(event->key() == Qt::Key_BracketLeft)
{
 on_fromButton_clicked();
}
else if(event->key() == Qt::Key_BracketRight)
{
 on_toButton_clicked();
}
else
{
emit EmitKeyPressEvent(event);  
}
}

bool VideoPlayerWidget::setHighlight(double px, double py)
{
  bool val=OpenCVViewer[0][0]->setHighlight(px,py);
  return val;
}

bool VideoPlayerWidget::setTrackToHighlight(vector<cv::Point2f> cvtrack)
{
  bool val=OpenCVViewer[0][0]->setTrackToHighlight(cvtrack);
  return val;
}

bool VideoPlayerWidget::setHighlight(double px, double py, int c, int r)
{
  if(isOpen[c][r])
  {
  bool val=OpenCVViewer[c][r]->setHighlight(px,py);
  return val;
  }
  return false;
}

bool VideoPlayerWidget::setTrackToHighlight(vector<cv::Point2f> cvtrack, int c, int r)
{
  if(isOpen[c][r])
  {
  bool val=OpenCVViewer[c][r]->setTrackToHighlight(cvtrack);
  return val;
  }
  return false;
}

void VideoPlayerWidget::setROI(QPoint m, int c, int r, int dx, int dy)
{
  QPointF x=OpenCVViewer[c][r]->posToImgCoords(m);
  QPoint n;
  n.setX(m.x()-dx);
  n.setY(m.y()-dy);
  QPointF y=OpenCVViewer[c][r]->posToImgCoords(n);
  if(x.x()<y.x() && x.y()<y.y())
  {
   OpenCVViewer[c][r]->setROI((int)x.x(),(int)x.y(),(int)y.x(),(int)y.y());
   if(seq[c][r])
    seq[c][r]->setRoi((int)x.x(),(int)x.y(),(int)(y.x()-x.x()),(int)(y.y()-x.y()));
  }
  else if(x.x()>=y.x() && x.y()<y.y())
  {
   OpenCVViewer[c][r]->setROI((int)y.x(),(int)x.y(),(int)x.x(),(int)y.y());
   if(seq[c][r])
    seq[c][r]->setRoi((int)y.x(),(int)x.y(),(int)(x.x()-y.x()),(int)(y.y()-x.y()));
  }
  else if(x.x()<y.x() && x.y()>=y.y())
  {
   OpenCVViewer[c][r]->setROI((int)x.x(),(int)y.y(),(int)y.x(),(int)x.y());
   if(seq[c][r])
    seq[c][r]->setRoi((int)x.x(),(int)y.y(),(int)(y.x()-x.x()),(int)(x.y()-y.y()));
  }
  else if(x.x()>=y.x() && x.y()>=y.y())
  {
   OpenCVViewer[c][r]->setROI((int)y.x(),(int)y.y(),(int)x.x(),(int)x.y());
   if(seq[c][r])
    seq[c][r]->setRoi((int)y.x(),(int)y.y(),(int)(x.x()-y.x()),(int)(x.y()-y.y()));
  }
  OpenCVViewer[c][r]->clearSelection();

}

void VideoPlayerWidget::setROI(Rect iRoi, int c, int r)
{
   if(seq[c][r])
    OpenCVViewer[c][r]->setROI(iRoi.x,iRoi.y,iRoi.width+iRoi.x,iRoi.height+iRoi.y);
}

void VideoPlayerWidget::clearROI( int c, int r)
{
   OpenCVViewer[c][r]->clearROI();
   OpenCVViewer[c][r]->reRenderImage();
}

bool VideoPlayerWidget::clearPolygons(int c/*=0*/, int r/*=0*/)
{
  OpenCVViewer[c][r]->removePolygons();
  OpenCVViewer[c][r]->reRenderImage();
  return true;
}

void VideoPlayerWidget::clearTrackToHighlight(int c/*=0*/, int r/*=0*/)
{
    OpenCVViewer[c][r]->clearTrackToHighlight();
    OpenCVViewer[c][r]->reRenderImage();
}

bool VideoPlayerWidget::findCurrSeq(iSeq* currSeq,int &c, int &r)
{
  bool found=false;
    for(int n=0; n<vw; n++)
     for(int m=0; m<vh; m++)
      if(currSeq==seq[n][m])
       {
         found=true;
         c=n;
         r=m;
         break;
       }
  return found;
}

void VideoPlayerWidget::setFromFrame(unsigned long n)
{
 fromFrame=n;
}

void VideoPlayerWidget::setToFrame(unsigned long n)
{
 toFrame=n;
}
