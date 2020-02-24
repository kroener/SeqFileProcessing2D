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
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include "Header.hpp"
#include "Image.hpp"
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

template<>
int Image<unsigned char>::loadFromSeq(fstream *seqFile, Header header, unsigned long i)
{
    if(!has_buffer)
        create(header.width(),header.height());
    else
    {
        if(!(x==header.width() && y==header.height()))
        {
            x=header.width();
            y=header.height();
            if (buffer!=NULL)
              delete[] buffer;
            buffer= new unsigned char[x*y];
        }
    }
    if(seqFile)
    {
        seqFile->seekg(header.imageStartOffset(i), seqFile->beg);
        seqFile->read((char*)buffer, sizeof(unsigned char)*header.height()*header.width());
        if(header.streampix6())
        {
            seqFile->read(reinterpret_cast<char*>(&sec), sizeof(int));
            seqFile->read(reinterpret_cast<char*>(&ms), sizeof(unsigned short));
            seqFile->read(reinterpret_cast<char*>(&us), sizeof(unsigned short));
            pts=sec+ms/1000.+us/1000000.;
        }
        else
        {
            seqFile->read(reinterpret_cast<char*>(&sec), sizeof(int));
            seqFile->read(reinterpret_cast<char*>(&ms), sizeof(unsigned short));
            seqFile->read(reinterpret_cast<char*>(&us), sizeof(unsigned short));
            pts=sec+ms/1000.+us/1000000.;
        }
    }
    return 0;
}

template <>
int Image<unsigned char>::save(string filename)
{
    if(has_buffer==1)
    {
        unsigned char maxg;
        unsigned char ming;
        ming=buffer[0];
        maxg=buffer[0];
        for(unsigned long i=0; i<x*y; i++)
        {
            if(buffer[i]<ming)ming=buffer[i];
            if(buffer[i]>maxg)maxg=buffer[i];
        }
        cerr <<"Picture has gray values from "<<static_cast<unsigned>(ming)<<" to "<<static_cast<unsigned>(maxg)<<"."<<endl;        
        Mat A = Mat(y, x, CV_8UC1, &buffer[0]);
        imwrite(filename, A);
    }
    else
    {
        cerr << "Have no image data to Save"<<endl;
    }
    return 0;
}

template <>
vector<vector<int> > Image<unsigned char>::dHistogram(int* maxi)
{

    vector<vector<int> > hist;
    int steps=256;
    int* counter;
    counter=new int[steps];

    for(int i=0; i<steps; i++) counter[i]=0;
    for(unsigned long i=0; i<x*y; i++)
        counter[int(buffer[i])]++;

    int n=0;
    int maxv=-1;
    for(int i=0; i<steps; i++)
        if(counter[i]!=0)
        {
            if(counter[i]>maxv)
            {
                maxv=counter[i];
                *maxi=n;
            }
            vector<int> tmp;
            tmp.push_back(i);
            tmp.push_back(counter[i]);
            hist.push_back(tmp);
            n++;
        }
    delete[] counter;
    return hist;
}

template <>
vector<vector<int> > Image<short>::dHistogram(int* maxi)
{
vector<vector<int> > hist;
int off=255;
int nb=511;
int histogram[nb];
for(int i=0; i<nb; i++) histogram[i] = 0;
#pragma omp parallel
{
    int j, histogram_private[nb];
    for(j=0; j<nb; j++) histogram_private[j] = 0;
    #pragma omp for nowait
    for(j=0; j<x*y; j++) {
           histogram_private[int(buffer[j])+off]++;
    }      
    #pragma omp critical 
    {
        for(j=0; j<nb; j++) histogram[j] += histogram_private[j];
    }
}
    int n=0;
    int maxv=-255;
    for(int i=0; i<nb; i++)
        if(histogram[i]!=0)
        {
            if(histogram[i]>maxv)
            {
                maxv=histogram[i];
                *maxi=n;
            }
            vector<int> tmp;
            tmp.push_back(i-off);
            tmp.push_back(histogram[i]);
            hist.push_back(tmp);
            n++;
        }
    return hist;
}

template <>
vector<vector<int> > Image<double>::dHistogram(int* maxi)
{
vector<vector<int> > hist;
int off=255;
int nb=511;
int histogram[nb];
for(int i=0; i<nb; i++) histogram[i] = 0;
#pragma omp parallel
{
    int j, histogram_private[nb];
    for(j=0; j<nb; j++) histogram_private[j] = 0;
    #pragma omp for nowait
    for(j=0; j<x*y; j++) {
           histogram_private[int(buffer[j]+0.5)+off]++;
    }      
    #pragma omp critical 
    {
        for(j=0; j<nb; j++) histogram[j] += histogram_private[j];
    }
}
    int n=0;
    int maxv=-255;
    for(int i=0; i<nb; i++)
        if(histogram[i]!=0)
        {
            if(histogram[i]>maxv)
            {
                maxv=histogram[i];
                *maxi=n;
            }
            vector<int> tmp;
            tmp.push_back(i-off);
            tmp.push_back(histogram[i]);
            hist.push_back(tmp);
            n++;
        }
    return hist;
}

//source: http://opencv-code.com/quick-tips/code-replacement-for-matlabs-bwareaopen/
template <>
vector<vector <double> >  Image<short>::removeSmallBlobs(double minsize,double maxsize,short minV,short maxV, bool filterByArea/*=false*/)
{
    unsigned char* tmpdata;
    tmpdata=new unsigned char[x*y];
    double scale=1;
    double offset=0;
    if(minV>=0 && maxV<=255)
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=buffer[i];
    else
    {
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=double(buffer[i]-minV)/double(maxV-minV)*255.;
        scale=255./double(maxV-minV);
        offset=minV;
    }
    Mat A = Mat(y, x, CV_8UC1, &tmpdata[0]);
    vector<std::vector<Point> > contours;
    findContours(A.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    vector< vector <double> > blobs;
    for (unsigned long i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        if ((area > 0 && (area <= minsize || area >=maxsize)) && filterByArea)
        {
            drawContours(A, contours, i, CV_RGB(0,0,0), -1);
        }
        else if(area!=0)
        {
            Moments mu=moments(contours[i],false);
            double xp=mu.m10/mu.m00;
            double yp=mu.m01/mu.m00;
            cv::Mat m( y, x, CV_8UC1, cv::Scalar(0) );
            drawContours(m, contours, i, Scalar(255), cv::FILLED);
            double minVal, maxVal;
            Point minLoc, maxLoc;
            minMaxLoc(A,&minVal,&maxVal,&minLoc,&maxLoc, m);
            vector<double> tmp;
            tmp.push_back(xp);
            tmp.push_back(yp);
            tmp.push_back(area);
            tmp.push_back(maxVal/scale+offset);
            double ratio;
            double perimeter = arcLength(Mat(contours[i]), true);
            ratio = 4 * CV_PI * mu.m00 / (perimeter * perimeter);
            tmp.push_back(ratio);
            double denominator = std::sqrt(std::pow(2 * mu.mu11, 2) + std::pow(mu.mu20 - mu.mu02, 2));
            const double eps = 1e-2;
            if (denominator > eps)
            {
                double cosmin = (mu.mu20 - mu.mu02) / denominator;
                double sinmin = 2 * mu.mu11 / denominator;
                double cosmax = -cosmin;
                double sinmax = -sinmin;

                double imin = 0.5 * (mu.mu20 + mu.mu02) - 0.5 * (mu.mu20 - mu.mu02) * cosmin - mu.mu11 * sinmin;
                double imax = 0.5 * (mu.mu20 + mu.mu02) - 0.5 * (mu.mu20 - mu.mu02) * cosmax - mu.mu11 * sinmax;
                ratio = imin / imax;
            }
            else
            {
                ratio = 1;
            }
            tmp.push_back(ratio);
            vector < Point > hull;
            convexHull(Mat(contours[i]), hull);
            double hullArea = contourArea(Mat(hull));
            ratio = area / hullArea;
            tmp.push_back(ratio);
            vector<double> dists;
            for (size_t j = 0; j < contours[i].size(); j++)
            {
                Point2d pt = contours[i][j];
                dists.push_back(norm(Point2d(xp,yp) - pt));
            }
            std::sort(dists.begin(), dists.end());
            ratio = (dists[(dists.size() - 1) / 2] + dists[dists.size() / 2]) / 2.;
            tmp.push_back(ratio);
            tmp.push_back(double(contours[i].size()));
            for(unsigned int j=0; j<contours[i].size(); j++)
            {
             tmp.push_back(double(contours[i][j].x));
             tmp.push_back(double(contours[i][j].y));
            }
            blobs.push_back(tmp);
        }
    }
    delete[] tmpdata;

    return blobs;
}

//source: http://opencv-code.com/quick-tips/code-replacement-for-matlabs-bwareaopen/
template <>
vector< std::vector<Point> >  Image<short>::removeSmallBlobsContours(double minsize,double maxsize,short minV,short maxV)
{
    unsigned char* tmpdata;
    tmpdata=new unsigned char[x*y];
    if(minV>=0 && maxV<=255)
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=buffer[i];
    else
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=double(buffer[i]-minV)/double(maxV-minV)*255.;

    Mat A = Mat(y, x, CV_8UC1, &tmpdata[0]);
    vector<std::vector<Point> > contours;
    vector<std::vector<Point> > tmp;
    findContours(A.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (unsigned long i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        if(area > minsize && area <maxsize)
        {
            tmp.push_back(contours[i]);
        }
    }
  
    delete[] tmpdata;

    return tmp;
}

template <>
template <>
int Image<unsigned char>::insert(Image<unsigned char>& a,int ofx, int ofy)
{
    if( a.hasBuffer()==1 && has_buffer==1)
    {
    int ix=a.getX();
    int iy=a.getY();
        if(ix+ofx <= x && iy+ofy <= y)
        {
            #pragma omp parallel for
            for(unsigned long j=0; j<iy; j++)
            {
                memcpy(&buffer[ofx+(j+ofy)*y], &a.buffer[j*iy], ix);
            }
        }
        else{
            cerr<<"Image of size "<<a.getX()<<" x "<<a.getY()<<" does not fit into "<<x<<"x"<<y<<" with offset "<<ofx<<","<<ofy<<"\n";
        }

    }
    else
     cerr<<"Image has no data\n";
    return 0;
}

template <>
template <>
int Image<unsigned char>::insert(Image<unsigned char>* a,int ofx, int ofy)
{
    if( a->hasBuffer()==1 && has_buffer==1)
    {
    int ix=a->getX();
    int iy=a->getY();
        if(ix+ofx <= x && iy+ofy <= y)
        {
            for(unsigned long j=0; j<iy; j++)
            {
                memcpy(&buffer[ofx+(j+ofy)*x], &a->buffer[j*ix], ix);
            }
        }
        else{
            cerr<<"Image of size "<<a->getX()<<" x "<<a->getY()<<" does not fit into "<<x <<"x"<< y<<" with offset "<<ofx<<","<<ofy<<"\n";
        }

    }
    else
     cerr<<"Image has no data\n";
    return 0;
}


template <>
void Image<unsigned char>::medianBlur(int k)
{
     cv::Mat A = cv::Mat(y, x, CV_8UC1, &buffer[0]);
     cv::Mat B;
     cv::medianBlur(A,B,k);
     B.copyTo(A);
}

template <>
void Image<double>::medianBlur(int k)
{
     if(k==3 || k==5)
     {
      cv::Mat A = cv::Mat(y, x, CV_64FC1, &buffer[0]);
      cv::Mat B;
      A.convertTo(B, CV_32FC1);            
      cv::Mat C;
      cv::medianBlur(B,C,k);
      C.copyTo(A);
     }
     else
     {
      cerr<<"Warming: for k>5 I need to convert to UINT8...\n";
      cv::Mat A = cv::Mat(y, x, CV_64FC1, &buffer[0]);
      cv::Mat B;
      A.convertTo(B, CV_8UC1);
      cv::Mat C;
      cv::medianBlur(B,C,k);
      C.convertTo(B, CV_64FC1);
      B.copyTo(A);      
     }
}

template <>
void Image<short>::medianBlur(int k)
{
     if(k==3 || k==5)
     {
      cv::Mat A = cv::Mat(y, x, CV_16SC1, &buffer[0]);
      cv::Mat B;
      cv::medianBlur(A,B,k);
      B.copyTo(A);
     }
     else
     {
      cerr<<"Warming: for k>5 I need to convert to UINT8...\n";
      cv::Mat A = cv::Mat(y, x, CV_16SC1, &buffer[0]);
      cv::Mat B;
      A.convertTo(B, CV_8UC1);
      cv::Mat C;
      cv::medianBlur(B,C,k);
      C.convertTo(B, CV_16SC1);
      B.copyTo(A);      
     }
}


template <>
void Image<unsigned char>::gaussianBlur(int k, double s)
{
     cv::Mat A = cv::Mat(y, x, CV_8UC1, &buffer[0]);
     cv::Mat B;
     cv::GaussianBlur( A, B, cv::Size( k, k ), s, s );
     B.copyTo(A);
}

template <>
void Image<double>::gaussianBlur(int k, double s)
{
     cv::Mat A = cv::Mat(y, x, CV_64FC1, &buffer[0]);
     cv::Mat B;
     cv::GaussianBlur( A, B, cv::Size( k, k ), s, s );
     B.copyTo(A);
}

template <>
void Image<short>::gaussianBlur(int k, double s)
{
     cv::Mat A = cv::Mat(y, x, CV_16SC1, &buffer[0]);
     cv::Mat B;
     cv::GaussianBlur( A, B, cv::Size( k, k ), s, s );
     B.copyTo(A);
}

template <>
vector<vector <double> >  Image<short>::removeSmallBlobsNew(double minsize,double maxsize,short minV,short maxV)
{
    vector< vector <double> > blobs;
    unsigned char* tmpdata;
    tmpdata=new unsigned char[x*y];
    double scale=1;
    double offset=0;
    if(minV>=0 && maxV<=255)
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=buffer[i];
    else
    {
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=double(buffer[i]-minV)/double(maxV-minV)*255.;
        scale=255./double(maxV-minV);
        offset=minV;
    }
    Mat A = Mat(y, x, CV_8UC1, &tmpdata[0]);
    cv::Mat labels;
    cv::Mat stats;
    cv::Mat centroids;
    int nLabels = connectedComponentsWithStats(A, labels, stats, centroids, 8, CV_32S);
    cv::Mat objImg;
    cv::Mat tmp;
    for(unsigned int i=1; i<nLabels; i++)
    {
      int area=stats.at<int>(i,cv::CC_STAT_AREA);
      if(area>=minsize && area<=maxsize)
      {
       int bb_left=stats.at<int>(i,cv::CC_STAT_LEFT);
       int bb_top=stats.at<int>(i,cv::CC_STAT_TOP);
       int bb_w=stats.at<int>(i,cv::CC_STAT_WIDTH);
       int bb_h=stats.at<int>(i,cv::CC_STAT_HEIGHT);
       if(bb_left>0) bb_left--;
       if(bb_top>0) bb_top--;
       if(bb_left+bb_w+2<x) bb_w+=2; 
       else bb_w=x-bb_left-1;
       if(bb_top+bb_h+2<y) bb_h+=2; 
       else bb_h=y-bb_top-1;
       cv::Rect roi=cv::Rect(bb_left, bb_top, bb_w, bb_h);
       labels(roi).copyTo(tmp);
       objImg = (tmp==i);
       Moments mu = cv::moments(objImg, true);
       double xp=centroids.at<double>(i,0);
       double yp=centroids.at<double>(i,1);
       double minVal, maxVal;
       Point minLoc, maxLoc;
       minMaxLoc(A(roi),&minVal,&maxVal,&minLoc,&maxLoc, objImg);
       vector<double> tmp;
       tmp.push_back(xp);
       tmp.push_back(yp);
       tmp.push_back(area);
       tmp.push_back(maxVal/scale+offset);
       vector<std::vector<Point> > contours;
       findContours(objImg.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
       if(contours.size()==1 && contours[0].size()>2)
       {
       double ratio;
       double perimeter = arcLength(Mat(contours[0]), true);
       ratio = 4 * CV_PI * mu.m00 / (perimeter * perimeter);
       tmp.push_back(ratio);
       double denominator = std::sqrt(std::pow(2 * mu.mu11, 2) + std::pow(mu.mu20 - mu.mu02, 2));
       const double eps = 1e-2;
       if (denominator > eps)
       {
         double cosmin = (mu.mu20 - mu.mu02) / denominator;
         double sinmin = 2 * mu.mu11 / denominator;
         double cosmax = -cosmin;
         double sinmax = -sinmin;
         double imin = 0.5 * (mu.mu20 + mu.mu02) - 0.5 * (mu.mu20 - mu.mu02) * cosmin - mu.mu11 * sinmin;
         double imax = 0.5 * (mu.mu20 + mu.mu02) - 0.5 * (mu.mu20 - mu.mu02) * cosmax - mu.mu11 * sinmax;
         ratio = imin / imax;
        }
        else
        {
         ratio = 1;
        }
        tmp.push_back(ratio);
        vector < Point > hull;
        convexHull(Mat(contours[0]), hull);
        double hullArea = contourArea(Mat(hull));
        ratio = area / hullArea;
        tmp.push_back(ratio);
        vector<double> dists;
        for (size_t j = 0; j < contours[0].size(); j++)
        {
         Point2d pt = contours[0][j];
         pt +=cv::Point2d(bb_left,bb_top);
         dists.push_back(norm(Point2d(xp,yp) - pt));
        }
        std::sort(dists.begin(), dists.end());
        ratio = (dists[(dists.size() - 1) / 2] + dists[dists.size() / 2]) / 2.;
        tmp.push_back(ratio);
        tmp.push_back(double(contours[0].size()));
        for(unsigned int j=0; j<contours[0].size(); j++)
        {
         tmp.push_back(double(contours[0][j].x)+bb_left);
         tmp.push_back(double(contours[0][j].y)+bb_top);
        } 
        }
        else
        {
         double ratio=0;
         tmp.push_back(ratio);
         tmp.push_back(ratio);
         tmp.push_back(ratio);   
        }
        blobs.push_back(tmp);          
      }
    }
    delete[] tmpdata;
    return blobs;
}

template <>
int Image<double>::erodeAndDilate(int i, int j)
{
  cv::Mat A = cv::Mat(y, x, CV_64F, &buffer[0]);
  if(i>0)
  {
  erode(A, A, getStructuringElement(cv::MORPH_RECT, cv::Size(i, i)) );
  dilate( A, A, getStructuringElement(cv::MORPH_RECT, cv::Size(i, i)) ); 
  }
  if(j>0)
  {
  dilate( A, A, getStructuringElement(cv::MORPH_RECT, cv::Size(j, j)) ); 
  erode( A, A, getStructuringElement(cv::MORPH_RECT, cv::Size(j, j)) );
  }
  return 0;
};

template <>
int Image<unsigned char>::erodeAndDilate(int i, int j)
{
  cv::Mat A = cv::Mat(y, x, CV_8U, &buffer[0]);
  if(i>0)
  {
  erode(A, A, getStructuringElement(cv::MORPH_RECT, cv::Size(i, i)) );
  dilate( A, A, getStructuringElement(cv::MORPH_RECT, cv::Size(i, i)) ); 
  }
  if(j>0)
  {
  dilate( A, A, getStructuringElement(cv::MORPH_RECT, cv::Size(j, j)) ); 
  erode( A, A, getStructuringElement(cv::MORPH_RECT, cv::Size(j, j)) );
  }
  return 0;
};
