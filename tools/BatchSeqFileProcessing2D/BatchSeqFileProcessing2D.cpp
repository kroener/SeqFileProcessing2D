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
#include "Image.hpp"
#include <vector>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <string>
#include <unistd.h>
#include "iSeq.hpp"

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

Image<short> *work;
Image<double> *dwork;

vector<vector<double> > process(Image<unsigned char>* a, Image<unsigned char>* b, double minA, double maxA, 
                                double fracN, double minThreshold, int erode, int dilute, int blackOnWhite, 
                                int medianBlur1, int medianBlur2, int gaussK1, double gaussS1, int gaussK2, 
                                double gaussS2, vector<cv::Point> poly, int maskByThreshold, bool doAbs)
{
 vector<vector<double> > centers;
 if(!(work->getX()==a->getX() && work->getY()==a->getY()))
 work->create(a->getX(),a->getY());
 if(!(dwork->getX()==a->getX() && dwork->getY()==a->getY()))
 dwork->create(a->getX(),a->getY());
 Mat A;
 if(maskByThreshold!=0)
 {
  Image<unsigned char> mask;
  int maskT = maskByThreshold;
  if(maskT<0) maskT=-maskT;
  a->copyTo(mask);
  unsigned char* dataBuffer = mask.bufferPointer();
  Mat B = Mat(mask.getY(), mask.getX(), CV_8UC1, dataBuffer);
  if(maskByThreshold>0)
   threshold(B,A,maskT, 255, THRESH_BINARY_INV);
  else
   threshold(B,A,maskT, 255, THRESH_BINARY);
  cv::erode(A, A, getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)) ); 
 }
 bool useDouble=false;
 if(medianBlur1)
 {
  Image<unsigned char> a2(a);
  Image<unsigned char> b2(b);
  a2.medianBlur(medianBlur1);
  b2.medianBlur(medianBlur1);
  if(gaussK1 && gaussK1%2==1)
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
  if(gaussK1 && gaussK1%2==1)
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
 if(doAbs)
 {
  if(useDouble)
   dwork->Abs();
  else
   work->Abs();
 }
 if(medianBlur2)
 {
  if(useDouble)
   dwork->medianBlur(medianBlur2);
  else
   work->medianBlur(medianBlur2);
 }
 if(gaussK2 && gaussK2%2==1)
 {
  if(!useDouble)
  {
   (*dwork)=Image<double>(work);
   useDouble=true;
  }
  dwork->gaussianBlur(gaussK2,gaussS2);
 }
       
 if(poly.size()>2)
 {
  if(useDouble)
   dwork->maskWithPoly(poly);
  else
   work->maskWithPoly(poly);
 }
    
 if(maskByThreshold!=0)
 {
  if(useDouble)
   dwork->maskWithMat(A);
  else
   work->maskWithMat(A);
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
  cerr<<"threshold="<<threshold<<", maxdiff="<<maxdiff<<", hist[maxi][0]="<<hist[maxi][0]<<"\n";
  if(threshold<minThreshold+hist[maxi][0])threshold=minThreshold+hist[maxi][0];
  if(useDouble)
   dwork->Threshold(threshold);
  else
   work->Threshold(threshold);
  if(useDouble)
   dwork->erodeAndDilate(erode, dilute);
  else
   work->erodeAndDilate(erode, dilute);
  if(useDouble)
   centers=dwork->removeSmallBlobs(minA,maxA,threshold,maxdiff);
  else
   centers=work->removeSmallBlobs(minA,maxA,threshold,maxdiff);
  vector<double> last;
  last.push_back(maxdiff);
  centers.push_back(last);
 }
 return centers;
}

vector<vector<double> > process(Image<unsigned char>* a, Image<unsigned char>* b, double minA, double maxA, 
                                double fracN, double minThreshold, int erode, int dilute, int blackOnWhite, Rect  roi,
                                int medianBlur1, int medianBlur2, int gaussK1, double gaussS1, int gaussK2, double gaussS2,
                                vector<cv::Point> poly, int maskByThreshold, bool doAbs) 
{
 vector<vector<double> > centers;
 if(!(work->getX()==roi.width && work->getY()==roi.height))
  work->create(roi.width,roi.height);
 if(!(dwork->getX()==roi.width && dwork->getY()==roi.height))
  dwork->create(roi.width,roi.height);
 Mat A;
 if(maskByThreshold!=0)
 {
  Image<unsigned char> mask;
  int maskT = maskByThreshold;
  if(maskT<0) maskT=-maskT;
  a->copyTo(mask);
  unsigned char* dataBuffer = mask.bufferPointer();
  Mat B = Mat(mask.getY(), mask.getX(), CV_8UC1, dataBuffer);
  if(maskByThreshold<0)
   threshold(B,A,maskT, 255, THRESH_BINARY_INV);
  else
   threshold(B,A,maskT, 255, THRESH_BINARY);
  cv::erode(A, A, getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)) ); 
 }
 bool useDouble=false;
 if(medianBlur1)
 {
  Image<unsigned char> a2(a);
  Image<unsigned char> b2(b);    
  a2.medianBlur(medianBlur1);
  b2.medianBlur(medianBlur1);
  if(gaussK1 && gaussK1%2==1)
  {
   Image<double> a3(a2);
   Image<double> b3(b2);    
   a3.gaussianBlur(gaussK1,gaussS1);
   b3.gaussianBlur(gaussK1,gaussS1);
   useDouble=true;
   if(blackOnWhite)
    b3.diff(&a3,dwork,roi);
   else
    a3.diff(&b3,dwork,roi);            
  }
  else
  {
   if(blackOnWhite)
    b2.diff(&a2,work,roi);
   else
    a2.diff(&b2,work,roi);
  }
 }
 else
 {
  if(gaussK1 && gaussK1%2==1)
  {
   useDouble=true;
   Image<double> a2(a);
   Image<double> b2(b);
   a2.gaussianBlur(gaussK1,gaussS1);
   b2.gaussianBlur(gaussK1,gaussS1);
   if(blackOnWhite)
    b2.diff(&a2,dwork,roi);
   else
    a2.diff(&b2,dwork,roi);
  }
  else
  {
   if(blackOnWhite)
    b->diff(a,work,roi);
   else
    a->diff(b,work,roi);
  }
 }
 if(doAbs)
 {
  if(useDouble)
   dwork->Abs();
  else
   work->Abs();
 }
 if(medianBlur2)
 {
  if(useDouble)
   dwork->medianBlur(medianBlur2);
  else
   work->medianBlur(medianBlur2);
 }
 if(gaussK2 && gaussK2%2==1)
 {
  if(!useDouble) 
  {
   (*dwork)=Image<double>(work);
   useDouble=true;
  }
  dwork->gaussianBlur(gaussK2,gaussS2);
 }
 if(poly.size()>2)
 {
  if(useDouble)
   dwork->maskWithPoly(poly,roi.x,roi.y);
  else
   work->maskWithPoly(poly,roi.x,roi.y);
 }
 if(maskByThreshold!=0)
 {
  if(useDouble)
   dwork->maskWithMat(A(roi));
  else
   work->maskWithMat(A(roi));
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
  if(useDouble)
   dwork->erodeAndDilate(erode, dilute);
  else
   work->erodeAndDilate(erode, dilute);
  vector<vector<double> > centers;
  if(useDouble)
   centers=dwork->removeSmallBlobs(minA,maxA,threshold,maxdiff);
  else
   centers=work->removeSmallBlobs(minA,maxA,threshold,maxdiff);
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
  vector<double> last;
  last.push_back(maxdiff);
  centers.push_back(last);
 }
 return centers;
}


int main ( int argc, char *argv[])
{
 if(argc==4 || argc==6 || argc==19 || argc==20 || argc==21)
 {
  unsigned long start=0;
  unsigned long stop=0;
  double minA;
  double maxA;
  double fracN;
  double minThreshold;
  int erode;
  int dilute;
  int blackOnWhite;
  int useIth;
  int medianBlur1 = 0;
  int medianBlur2 = 0; 
  int gaussK1 = 0;
  double gaussS1 = 0;
  int gaussK2 = 0;
  double gaussS2 = 0;
  string polyRoiFilename;

  int maskT=0;
  vector<cv::Point> poly;
  
  if(argc==4 || argc==6)
  {
   FileStorage fs(argv[1], FileStorage::READ );
   fs["minArea"] >> minA;
   fs["maxArea"] >> maxA;
   fs["fracN"] >> fracN;
   fs["minThreshold"] >> minThreshold;
   fs["erode"] >> erode;
   fs["dilute"] >> dilute;
   fs["blackOnWhite"] >> blackOnWhite;
   fs["useIth"] >> useIth;
   fs["medianBlur1"] >> medianBlur1;
   fs["medianBlur2"] >> medianBlur2;
   fs["gaussK1"] >> gaussK1;
   fs["gaussS1"] >> gaussS1;
   fs["gaussK2"] >> gaussK2;
   fs["gaussS2"] >> gaussS2;
   fs["maskT"] >> maskT;
   fs["polyRoiFilename"] >> polyRoiFilename;
   if( polyRoiFilename != string())
   {
    FileStorage fs( polyRoiFilename.c_str(), FileStorage::READ );
    Mat tmp;
    fs["polyRoi"] >> tmp;
    for(int i=0; i< tmp.rows;i++)
    {
     poly.push_back(tmp.at<Point>(i,0));
    }  
   }
  }
  else
  {
   minA=atof(argv[5]);
   maxA=atof(argv[6]);
   fracN=atof(argv[7]);
   minThreshold=atof(argv[8]);
   useIth=atoi(argv[9]);
   erode=atoi(argv[10]);
   dilute=atoi(argv[11]);
   blackOnWhite=atoi(argv[12]);
   medianBlur1 = atoi(argv[13]);
   medianBlur2 = atoi(argv[14]); 
   gaussK1 = atoi(argv[15]);
   gaussS1 = atof(argv[16]);
   gaussK2 = atoi(argv[17]);
   gaussS2 = atof(argv[18]);
   if(argc==20 || argc==21)
    maskT = atoi(argv[19]);
   if(argc==21)
   {
    FileStorage fs( argv[20], FileStorage::READ );
    Mat tmp;
    fs["polyRoi"] >> tmp;
    for(int i=0; i< tmp.rows;i++)
    {
     poly.push_back(tmp.at<Point>(i,0));
    }
   }
  }
  cerr<<"minArea="<<minA<<"\n";
  cerr<<"maxArea="<<maxA<<"\n";
  cerr<<"fraM="<<fracN<<"\n";
  cerr<<"minThreshold="<<minThreshold<<"\n";
  cerr<<"erode="<<erode<<"\n";
  cerr<<"dilute="<<dilute<<"\n";
  cerr<<"blackOnWhite="<<blackOnWhite<<"\n";
  cerr<<"useIth="<<useIth<<"\n";
  iSeq seq;
  char* sFile=argv[2];
  if(argc>6) sFile=argv[1];
  bool ret=seq.open(sFile);
  if(ret!=0)
  {
   cerr<<"I can't open seqFile...!\n";
   return -1;
  }
  seq.setRequired(useIth+1);

  if(!seq.hasMeta())
  {
   cerr<<"Warning: I don't have metadata...\n";
  }
  unsigned long nFrames=seq.allocatedFrames();
  start=useIth;
  stop=nFrames-1;
  if(argc==6)
  {
   start=atoi(argv[4]);
   stop=atoi(argv[5]);
  }
  if(argc>6)
  {
   unsigned long tmpstart=atoi(argv[3]);
   unsigned long tmpstop=atoi(argv[4]);
   if(!(tmpstart==0 && tmpstop==0))
   {
   start=tmpstart;
   stop=tmpstop;
   }
  }
  if(start<useIth) start=useIth;
  cerr<<"Looping over "<<start << " to "<< stop<<"\n";
  work=new Image<short>;
  dwork=new Image<double>;
  FILE * pFile;
  char* oFile=argv[3];
  if(argc>6) oFile=argv[2];
  if(!( access( oFile, F_OK ) != -1 ))
  {
   pFile = fopen (oFile,"w");
   if(pFile == NULL)
   {
    cerr << "Error file "<<oFile<< " does exists!"<<endl;
    return -1;
   }
   else
   {
    if(start<stop && stop<=nFrames)
    {
     for( unsigned long n=start; n<stop; n++)
     {
      seq.loadImage(n);
      vector<vector<double> > centers;
      if(seq.getRoi())
      {
       Rect tmp=*seq.getRoi();
       centers=process(seq.getImagePointer(0), seq.getImagePointer(useIth), minA, maxA,fracN,minThreshold,erode,dilute,blackOnWhite,tmp,
                       medianBlur1,medianBlur2,gaussK1,gaussS1,gaussK2,gaussS2,poly,maskT,false);
      }
      else
      {
       centers=process(seq.getImagePointer(0), seq.getImagePointer(useIth), minA, maxA,fracN,minThreshold,erode,dilute,blackOnWhite,
                       medianBlur1,medianBlur2,gaussK1,gaussS1,gaussK2,gaussS2,poly, maskT,false);
      }
      if(centers.size()>1)
      {
       double maxDiff=centers[centers.size()-1][0];
       for(unsigned long i=0; i<centers.size()-1; i++)
       {
        vector<cv::Point> c;
        for(unsigned long j=0; j<centers[i][8]; j++)
         if(9+j*2+1<centers[i].size())
        {
          cv::Point tmp=cv::Point(centers[i][9+j*2],centers[i][9+j*2+1]);
          c.push_back(tmp);
        }
        vector<int> t=seq.timeOfFrameI(n);
        if(t.size()!=3)
         cerr<<"ERROR: could not get time for frame....\n";
        else
        {
         fprintf(pFile,"%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\n",
                       centers[i][0],
                       centers[i][1],
                       centers[i][3],
                       centers[i][2],
                       maxDiff,
                       static_cast<double>(n),
                       static_cast<double>(t[0]),
                       static_cast<double>(t[1]),
                       static_cast<double>(t[2]));
         fprintf(pFile,"#contour %u",c.size());
         for(int cc=0; cc<c.size(); cc++)
          fprintf(pFile," %i %i",c[cc].x,c[cc].y);
         fprintf(pFile,"\n");
        }
       }
      }
     }
    }
   }
  }
  else
  {
   cerr << "Error file "<<oFile<< " does exists!"<<endl;
  }
 }
 else
 {
  cerr<<"Usage: "<<argv[0]<<" settings.yml iseq out.txt [start stop]\n"<<"or\n"
      <<"       "<<argv[0]<<" iseq out.txt start stop minA maxA fracN \\\n"
      <<"minThreshold useIth erode dilute blackOnWhite medianBlur1 \\\n"
      <<"medianBlur2 gaussK1 gaussS1 gaussK2 gaussS2\\\n"
      <<"[maskT] [poly.yml] \n";
 }
 return 0;
}
