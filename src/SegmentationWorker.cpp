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
#include "SegmentationWorker.hpp"
#include "Image.hpp"
#include <vector>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
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
SegmentationWorker::SegmentationWorker()
{
    work=new Image<short>;
    dwork=new Image<double>;
}

SegmentationWorker::~SegmentationWorker()
{
    delete work;
    delete dwork;
}

void SegmentationWorker::process(Image<unsigned char>* a, Image<unsigned char>* b, double minA, double maxA, double fracN, double minThreshold, int erode, int dilute, int blackOnWhite, 
                                 int medianBlur1/*=0*/, int medianBlur2/*=0*/, int gaussK1/*=0*/, double gaussS1/*=0*/, int gaussK2/*=0*/, double gaussS2/*=0*/,
                                 vector<cv::Point> poly/*=vector<cv::Point>()*/ , int maskByThreshold /*= 0*/, bool doAbs /*=false*/)
{
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
        vector<vector<double> > centers;
        if(useDouble)
         centers=dwork->removeSmallBlobsNew(minA,maxA,threshold,maxdiff);
        else
         centers=work->removeSmallBlobsNew(minA,maxA,threshold,maxdiff);
        emit finished(centers,maxdiff);
    }

}

void SegmentationWorker::process(Image<unsigned char>* a, Image<unsigned char>* b, double minA, double maxA, double fracN, double minThreshold, int erode, int dilute, int blackOnWhite, Rect  roi,
                                 int medianBlur1 /*= 0*/, int medianBlur2 /*= 0*/, int gaussK1/*=0*/, double gaussS1/*=0*/, int gaussK2/*=0*/, double gaussS2/*=0*/,
                                 vector<cv::Point> poly/*=vector<cv::Point>()*/, int maskByThreshold /*= 0*/, bool doAbs /*=false*/) //if maskByThreshold is negative, keep above else below 
{
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
     if(!useDouble) {
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
         centers=dwork->removeSmallBlobsNew(minA,maxA,threshold,maxdiff);
        else
         centers=work->removeSmallBlobsNew(minA,maxA,threshold,maxdiff);
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
        emit finished(centers,maxdiff);
    }
}
