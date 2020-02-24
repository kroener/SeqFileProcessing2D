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
#include "DisplayAllPointsWidget.h"
#include "Viewer.h"
#include "ViewerWithMouse.h"
#include "iSeq.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoRegistry.hpp"
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QCheckBox>
#include <QLabel>
#include <QString>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <unistd.h>
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
using namespace std;
using namespace cv;

//template<> int Image<unsigned char>::haveWindow=0;
//template<> int Image<double>::haveWindow=0;
//template<> int Image<short>::haveWindow=0;

DisplayAllPointsWidget::DisplayAllPointsWidget()
    : OpenCVViewer( new ViewerWithMouse),
      saveImageButton( new QPushButton ("Save") ),
      updateButton( new QPushButton ("Redraw") ),
      radius( new QSpinBox(this) ),
      trackWidth( new QSpinBox(this) ),
      from( new QSpinBox(this) ),
      to( new QSpinBox(this) ),
      step( new QSpinBox(this) ),
      fontS( new QDoubleSpinBox(this) ),
      displayTracks( new QCheckBox(this) ),
      selectMode( new QLabel("Frame") )
{
    haveSeq=false;
    QWidget *rightW = new QWidget;
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("&SelectMode"),selectMode);
    formLayout->addRow(tr("&From"),from);
    formLayout->addRow(tr("&To"),to);
    formLayout->addRow(tr("&Step"),step);
    formLayout->addRow(tr("&Radius"),radius);
    formLayout->addRow(tr("&Track Width"),trackWidth);
    formLayout->addRow(tr("&FontScale"),fontS);
    formLayout->addRow(tr("&DisplayTracks"),displayTracks);
    QVBoxLayout *vLayout=new QVBoxLayout(rightW);
    vLayout->addLayout(formLayout);
    vLayout->addWidget(updateButton);
    vLayout->addWidget(saveImageButton);
    QHBoxLayout *hLayout=new QHBoxLayout();
    QSizePolicy spLeft(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spLeft.setHorizontalStretch(4);
    OpenCVViewer->setSizePolicy(spLeft);
    hLayout->addWidget(OpenCVViewer);
    QSizePolicy spRight(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spRight.setHorizontalStretch(1);
    rightW->setSizePolicy(spRight);
    hLayout->addWidget(rightW);    
    setWindowTitle( "Display all Points" );
    setLayout(hLayout);
    resize(1024,786);
    
    //need to connect buttons and co:
    connect(updateButton,SIGNAL (clicked()),this,SLOT(on_updateButton_clicked()));
    connect(saveImageButton,SIGNAL (clicked()),this,SLOT(on_saveImageButton_clicked()));
    
    //I will not connect to them, but only read there values, if update button is clicked.
    //connect(background,SIGNAL(valueChanged(int)),this,SLOT(if_background_changed(int)));
    //connect(from,SIGNAL(valueChanged(int)),this,SLOT(if_from_changed(int)));
    //connect(to,SIGNAL(valueChanged(int)),this,SLOT(if_to_changed(int)));
    //connect(radius,SIGNAL(valueChanged(int)),this,SLOT(if_radius_changed(int)));
    
    from->setMinimum(0);
    from->setMaximum(0);
    from->setValue(0);
    to->setMinimum(0);
    to->setMaximum(0);
    to->setValue(0);
    step->setMinimum(1);
    step->setMaximum(1);
    step->setValue(1);
    radius->setMinimum(1);
    radius->setMaximum(100);
    radius->setValue(1);
    fontS->setMinimum(0);
    fontS->setMaximum(5);
    fontS->setValue(0.7);
    fontS->setDecimals(3);
    fontS->setSingleStep(0.001);
    trackWidth->setMinimum(1);
    trackWidth->setMaximum(100);
    trackWidth->setValue(1);
    isOpen=false;
    nativeFD=false;
    useTrigger=false;
    minTrigger=0;
    maxTrigger=0;
    displayTracks->setChecked(true);
    canCalcTime=false;
    dt=1;
    t0=0; 
    haveTracks=false;
    Registry=nullptr;
    selectMode->setText("Frame");
}

int DisplayAllPointsWidget::initialize(MosquitoesInSeq* ms, Mat img, int fr, int t, int st, iSeq* currSeq,bool nFD, bool uT /* = false */, int minT /* = 0 */, int maxT /* = 0 */)
{
  haveTracks=false;
  Registry=nullptr;  
  useTrigger=uT;
  minTrigger=minT;
  maxTrigger=maxT;
  cerr<<"Trigger from "<< minT <<" to "<<maxT<<"\n";
  iImg=img;
  MosqInSeq=ms;
  seq=currSeq;
  haveSeq=true;
  step->setMaximum(MosqInSeq->getNoOfFrames()-2);
  step->setValue(st);
  if(useTrigger)
  {
   //from->setMinimum(minTrigger);
   from->setMinimum(0);
   long xxx=ms->getMaxTrigger();
   if(xxx>maxTrigger)
    from->setMaximum(xxx);
   else
    from->setMaximum(maxTrigger);
   selectMode->setText("Trigger");
  }
  else
  { 
   from->setMaximum(MosqInSeq->getNoOfFrames()-1);
   selectMode->setText("Frame");

  }
  from->setValue(fr);
  if(useTrigger)
  {
   to->setMinimum(0);
   long xxx=ms->getMaxTrigger();
   if(xxx>maxTrigger)
    to->setMaximum(xxx);
   else
    to->setMaximum(maxTrigger);
   cerr<<"resetting to range to: "<< minTrigger <<" -> "<<maxTrigger<<"\n";
  }
  else to->setMaximum(MosqInSeq->getNoOfFrames()-1);
  to->setValue(t);
  isOpen=true;
  nativeFD=nFD;
  //set time range from mosqinseq?
  if(!useTrigger)
  {
   from->setValue(MosqInSeq->getFirstFrameWithMosquito());
   to->setValue(MosqInSeq->getLastFrameWithMosquito()); 
  }
  else
  {
   from->setValue(MosqInSeq->getFirstFrameWithMosquito());
   to->setValue(MosqInSeq->getLastFrameWithMosquito()); 
  }
  updateImage();
  return 0;
}

void DisplayAllPointsWidget::giveRegistryPointer(MosquitoRegistry* iReg)
{
 if(iReg)
 {
  Registry=iReg;
  haveTracks=true;
 }
 else
 {
  Registry=nullptr;
  haveTracks=false;
 }
}

void DisplayAllPointsWidget::on_updateButton_clicked()
{
//get image from seq
//draw points with current settings
//display
updateImage();
}

void DisplayAllPointsWidget::on_saveImageButton_clicked()
{
//save drawn image
//i need to store it somewhere, or can the widget return it?
//open file dialog, save as
std::cout<<"Saving Image clicked ..."<<std::endl;
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, nativeFD);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("PNG image files (*.png)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    while (dialog.exec() == QDialog::Accepted && !saveImage(dialog.selectedFiles().first())) {}
}

bool DisplayAllPointsWidget::saveImage(const QString &fileName)
{
string filename=fileName.toUtf8().constData();
if(!( access( filename.c_str(), F_OK ) != -1 ))
{
imwrite( filename, oImg );
}
else
{
   QMessageBox msgBox;
   msgBox.setText(QString("Error: Image already exists, choose another file...\n"));
   msgBox.exec();
std::cout<<"Error: File exists!"<<std::endl;
}
return true;
}

void DisplayAllPointsWidget::updateImage()
{
///todo
if(isOpen)
{
   //get time <= for !useTrigger, how to get time from 
   int ts0=MosqInSeq->getTsSec((unsigned long) from->value(),useTrigger);
   int tm0=MosqInSeq->getTsMSec((unsigned long) from->value(),useTrigger);
   int tu0=MosqInSeq->getTsUSec((unsigned long) from->value(),useTrigger);
   int ts1=MosqInSeq->getTsSec((unsigned long) to->value(),useTrigger);
   int tm1=MosqInSeq->getTsMSec((unsigned long) to->value(),useTrigger);
   int tu1=MosqInSeq->getTsUSec((unsigned long) to->value(),useTrigger);
   if(!useTrigger)
    if(haveSeq){
     vector<int> tt0;
     vector<int> tt1;
     tt0=seq->timeOfFrameI((unsigned long) from->value());
     tt1=seq->timeOfFrameI((unsigned long) to->value());
     if(tt0.size()==3){
      ts0=tt0[0];
      tm0=tt0[1];
      tu0=tt0[2];
      cerr<<"T0: "<<tt0[0]<<"sec "<<tt0[1]<<"ms "<<tt0[1]<<"us\n";   
     }
     if(tt1.size()==3){
      ts1=tt1[0];
      tm1=tt1[1];
      tu1=tt1[2];
      cerr<<"T1: "<<tt1[0]<<"sec "<<tt1[1]<<"ms "<<tt1[1]<<"us\n";   
     }
    }
   double t0=double(ts0+tm0/1000.+tu0/1000000.);
   double t1=double(ts1+tm1/1000.+tu1/1000000.);
   double d=t1-t0;
   //iImg.copyTo(oImg);
   int top=0;
   int bottom=(int) (iImg.rows*0.05);
   int left=0;
   int right=(int) (iImg.cols*0.2);
   oImg=iImg;
   copyMakeBorder(iImg,oImg,top,bottom,left,right,BORDER_CONSTANT,Scalar(0,0,0));
   cvtColor(oImg, oImg, COLOR_GRAY2RGB);
   cerr<<"Draw Tracks...\n";
   if(displayTracks->isChecked() && haveTracks)
   {
     vector<vector<Point2f> > cvtracks=Registry->getAllTracks(to->value() , to->value()-from->value() ,2);
     vector<vector<Scalar> > colors=Registry->getTrackMetaForAni(to->value() , to->value()-from->value(), 2, 5, t0, t1);
     //test: draw all line segments with line
     for (unsigned int j=0; j<cvtracks.size(); j++)
     {
         for(unsigned int jj=0; jj<cvtracks[j].size()-1; jj++)
         {
              line( oImg, cvtracks[j][jj],  cvtracks[j][jj+1], colors[j][jj], trackWidth->value(), 8 );
         }
     }
   }
   cerr<<"Draw Points...\n";
   for(unsigned long i=from->value(); i<to->value(); i+=step->value())
   {
    vector<cv::Point2f> a;
    if(useTrigger)
    {
      a=MosqInSeq->getAllPos(i,true);
    }
    else
    {
      a=MosqInSeq->getAllPos(i);
    }
    if(a.size()>0)
    {
     //doing it right, time instead of frames:
     int ts2=MosqInSeq->getTsSec(i,useTrigger);
     int tm2=MosqInSeq->getTsMSec(i,useTrigger);
     int tu2=MosqInSeq->getTsUSec(i,useTrigger);
     //cerr<<"Got time("<<useTrigger<<"): "<< ts2<<"s "<<tm2<<"ms "<<tu2<<"us =>"<<ts0<<"s -> "<<ts1<<"s \n";
     if(haveSeq && !useTrigger){
      vector<int> tt2=seq->timeOfFrameI(i);
      if(tt2.size()==3){
       ts2=tt2[0];
       tm2=tt2[1];
       tu2=tt2[2];
      }
     }
     
     double t2=double(ts2+tm2/1000.+tu2/1000000.);
     double cx=(t2-t0)/(t1-t0);
     //double cx=double(i-from->value())/double(to->value()-from->value());
     Scalar ccolor=JetColorMap(cx);
     for(unsigned j=0; j<a.size(); j++)
     {
      circle(oImg,a[j],radius->value(),ccolor,-1,8);
     }
    }   
   }
   //annotate times:
   char buff[160];

   time_t rawtime=ts0;
   struct tm * ptm;
   ptm = gmtime ( &rawtime );
   char buffer [80];
   //strftime (buffer,80,"%D:%H:%M:%S",ptm);
   strftime (buffer,80,"%A, %B %d, %Y %H:%M:%S",ptm);
   snprintf(buff, sizeof(buff), "%s:%0d:%0d -> Duration: %lf s", buffer,tm0,tu1, d);
   std::string buffAsStdStr = buff;
   int fontFace = FONT_HERSHEY_SIMPLEX;
   //FONT_HERSHEY_SCRIPT_SIMPLEX;
   double fontScale = fontS->value();//0.7;
   int thickness = 1;
   int baseline=0;
   Size textSize = getTextSize(buffAsStdStr, fontFace,
                            fontScale, thickness, &baseline);
   baseline += thickness;
   
   Point textOrg((oImg.cols-textSize.width)/2, oImg.rows-16);
   putText(oImg, buffAsStdStr, textOrg, fontFace, fontScale, Scalar::all(255), thickness,8);

   //Add color bar:
   //where?

   int xc1=0.85*oImg.cols;
   int xc2=0.9*oImg.cols;
   int yc1=0.1*oImg.rows;
   int yc2=0.9*oImg.rows;
   int cbt=1;
   rectangle(oImg,Point(xc1-cbt,yc1-cbt),Point(xc2+cbt,yc2+cbt),Scalar::all(255),1);
   rectangle(oImg,Point(xc2,yc1-1),Point(xc2+8,yc1),Scalar::all(255),1);
   rectangle(oImg,Point(xc2,yc2),Point(xc2+8,yc2+1),Scalar::all(255),1);
   rectangle(oImg,Point(xc2,(yc2+yc1)/2.),Point(xc2+8,(yc2+yc1)/2.+1),Scalar::all(255),1);
   rectangle(oImg,Point(xc2,(yc2-yc1)/4.+yc1),Point(xc2+8,(yc2-yc1)/4.+yc1+1),Scalar::all(255),1);
   rectangle(oImg,Point(xc2,(yc2-yc1)/4.*3.+yc1),Point(xc2+8,(yc2-yc1)/4.*3.+yc1+1),Scalar::all(255),1);
   
   for(int j=yc1; j<yc2;j++)
   {
    double v=double(j-yc1)/double(yc2-yc1);
    Scalar color=JetColorMap(1.-v);
    rectangle(oImg,Point(xc1,j),Point(xc2,j+1),color,1);
   }
   //annotate values:
   
   putText(oImg, "0.0000", Point(xc2+16,yc2+4), fontFace, fontScale/1.5, Scalar::all(255), thickness,8);
   putText(oImg, "t [s]", Point((xc1+xc2)/2-8,yc1-16), fontFace, fontScale/1.1, Scalar::all(255), thickness,8);
   snprintf(buff, sizeof(buff), "%lf",d);
   putText(oImg, buff, Point(xc2+16,yc1+4), fontFace, fontScale/1.5, Scalar::all(255), thickness,8);
   snprintf(buff, sizeof(buff), "%lf",d/2.);
   putText(oImg, buff, Point(xc2+16,(yc1+yc2)/2.+4), fontFace, fontScale/1.5, Scalar::all(255), thickness,8);
   snprintf(buff, sizeof(buff), "%lf",d/4*3);
   putText(oImg, buff, Point(xc2+16,(yc2-yc1)/4.+yc1+4), fontFace, fontScale/1.5, Scalar::all(255), thickness,8);
   snprintf(buff, sizeof(buff), "%lf",d/4.);
   putText(oImg, buff, Point(xc2+16 ,+(yc2-yc1)/4.*3.+yc1+4), fontFace, fontScale/1.5, Scalar::all(255), thickness,8);

   OpenCVViewer->showImage(oImg);
}
///todo
}

Scalar DisplayAllPointsWidget::JetColorMap(double x)
{
   double cx=x*8.;
   int r=0;
   int g=0;
   int b=0;
   if(cx<1)
   {
    r=0;
    g=0;
    b=128+cx*128;
   }
   else if(cx<3)
   {
    r=0;
    g=256*(cx-1.)/2.;
    b=255;
   }
   else if(cx<5)
   {
    r=256*(cx-3.)/2.;
    g=255;
    b=255-256*(cx-3.)/2.;
   }
   else if(cx<7)
   {
    r=255;
    g=255-256*(cx-5.)/2.;
    b=0;
   }
   else
   {
    r=255-256*(cx-7.)/2.;
    g=0;
    b=0;
   }
   return Scalar(b,g,r);
}
//add set bg, from, to, radius

int DisplayAllPointsWidget::getRadius()
{
  return radius->value();
}

void DisplayAllPointsWidget::setRadius(double v)
{
    if(v>=0 && v<=100)
     radius->setValue(v);
}

