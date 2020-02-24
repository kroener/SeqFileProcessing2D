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
#ifndef Image_hpp
#define Image_hpp
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
class iSeq;
class oSeq;
/**
  * \brief template class for the storage of image data 
  */
template <class T>
class Image
{
    friend class iSeq;
    friend class oSeq;
public:
    Image();
    template <class S> Image(Image<S>& a);
    template <class S> Image(Image<S>* a);
    Image(Image<T>&& other);
    Image<T>& operator=(Image<T>&& other);
    ~Image();
    template <class S, class U> int diff(Image<S>& a,Image<U>& b);
    template <class S, class U> int diff(Image<S>* a,Image<U>* b);
    template <class S, class U> int diff(Image<S>& a,Image<U>& b, cv::Rect roi);
    template <class S, class U> int diff(Image<S>* a,Image<U>* b, cv::Rect roi);
    unsigned long getX();
    unsigned long getY();
    bool hasBuffer();
    void hasNoBuffer();

    template <class S> int maskWithImage(Image<S>& a);
    template <class S> int maskWithImage(Image<S>* a);

    int maskWithMat(cv::Mat mask);
    int maskWithPoly(vector<cv::Point> mask, int ox=0, int oy =0);
    bool isInsidePoly(int x, int y, vector<cv::Point> ppp);

    double getT();
    int setT(double t);

    int getTsec();
    int setTsec(int t);

    unsigned short getTmsec();
    int setTmsec(unsigned short t);

    unsigned short getTusec();
    int setTusec(unsigned short t);

    int create(unsigned long xp, unsigned long yp);
    int create(unsigned long xp, unsigned long yp,unsigned char c);
    int setValue(unsigned long xp, unsigned long yp, T value);
    int save(string filename);
    T getValue(unsigned long xp, unsigned long yp);
    int read(fstream *seqFileHandle,unsigned long offx, unsigned long x, unsigned long y, int depth);
    int write();
    int loadFromSeq(fstream *seqFile, Header header, unsigned long i);
    int saveToSeq(fstream& seqFile, Header header);
    int saveToSeq(fstream* seqFile, Header header,unsigned int i);

    int createDifference(int i,int j);
    int calculateHistogram(int i);
    int Threshold(int i, int th);
    int clean(int minArea,int maxArea);
    vector< vector<double> > center();
    int displayImage();
    vector< vector<double> > Histogram();
    vector< vector<int> > dHistogram(int* maxi);
    int Threshold(double threshold);
    int Threshold(double threshold,cv::Mat n, double mT, double nm);
    vector<vector<double> > findBlob(double minArea,double maxAreaa, double minThreshold, double maxThreshold);
    int normalize(int minx, int maxx);
    T* bufferPointer();
    int erodeAndDilate(int i, int j);

    void simpleDownScale(int xd, int yd);
    vector<vector <double> > removeSmallBlobs(double minsize,double maxsize,T minV, T maxV,bool filterByArea = true);
    vector<vector <double> > removeSmallBlobsNew(double minsize,double maxsize,T minV = 0, T maxV = 255);
    vector<std::vector<cv::Point> > removeSmallBlobsContours(double minsize,double maxsize,T minV, T maxV);
    vector<vector <double> > removeSmallBlobs(double minsize,double maxsize);
    cv::Mat image(int n  = 0);
    cv::Mat image(cv::Rect roi);
    T* buffer;
    template <class S> int insert(Image<S>& a, int ofx, int ofy);
    template <class S> int insert(Image<S>* a, int ofx, int ofy);
    template <class S> int copyTo(Image<S>& a);
    template <class S> int copyTo(Image<S>* a);
    bool addOneInPoly(vector<cv::Point2f> poly);
    bool isInPoly(int ix, int iy,vector<cv::Point2f> poly);
    vector<int> polyBin(vector<cv::Point2f> poly);
    bool copyFrom(cv::Mat A);
    vector<double> getAverage();
    int loadROIFromSeq(fstream *seqFile, Header header, unsigned int i, int xmin, int ymin, int xmax,int ymax);
    Image<double> SobelMagnitude();
    int linXFiltI(int f0, int f1, int f2);
    int linYFiltI(int f0, int f1, int f2);
    template<class S> Image<double> magWith(Image<S> a);
    T getMaxValue();
    T getMinValue();
    void medianBlur(int k);
    void gaussianBlur(int k, double s);
    void Abs();
    void bwareaopen(int lowerCutOff, int upperCutOff);

private:
    int has_buffer;
    unsigned long x;
    unsigned long y;
    double pts;
    unsigned long sec;
    unsigned short ms;
    unsigned short us;
    static int haveWindow;
    int offx;
    int offy;
};

template <class T>
Image<T>::Image()
{
    has_buffer=0;
    x=0;
    y=0;
    sec=0;
    ms=0;
    us=0;
    offx=0;
    offy=0;
}

template <class T>
template <class S>
Image<T>::Image(Image<S>& a)
{
    x=a.getX();
    y=a.getY();
    if(a.hasBuffer())
    {
        buffer=new T[x*y];
        has_buffer=1;
        #pragma omp parallel for
        for(unsigned long i=0; i<x; i++)
            for(unsigned long j=0; j<y; j++)
            {
                buffer[i+x*j]=(T) a.getValue(i,j);
            }
    sec=a.getTsec();
    ms=a.getTmsec();
    us=a.getTusec();
    }
    else
    {
     has_buffer=0;
     x=0;
     y=0;
     offx=0;
     offy=0;
    }
}

template <class T>
template <class S>
Image<T>::Image(Image<S>* a)
{
    x=a->getX();
    y=a->getY();
    if(a->hasBuffer())
    {
        buffer=new T[x*y];
        has_buffer=1;
        #pragma omp parallel for
        for(unsigned int i=0; i<x; i++)
            for(unsigned int j=0; j<y; j++)
            {
                buffer[i+x*j]=(T) a->getValue(i,j);
            }
    sec=a->getTsec();
    ms=a->getTmsec();
    us=a->getTusec();
    }
    else
    {
     has_buffer=0;
     x=0;
     y=0;
     offx=0;
     offy=0;
    }
}

template <class T>
Image<T>::Image(Image<T>&& other)
{
    has_buffer=other.has_buffer;
    x=other.x;
    y=other.y;
    sec=other.sec;
    ms=other.ms;
    us=other.us;
    buffer=other.buffer;
    other.has_buffer=0;
}

template <class T>
Image<T>& Image<T>::operator=(Image<T>&& other)
{
    if(this!=&other)
    {
    if(has_buffer==1)
      delete[] buffer;
    has_buffer=other.has_buffer;
    x=other.x;
    y=other.y;
    sec=other.sec;
    ms=other.ms;
    us=other.us;
    buffer=other.buffer;
    other.has_buffer=0;
    }
    return *this;
}

template <class T>
Image<T>::~Image()
{
    if(has_buffer==1)
        delete[] buffer;
}

template <class T>
unsigned long Image<T>::getX()
{
    return x;
}

template <class T>
unsigned long Image<T>::getY()
{
    return y;
}

template <class T>
double Image<T>::getT()
{
    return pts;
}

template <class T>
int Image<T>::setT(double t)
{
    pts=t;
    sec=int(t);
    ms=int((pts-sec)*1000);
    us=int((pts-sec-ms/1000)*1000);
    return 0;
}

template <class T>
int Image<T>::getTsec()
{
    return sec;
}

template <class T>
int Image<T>::setTsec(int t)
{
    sec=t;
    pts=sec+ms/1000.+us/1000000.;
    return 0;
}

template <class T>
unsigned short Image<T>::getTmsec()
{
    return ms;
}

template <class T>
int Image<T>::setTmsec(unsigned short t)
{
    ms=t;
    pts=sec+ms/1000.+us/1000000.;
    return 0;
}


template <class T>
unsigned short Image<T>::getTusec()
{
    return us;
}

template <class T>
int Image<T>::setTusec(unsigned short t)
{
    us=t;
    pts=sec+ms/1000.+us/1000000.;
    return 0;
}


template <class T>
int Image<T>::create(unsigned long xp, unsigned long yp)
{
    if(!(x==xp && y==yp && has_buffer==1))
    {
    x=xp;
    y=yp;
    if(has_buffer==1)
     delete[] buffer;
    buffer=new T[x*y];
    has_buffer=1;
    #pragma omp parallel for
    for(unsigned long i=0; i<x; i++)
        for(unsigned long j=0; j<y; j++)
            buffer[i+x*j]=0;
    }
    return 0;
}

template <class T>
int Image<T>::create(unsigned long xp, unsigned long yp,unsigned char c)
{
    if(!(x==xp && y==yp && has_buffer==1))
    {
    x=xp;
    y=yp;
    if(has_buffer==1)
     delete[] buffer;
    buffer=new T[x*y];
    has_buffer=1;
    #pragma omp parallel for
    for(unsigned long i=0; i<x; i++)
        for(unsigned long j=0; j<y; j++)
            buffer[i+x*j]=c;
    }
    return 0;
}

template <class T>
bool Image<T>::hasBuffer()
{
    if(has_buffer==1)
        return true;
    else
        return false;
}

template <class T>
void Image<T>::hasNoBuffer()
{
  has_buffer=0;
}

template <class T>
int Image<T>::setValue(unsigned long xp, unsigned long yp, T value)
{
    if(has_buffer && xp<x && yp<y)
        buffer[xp+x*yp]=value;
    return 0;
}

template <class T>
T Image<T>::getValue(unsigned long xp, unsigned long yp)
{
    T tmp=0;
    if(has_buffer && xp<x && yp<y)
        tmp=buffer[xp+x*yp];
    return tmp;
}


template<class T>
int Image<T>::loadFromSeq(fstream *seqFile, Header header, unsigned long i)
{
    if(!has_buffer)
        create(header.width(),header.height());
    else
    {
        if(!(x==header.width() && y==header.height()))
        {
            x=header.width();
            y=header.height();
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
            us=0;
            seqFile->read(reinterpret_cast<char*>(&ms), sizeof(unsigned short));
            pts=sec+ms/1000.;
        }
    }
    return 0;
}

template <class T>
int Image<T>::save(string filename)
{
    if(has_buffer==1)
    {
        T maxg;
        T ming;
        ming=buffer[0];
        maxg=buffer[0];
        #pragma omp parallel for reduction(max : maxg)
        for(unsigned long i=0; i<x*y; i++)
        {
            if(buffer[i]>maxg)maxg=buffer[i];
        }
        #pragma omp parallel for reduction(min : ming)
        for(unsigned long i=0; i<x*y; i++)
        {
            if(buffer[i]<ming)ming=buffer[i];
        }
        cerr <<"Picture has gray values from "<<ming<<" to "<<maxg<<"."<<endl;
        if(ming<0||maxg>255)
        {
            cerr <<"Need to rescale!"<<endl;
            #pragma omp parallel for
            for(unsigned long i=0; i<x*y; i++) buffer[i]=int((double(buffer[i])-double(ming))/(double(maxg)-double(ming))*255.);
        }
        else
        {
            cerr <<"Gray values are OK!"<<endl;
        }
        ofstream pgm_save;
        pgm_save.open(filename.c_str(), ios::binary);
        pgm_save << "P5"<<endl<<x<<" "<<y<<endl<<255<<endl;
        for(unsigned long i=0; i<y; i++)
            for(unsigned long j=0; j<x; j++)
                pgm_save << static_cast < unsigned char >(buffer[j+x*i]);
        pgm_save.close();
    }
    else
    {
        cerr << "Have no image data to Save"<<endl;
    }
    return 0;
}

template <class T>
template <class S,class U>
int Image<T>::diff(Image<S>& a,Image<U>& b)
{
    if( a.hasBuffer()==1 && b.hasBuffer()==1 && has_buffer==1)
        if(a.getX() == x && b.getX() == x && a.getY() == y && b.getY() == y)
        {
            #pragma omp parallel for
            for(unsigned long i=0; i<x*y; i++)
            {
                int iy=int(i/x);
                int ix=i-x*iy;
                b.buffer[ix+x*iy]=buffer[ix+x*iy]-a.buffer[ix+x*iy];
            }
        }
    return 0;
}

template <class T>
template <class S,class U>
int Image<T>::diff(Image<S>& a, Image<U>& b, cv::Rect roi)
{
    if( a.hasBuffer()==1 && b.hasBuffer()==1 && has_buffer==1)
        if(a.getX() == x && b.getX() == roi.width && a.getY() == y && b.getY() == roi.height && 
           roi.x>=0 && roi.x<x && roi.y>=0 && roi.y<y && roi.x+roi.width<x && roi.y+roi.height<y && roi.height>0 && roi.width>0 )
        {
            #pragma omp parallel for
            for(unsigned long i=0; i<roi.width*roi.height; i++)
            {
                int iy=int(i/roi.width);
                int ix=i-roi.width*iy;
                b.buffer[ix+roi.width*iy]=buffer[ix+roi.x+x*(iy+roi.y)]-a.buffer[ix+roi.x+x*(iy+roi.y)];
            }
        }
    return 0;
}

template <class T>
template <class S>
int Image<T>::maskWithImage(Image<S>& a)
{
    if( a.hasBuffer()==1)
        if(a.getX() == x && a.getY() == y)
        {
            #pragma omp parallel for
            for(unsigned long i=0; i<x*y; i++)
            {
                int iy=int(i/x);
                int ix=i-x*iy;
                if(a.getValue(ix,iy)==0)
                   buffer[ix+x*iy]=0;
            }

        }
    return 0;
}

template <class T>
template <class S>
int Image<T>::maskWithImage(Image<S>* a)
{
    if( a->hasBuffer()==1)
        if(a->getX() == x && a->getY() == y)
        {
            #pragma omp parallel for
            for(unsigned long i=0; i<x*y; i++)
            {
                int iy=int(i/x);
                int ix=i-x*iy;
                if(a->getValue(ix,iy)==0)
                   buffer[ix+x*iy]=0;
            }

        }
    return 0;
}

template <class T>
int Image<T>::maskWithMat(cv::Mat mask)
{
     if(mask.cols == x && mask.rows == y)
     {
        if(mask.type()==CV_8UC1)
        {
            #pragma omp parallel for
            for(unsigned long i=0; i<x*y; i++)
            {
                int iy=int(i/x);
                int ix=i-x*iy;
                if(mask.at<unsigned char>(iy,ix)==0)
                   buffer[ix+x*iy]=0;
            }
        }
        else
        {
           cerr<<"Only Mat of type CV_8UC1 are supported as mask...!\n";     
        }
     }
     else
        cerr<<"Mask does not fit Image dimensions...!\n";
    return 0;
}

template <class T>
int Image<T>::maskWithPoly(vector<cv::Point> mask,int ox/*=0*/, int oy /*=0*/)
{
     if(mask.size() > 2)
     {
        for(unsigned long i=0; i<x*y; i++)
        {
            int iy=int(i/x);
            int ix=i-x*iy;
            if(!isInsidePoly(ix+ox,iy+oy,mask))
               buffer[ix+x*iy]=0;
        }
     }
     else
        cerr<<"Not enough Points to build polygon\n";
    return 0;
}

template <class T>
template <class S,class U>
int Image<T>::diff(Image<S>* a,Image<U>* b)
{
    if( a->hasBuffer()==1 && b->hasBuffer()==1 && has_buffer==1)
        if(a->getX() == x && b->getX() == x && a->getY() == y && b->getY() == y)
        {
            #pragma omp parallel for
            for(unsigned long i=0; i<x*y; i++)
            {
                int iy=int(i/x);
                int ix=i-x*iy;
                (*b).buffer[ix+x*iy]=buffer[ix+x*iy]-(*a).buffer[ix+x*iy];
            }
        }
    return 0;
}

template <class T>
template <class S,class U>
int Image<T>::diff(Image<S>* a,Image<U>* b, cv::Rect roi)
{
    if( a->hasBuffer()==1 && b->hasBuffer()==1 && has_buffer==1)
        if(a->getX() == x && b->getX() == roi.width && a->getY() == y && b->getY() == roi.height &&
           roi.x>=0 && roi.x<x && roi.y>=0 && roi.y<y && roi.x+roi.width<x && roi.y+roi.height<y && roi.height>0 && roi.width>0 )
        {
            #pragma omp parallel for
            for(unsigned long i=0; i<roi.width*roi.height; i++)
            {
                int iy=int(i/roi.width);
                int ix=i-roi.width*iy;
                (*b).buffer[ix+roi.width*iy]=buffer[ix+roi.x+x*(iy+roi.y)]-(*a).buffer[ix+roi.x+x*(iy+roi.y)];
            }
        }
    return 0;
}

template <class T>
int Image<T>::displayImage()
{
    unsigned char* tmpdata;
    tmpdata=new unsigned char[x*y];
    #pragma omp parallel for
    for(unsigned long i=0; i<x*y; i++)tmpdata[i]=buffer[i];
    if(haveWindow==0)
    {
        cv::namedWindow("View");
        haveWindow=1;
    }
    cv::Mat A = cv::Mat(y, x, CV_8UC1, &tmpdata[0]);
    cv::imshow("View", A);
    cv::waitKey(1000);
    delete[] tmpdata;
    return 0;
}

template <class T>
vector<vector<double> > Image<T>::Histogram()
{
    T minv=buffer[0];
    T maxv=buffer[0];
    #pragma omp parallel for reduction(min : minv)
    for(unsigned long i=0; i<x*y; i++)
    {
        if(buffer[i]<minv)
        {
            minv=buffer[i];
        }
    }

    #pragma omp parallel for reduction(max : maxv)
    for(unsigned long i=0; i<x*y; i++)
    {
        if(buffer[i]>maxv)
        {
            maxv=buffer[i];
        }
    }
    vector<vector<double> > hist;
    int steps=(maxv-minv+1)*1;
    T off=minv;
    double scale=(maxv-minv+1)/steps;
    int* counter;
    counter=new int[steps];

    #pragma omp parallel for
    for(int i=0; i<steps; i++) counter[i]=0;
    #pragma omp parallel for
    for(unsigned long i=0; i<x*y; i++)
        counter[int((buffer[i]-off)/scale)]++;

    for(int i=0; i<steps; i++)
        if(counter[i]!=0)
        {
            vector<double> tmp;
            tmp.push_back((i+0.5)*scale+off);
            tmp.push_back(counter[i]);
            hist.push_back(tmp);
        }
    delete[] counter;
    return hist;
}

template <class T>
vector<vector<int> > Image<T>::dHistogram(int* maxi)
{
    T minv=buffer[0];
    T maxv=buffer[0];
    #pragma omp parallel for reduction(min : minv)
    for(unsigned long i=0; i<x*y; i++)
    {
        if(buffer[i]<minv)
        {
            minv=buffer[i];
        }
    }

    #pragma omp parallel for reduction(max : maxv)
    for(unsigned long i=0; i<x*y; i++)
    {
        if(buffer[i]>maxv)
        {
            maxv=buffer[i];
        }
    }
    vector<vector<int> > hist;
    int steps=(maxv-minv+1)*1;
    T off=minv;
    double scale=(maxv-minv+1)/steps;
    int* counter;
    counter=new int[steps];
    #pragma omp parallel for
    for(int i=0; i<steps; i++) counter[i]=0;
    #pragma omp parallel for
    for(unsigned long i=0; i<x*y; i++)
        counter[int((buffer[i]-off)/scale)]++;
    int n=0;
    int maxiv=-1;
    for(int i=0; i<steps; i++)
        if(counter[i]!=0)
        {
            if(counter[i]>maxiv)
            {
                maxiv=counter[i];
                *maxi=n;
            }
            vector<int> tmp;
            tmp.push_back((i+0.5)*scale+off);
            tmp.push_back(counter[i]);
            hist.push_back(tmp);
            n++;
        }
    delete[] counter;
    return hist;
}

template <class T>
int Image<T>::Threshold(double threshold)
{
    if(has_buffer)
    {
        #pragma omp parallel for
        for(unsigned long i=0; i<x*y; i++)
            if(buffer[i]<threshold)buffer[i]=0;
    }
    return 0;
}

template <class T>
int Image<T>::Threshold(double threshold,cv::Mat n, double mT, double nm)
{
    cerr<<"new threshold!\n";
    if(has_buffer)
    {
        #pragma omp parallel for
        for(unsigned long i=0; i<x*y; i++)
        {
          int iy=int(i/x);
          int ix=i-x*iy;
          double t=threshold;
          if(t<n.at<double>(iy,ix)*nm)
           t=n.at<double>(iy,ix)*nm;
          if(t<mT)
           t=mT;
          if(buffer[i]<t)buffer[i]=0;
        }
    }
    return 0;
}

template <class T>
vector<vector<double> > Image<T>::findBlob(double minArea,double maxArea, double minThreshold, double maxThreshold)
{
    unsigned char* tmpdata;
    tmpdata=new unsigned char[x*y];
    T minv=buffer[0];
    T maxv=buffer[0];
    for(unsigned long i=0; i<x*y; i++)
    {
        if(buffer[i]<minv)
        {
            minv=buffer[i];
        }
        if(buffer[i]>maxv)
        {
            maxv=buffer[i];
        }
    }
    if(!(minv>=0 && minv<=maxv && maxv<=255))
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=255-double(buffer[i]-minv)/double(maxv-minv)*255.;
    else
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=255-buffer[i];

    cv::Mat A = cv::Mat(y, x, CV_8UC1, &tmpdata[0]);

    cv::SimpleBlobDetector::Params params;
    params.minThreshold = minThreshold;
    params.maxThreshold = maxThreshold;
    params.filterByArea = true;
    params.minArea = minArea;
    params.maxArea = maxArea;
    params.filterByCircularity = false;
    params.minCircularity = 0.1;
    params.filterByConvexity = false;
    params.minConvexity = 0.87;
    params.filterByInertia = false;
    params.minInertiaRatio = 0.01;
    vector<cv::KeyPoint> keypoints;
#if CV_MAJOR_VERSION < 3
    cv::SimpleBlobDetector detector(params);
    detector.detect( A, keypoints);
#else
    cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);
    detector->detect( A, keypoints);
#endif
    delete[] tmpdata;

    vector< vector <double> > centers;
    for(unsigned long i = 0; i<keypoints.size(); i++)
    {
        vector<double> tmp;
        tmp.push_back(keypoints[i].pt.x);
        tmp.push_back(keypoints[i].pt.y);
        tmp.push_back(keypoints[i].size);
        centers.push_back(tmp);
    }
    return centers;
}

template <class T>
T* Image<T>::bufferPointer()
{
    return buffer;
}

template <class T>
void Image<T>::simpleDownScale(int xd, int yd)
{
    if(has_buffer)
    {
        T* tmp;
        int newx=x/xd;
        int newy=y/yd;
        tmp= new T[newx*newy];

        for(unsigned long j=0; j<y; j+=xd)
            for(unsigned long i=0; i<x; i+=yd)
                tmp[i/xd+newx*j/yd]=buffer[i+x*j];

        delete[] buffer;
        x=newx;
        y=newy;
        buffer=new T[x*y];
        for(unsigned long i=0; i<x*y; i++)
            buffer[i]=tmp[i];
        delete[] tmp;
    }
    else
    {
        cerr << "Error: Have no buffer to scale!"<<endl;
    }
}

template <class T>
int Image<T>::normalize(int minx, int maxx)
{
    T minv=buffer[0];
    T maxv=buffer[0];
    for(unsigned long i=0; i<x*y; i++)
    {
        if(buffer[i]<minv)
        {
            minv=buffer[i];
        }
        if(buffer[i]>maxv)
        {
            maxv=buffer[i];
        }
    }
    for(unsigned long i=0; i<x*y; i++)buffer[i]=double(buffer[i]-minv)/double(maxv-minv)*(maxx-minx)+minx;
    return 0;
}

//source: http://opencv-code.com/quick-tips/code-replacement-for-matlabs-bwareaopen/
template <class T>
vector<vector <double> >  Image<T>::removeSmallBlobs(double minsize,double maxsize)
{
    unsigned char* tmpdata;
    tmpdata=new unsigned char[x*y];
    T minv=buffer[0];
    T maxv=buffer[0];
    for(unsigned long i=0; i<x*y; i++)
    {
        if(buffer[i]<minv)
        {
            minv=buffer[i];
        }
        if(buffer[i]>maxv)
        {
            maxv=buffer[i];
        }
    }
    if(!(minv>=0 && minv<=maxv && maxv<=255))
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=double(buffer[i]-minv)/double(maxv-minv)*255.;
    else
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=buffer[i];

    cv::Mat A = cv::Mat(y, x, CV_8UC1, &tmpdata[0]);
    vector<std::vector<cv::Point> > contours;
    findContours(A.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    vector< vector <double> > blobs;
    cv::Mat B;
    A.copyTo(B);

    for (unsigned long i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);

        if (area > 0 && (area <= minsize || area >=maxsize))
        {
            cv::drawContours(A, contours, i, CV_RGB(0,0,0), -1);
        }
        else if(area!=0)
        {
            cv::drawContours(B, contours, i, CV_RGB(255,255,255), 10);
            cv::Moments mu=moments(contours[i],false);
            double xp=mu.m10/mu.m00;
            double yp=mu.m01/mu.m00;
            cv::Mat m( y, x, CV_8UC1, cv::Scalar(0) );
            cv::drawContours(m, contours, i, cv::Scalar(255), cv::FILLED);
            double minVal, maxVal;
            cv::Point minLoc, maxLoc;
            cv::minMaxLoc(A,&minVal,&maxVal,&minLoc,&maxLoc, m);
            vector<double> tmp;
            tmp.push_back(xp);
            tmp.push_back(yp);
            tmp.push_back(area);
            tmp.push_back(maxVal);
            blobs.push_back(tmp);
        }
    }
    if(false)
    {
        for(unsigned long i=0; i<x*y; i++)
            if(tmpdata[i]==0)
                buffer[i]=0;
        if(haveWindow==0)
        {
            cv::namedWindow("View");
            haveWindow=1;
        }
        cv::imshow("View", B);
        cv::waitKey(20);
    }
    delete[] tmpdata;

    return blobs;
}

//source: http://opencv-code.com/quick-tips/code-replacement-for-matlabs-bwareaopen/
template <class T>
vector<vector <double> >  Image<T>::removeSmallBlobs(double minsize,double maxsize,T minV, T maxV, bool filterByArea/*=false*/)
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
    cv::Mat A = cv::Mat(y, x, CV_8UC1, &tmpdata[0]);
    vector<std::vector<cv::Point> > contours;
    cv::findContours(A.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    vector< vector <double> > blobs;
    for (unsigned long i = 0; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        if ((area > 0 && (area <= minsize || area >=maxsize)) && filterByArea)
        {
            cv::drawContours(A, contours, i, CV_RGB(0,0,0), -1);
        }
        else if(area!=0)
        {
            cv::Moments mu=cv::moments(contours[i],false);
            double xp=mu.m10/mu.m00;
            double yp=mu.m01/mu.m00;
            cv::Mat m( y, x, CV_8UC1, cv::Scalar(0) );
            cv::drawContours(m, contours, i, cv::Scalar(255), cv::FILLED);
            double minVal, maxVal;
            cv::Point minLoc, maxLoc;
            cv::minMaxLoc(A,&minVal,&maxVal,&minLoc,&maxLoc, m);
            vector<double> tmp;
            tmp.push_back(xp);
            tmp.push_back(yp);
            tmp.push_back(area);
            tmp.push_back(maxVal/scale+offset);
            double ratio;
            double perimeter = cv::arcLength(cv::Mat(contours[i]), true);
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
            vector < cv::Point > hull;
            cv::convexHull(cv::Mat(contours[i]), hull);
            double hullArea = cv::contourArea(cv::Mat(hull));
            ratio = area / hullArea;
            tmp.push_back(ratio);
            vector<double> dists;
            for (size_t j = 0; j < contours[i].size(); j++)
            {
                cv::Point2d pt = contours[i][j];
                dists.push_back(cv::norm(cv::Point2d(xp,yp) - pt));
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

template <class T>
vector<vector <double> >  Image<T>::removeSmallBlobsNew(double minsize,double maxsize,T minV, T maxV)
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
    cv::Mat A = cv::Mat(y, x, CV_8UC1, &tmpdata[0]);
    cv::Mat labels;
    cv::Mat stats;
    cv::Mat centroids;
    int nLabels = cv::connectedComponentsWithStats(A, labels, stats, centroids, 8, CV_32S);
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
       cv::Moments mu = cv::moments(objImg, true);
       double xp=centroids.at<double>(i,0);
       double yp=centroids.at<double>(i,1);
       double minVal, maxVal;
       cv::Point minLoc, maxLoc;
       cv::minMaxLoc(A(roi),&minVal,&maxVal,&minLoc,&maxLoc, objImg);
       vector<double> tmp;
       tmp.push_back(xp);
       tmp.push_back(yp);
       tmp.push_back(area);
       tmp.push_back(maxVal/scale+offset);
       vector<std::vector<cv::Point> > contours;
       findContours(objImg.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
       if(contours.size()==1 && contours[0].size()>2)
       {
       double ratio;
       double perimeter = arcLength(cv::Mat(contours[0]), true);
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
        vector < cv::Point > hull;
        convexHull(cv::Mat(contours[0]), hull);
        double hullArea = cv::contourArea(cv::Mat(hull));
        ratio = area / hullArea;
        tmp.push_back(ratio);
        vector<double> dists;
        for (size_t j = 0; j < contours[0].size(); j++)
        {
         cv::Point2d pt = contours[0][j];
         pt +=cv::Point2d(bb_left,bb_top);
         dists.push_back(cv::norm(cv::Point2d(xp,yp) - pt));
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

//source: http://opencv-code.com/quick-tips/code-replacement-for-matlabs-bwareaopen/
template <class T>
vector< std::vector<cv::Point> >  Image<T>::removeSmallBlobsContours(double minsize,double maxsize,T minV, T maxV)
{
    unsigned char* tmpdata;
    tmpdata=new unsigned char[x*y];
    if(minV>=0 && maxV<=255)
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=buffer[i];
    else
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=double(buffer[i]-minV)/double(maxV-minV)*255.;

    cv::Mat A = cv::Mat(y, x, CV_8UC1, &tmpdata[0]);
    vector<std::vector<cv::Point> > contours;
    vector<std::vector<cv::Point> > tmp;
    findContours(A.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (int i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        if( area > minsize && area <maxsize)
        {
            tmp.push_back(contours[i]);
        }
    }
    delete[] tmpdata;
    return tmp;
}

template <class T>
cv::Mat Image<T>::image(int n /*=0*/)
{
if(has_buffer)
{
    unsigned char* tmpdata;
    tmpdata=new unsigned char[x*y];
    T minv=buffer[0];
    T maxv=buffer[0];
    for(unsigned long i=0; i<x*y; i++)
    {
        if(buffer[i]<minv)
        {
            minv=buffer[i];
        }
        if(buffer[i]>maxv)
        {
            maxv=buffer[i];
        }
    }
    if(n==1)
    {
     minv=0;
     maxv=255;
     for(unsigned long i=0; i<x*y; i++)
     {
     double tmp=double(buffer[i]-minv)/double(maxv-minv)*255.;
     if (tmp>255)tmp=255;
     if(tmp<0) tmp=0;
     tmpdata[i]=tmp;
     }
    }
    else if(n==2)
    {
     maxv=255;
     minv=0;
     for(unsigned long i=0; i<x*y; i++)
     {
     double tmp=double(-buffer[i]-minv)/double(maxv-minv)*255.;
     if (tmp>255)tmp=255;
     if(tmp<0) tmp=0;
     tmpdata[i]=tmp;
     }
    }
    else
    {
    if(!(minv>=0 && minv<=maxv && maxv<=255))
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=double(buffer[i]-minv)/double(maxv-minv)*255.;
    else
        for(unsigned long i=0; i<x*y; i++)tmpdata[i]=buffer[i];
    }
    cv::Mat A = cv::Mat(y, x, CV_8UC1, &tmpdata[0]);
    cv::Mat tmp;
    A.copyTo(tmp);
    delete[] tmpdata;
    return tmp;
}
else
{
    cv::Mat tmp;
    return tmp;
}
}

template <class T>
cv::Mat Image<T>::image(cv::Rect roi)
{
if(has_buffer && roi.x>=0 && roi.x<x && roi.y>=0 && roi.y<y && 
   roi.x+roi.width<x && roi.y+roi.height<y && roi.height>0 && roi.width>0 )
{
    unsigned char* tmpdata;
    tmpdata=new unsigned char[roi.width*roi.height];
    T minv=buffer[0];
    T maxv=buffer[0];
    for(unsigned long i=0; i<x*y; i++)
    {
        if(buffer[i]<minv)
        {
            minv=buffer[i];
        }
        if(buffer[i]>maxv)
        {
            maxv=buffer[i];
        }
    }
    if(!(minv>=0 && minv<=maxv && maxv<=255))
    {
        for(unsigned long i=0; i<roi.width*roi.height; i++)
        {
            int iy=int(i/roi.width);
            int ix=i-roi.width*iy;
            tmpdata[ix+roi.width*iy]=double(buffer[ix+roi.x+x*(iy+roi.y)]-minv)/double(maxv-minv)*255.;
        }
    }
    else
    {
        for(unsigned long i=0; i<roi.width*roi.height; i++)
        {
            int iy=int(i/roi.width);
            int ix=i-roi.width*iy;
            tmpdata[ix+roi.width*iy]=buffer[ix+roi.x+x*(iy+roi.y)];
        }
    }
    cv::Mat A = cv::Mat(roi.height, roi.width, CV_8UC1, &tmpdata[0]);
    cv::Mat tmp;
    A.copyTo(tmp);
    delete[] tmpdata;
    return tmp;
}
else
{
    cerr<<"Something is wrong, I'm not cutting roi from image..\n";
    cv::Mat tmp;
    return tmp;
}
}

template <class T>
int Image<T>::erodeAndDilate(int i, int j)
{
  cv::Mat A = cv::Mat(y, x, CV_16S, &buffer[0]);
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

template <class T>
template <class S>
int Image<T>::insert(Image<S>& a,int ofx, int ofy)
{
    if( a.hasBuffer()==1 && has_buffer==1)
    {
        int ix=a.getX();
        int iy=a.getY();
        if(ix+ofx <= x && iy+ofy <= y)
        {
            for(unsigned long i=0; i<ix*iy; i++)
            {
                int ty=int(i/ix);
                int tx=i-ix*ty;
                buffer[(tx+ofx)+x*(ty+ofy)]=a.getValue(tx,ty);
            }

        }
        else{
            cerr<<"Image of size "<<a.getX()<<" x "<<a.getY()<<" does not fit into "<<x <<"x"<< y<<" with offset "<<ofx<<","<<ofy<<"\n";
        }
    }
    else
     cerr<<"Image has no data\n";
    
    return 0;
}

template <class T>
template <class S>
int Image<T>::insert(Image<S>* a,int ofx, int ofy)
{
    if( a->hasBuffer()==1 && has_buffer==1)
    {
    int ix=a->getX();
    int iy=a->getY();
        if(ix+ofx <= x && iy+ofy <= y)
        {
            for(unsigned long i=0; i<ix*iy; i++)
            {
                int ty=int(i/ix);
                int tx=i-ix*ty;
                buffer[(tx+ofx)+x*(ty+ofy)]=a->getValue(tx,ty);
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

template <class T>
template <class S>
int Image<T>::copyTo(Image<S>& a)
{
    if(has_buffer==1)
    {
    if(a.has_buffer==1)
    {
     if(!(a.x==x && a.y==y))
     {
      a.create(x,y);
     }
    }
    else
    {
     a.create(x,y);    
    }
    for(unsigned long i=0; i<x*y; i++)
      a.buffer[i]=(S)buffer[i];
    } 
    return 0;
}

template <class T>
template <class S>
int Image<T>::copyTo(Image<S>* a)
{
    if(has_buffer==1)
    {
    if(a->has_buffer==1)
    {
     if(!(a->x==x && a->y==y))
     {
      a->create(x,y);
     }
    }
    else
    {
     a->create(x,y);    
    }
    for(unsigned long i=0; i<x*y; i++)
      a->buffer[i]=(S)buffer[i];
    }
    return 0;
}

template <class T>
bool Image<T>::addOneInPoly(vector<cv::Point2f> poly)
{
    if(has_buffer==1 && poly.size()>2)
    {
      float minx=poly[0].x;
      float miny=poly[0].y;
      float maxx=poly[0].x;
      float maxy=poly[0].y;
      for(unsigned int i=1; i<poly.size();i++)
      {
         if(minx>poly[i].x) minx=poly[i].x;
         if(miny>poly[i].y) miny=poly[i].y;
         if(maxx<poly[i].x) maxx=poly[i].x;
         if(maxy<poly[i].y) maxy=poly[i].y;
      }
      int ax=minx;
      int ay=miny;
      int bx=maxx+2;
      int by=maxy+2;
      if(ax<0) ax=0;
      if(ay<0) ay=0;
      if(bx>=x) bx=x;
      if(by>=y) by=y;
      int cx=bx-ax;
      int cy=by-ay;
      
      if(cx>0 && cy>0)
      {      
      #pragma omp parallel for
      for(unsigned long i=0; i<cx*cy; i++)
      {
       int iy=int(i/cx);
       int ix=i-cx*iy;
       ix+=ax;
       iy+=ay;
       if(isInPoly(ix,iy,poly))
          buffer[ix+iy*x]++;
      }
      }
      else
      {
       cerr<<"I got negative width or hight!\n";
      }
    }
    else
    {
      cerr<<"Image has no data\n";    
    }
    return true;
}

template <class T>
bool Image<T>::isInPoly(int ix, int iy,vector<cv::Point2f> poly)
{
  int i, j, nvert = poly.size();
  bool c = false;
  for(i = 0, j = nvert - 1; i < nvert; j = i++) {
    if( ( (poly[i].y >= iy ) != (poly[j].y >= iy) ) &&
        (ix <= (poly[j].x - poly[i].x) * (iy - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x)
      )
      c = !c;
  }
  return c; 
}

template <class T>
vector<int> Image<T>::polyBin(vector<cv::Point2f> poly)
{
    vector<int> bin;
    int n0=0;
    int n1=0;
    if(has_buffer==1 && poly.size()>2)
    {
      float minx=poly[0].x;
      float miny=poly[0].y;
      float maxx=poly[0].x;
      float maxy=poly[0].y;
      for(unsigned int i=1; i<poly.size();i++)
      {
         if(minx>poly[i].x) minx=poly[i].x;
         if(miny>poly[i].y) miny=poly[i].y;
         if(maxx<poly[i].x) maxx=poly[i].x;
         if(maxy<poly[i].y) maxy=poly[i].y;
      }
      int ax=minx;
      int ay=miny;
      int bx=maxx+2;
      int by=maxy+2;
      if(ax<0) ax=0;
      if(ay<0) ay=0;
      if(bx>=x) bx=x;
      if(by>=y) by=y;
      int cx=bx-ax;
      int cy=by-ay;
      if(cx>0 && cy>0)
      {      
      #pragma omp parallel for reduction(+:n0,n1)
      for(unsigned long i=0; i<cx*cy; i++)
      {
       int iy=int(i/cx);
       int ix=i-cx*iy;
       ix+=ax;
       iy+=ay;
       if(isInPoly(ix,iy,poly))
       {
        if(buffer[ix+iy*x]==0)
        {
         n0++;
        }
        else
        {
         n1++;
        }
       }
      }
      
      bin.push_back(n0);
      bin.push_back(n1);
      }
      else
      {
       cerr<<"I got negative width or hight!\n";      
      }
    }
    else
    {
      cerr<<"Image has no data\n";    
    }
    return bin;
}

template <class T>
bool Image<T>::copyFrom(cv::Mat A)
{
    unsigned long xp=A.cols;
    unsigned long yp=A.rows;
    cerr<<"Resize image from "<<x<<"x"<<y <<" to "<<xp <<"x"<<yp<<"\n";
    if(!(x==xp && y==yp && has_buffer==1))
    {
    x=xp;
    y=yp;
    if(has_buffer==1)
     delete[] buffer;
    buffer=new T[x*y];
    has_buffer=1;
    }
    for(unsigned long i=0; i<x; i++)
        for(unsigned long j=0; j<y; j++)
        {
            buffer[i+x*j]=A.at<uint8_t>(j,i);
        }
}

template <class T>
vector<double> Image<T>::getAverage()
{
    vector<double> tmp;
    if(has_buffer)
    {
        double avg=0;
        for(unsigned int i=0; i<x; i++)
            for(unsigned int j=0; j<y; j++)
        {
                avg+=(double)buffer[i+x*j];
        }
        avg/=x*y;
        double stdv=0;
        for(unsigned int i=0; i<x; i++)
            for(unsigned int j=0; j<y; j++)
        {
                stdv+=pow((double)buffer[i+x*j]-avg,2);
        }
        stdv=sqrt(stdv/(x*y));
        tmp.push_back(avg);
        tmp.push_back(stdv);
    }
    return tmp;
}

template<class T>
int Image<T>::loadROIFromSeq(fstream *seqFile, Header header, unsigned int i, int xmin, int ymin, int xmax,int ymax)
{
if(xmin<xmax && ymin<ymax && xmin>=0 && xmin<header.width() && ymin>=0 && ymin<header.height() &&
   xmax>0 && xmax<=header.width() && ymax>0 && ymax<=header.height())
{
    if(!has_buffer)
        create(xmax-xmin,ymax-ymin);
    else
    {
        if(!(x==xmax-xmin && y==ymax-ymin))
        {
            x=xmax-xmin;
            y=ymax-ymin;
            delete[] buffer;
            buffer= new unsigned char[x*y];
        }
    }

    offx=xmin;
    offy=ymin;
    
    if(seqFile->is_open())
    {
        for(int line=0; line<y; line++)
        {
        seqFile->seekg(header.imageStartOffset(i)+header.width()*(line+ymin)+xmin, seqFile->beg);
        seqFile->read((char*)buffer+x*line, sizeof(unsigned char)*x);
        }
        seqFile->seekg(header.imageStartOffset(i)+header.width()*header.height(), seqFile->beg);                
        if(header.streampix6())
        {
            seqFile->read(reinterpret_cast<char*>(&sec), sizeof(unsigned int));
            seqFile->read(reinterpret_cast<char*>(&ms), sizeof(unsigned short));
            seqFile->read(reinterpret_cast<char*>(&us), sizeof(unsigned short));
            pts=sec+ms/1000.+us/1000000.;
        }
        else
        {
            seqFile->read(reinterpret_cast<char*>(&sec), sizeof(unsigned int));
            seqFile->read(reinterpret_cast<char*>(&ms), sizeof(unsigned short));
            seqFile->read(reinterpret_cast<char*>(&us), sizeof(unsigned short));
            pts=sec+ms/1000.+us/1000000.;
        }
    }
    return 0;
}
else
{
    cerr<<"Not valid roi selection!!!"<<endl;
    return 0;
}
}

template <class T>
Image<double> Image<T>::SobelMagnitude()
{
if(has_buffer)
{
    Image<int> sx;
    sx.create(x,y);
    for(int j=0;j<y;j++)
     for(int i=0;i<x;i++)
      sx.setValue(i,j,buffer[i+x*j]);
    sx.linYFiltI(1,2,1);
    sx.linXFiltI(-1,0,1);
    Image<int> sy;
    sy.create(x,y);
    for(int j=0;j<y;j++)
     for(int i=0;i<x;i++)
      sy.setValue(i,j,buffer[i+x*j]);
    sy.linXFiltI(1,2,1);
    sy.linYFiltI(-1,0,1);
    Image<double> tmp=sx.magWith(sy);
    return tmp;
}
else
{
    Image<double> tmp;
    return tmp;
}
}

template <class T>
int Image<T>::linXFiltI(int f0, int f1, int f2)
{
if(has_buffer)
{
T t1,t2;
for(int j=0; j<y;j++)
{
t1=buffer[j*x];
for(int i=1; i<x-1; i++)
{
 t2=buffer[i+j*x];
 buffer[i+j*x]=t1*f0+t2*f1+buffer[i+1+j*x]*f2;
 t1=t2;
}
}
return 0;
}
else
{
return -1;
}
}

template <class T>
int Image<T>::linYFiltI(int f0, int f1, int f2)
{
if(has_buffer)
{
T t1,t2;
for(int i=0; i<x;i++)
{
t1=buffer[i];
for(int j=1; j<y-1; j++)
{
 t2=buffer[i+j*x];
 buffer[i+j*x]=t1*f0+t2*f1+buffer[i+(j+1)*x]*f2;
 t1=t2;
}
}
return 0;
}
else
{
return -1;
}
}

template<class T>
template<class S> 
Image<double> Image<T>::magWith(Image<S> a)
{
if(has_buffer && a.hasBuffer() && x== a.getX() && y==a.getY())
{
Image<double> tmp;
tmp.create(x,y);
for(int i=1; i<x-1; i++)
 for(int j=1; j<y-1; j++)
  tmp.setValue(i,j,sqrt(pow(buffer[i+j*x],2)+pow(a.getValue(i,j),2)));
return tmp;
}
else
{
 Image<double> tmp;
 return tmp;
}
}

template <class T>
T Image<T>::getMaxValue()
{
    T tmp=0;
    if(has_buffer)
    {
     tmp=buffer[0];
     for(int j=0;j<y;j++)
      for(int i=0;i<x;i++)
       if(buffer[i+x*j]>tmp)
        tmp=buffer[i+x*j];
    }
    return tmp;
}
template <class T>
T Image<T>::getMinValue()
{
    T tmp=0;
    if(has_buffer)
    {
     tmp=buffer[0];
     for(int j=0;j<y;j++)
      for(int i=0;i<x;i++)
       if(buffer[i+x*j]<tmp)
        tmp=buffer[i+x*j];
    }
    return tmp;
}

template <class T>
void Image<T>::medianBlur(int k)
{
 cerr<<"Warning: Using general function transforming image to 8U and do medianBlur there...\n";
 unsigned char* tmpdata;
 tmpdata=new unsigned char[x*y];
 #pragma omp parallel for
 for(unsigned long i=0; i<x*y; i++)
  for(unsigned long j=0; j<y; j++)
  {
   if(buffer[i]<0)
   {
    tmpdata[i]=0;   
   }
   else if(buffer[i]>255)
   {
    tmpdata[i]=255;
   }
   else
   {
    tmpdata[i]=(unsigned int) buffer[i];
   }
  }
 cv::Mat A = cv::Mat(y, x, CV_8UC1, &tmpdata[0]);
 cv::Mat B;
 cv::medianBlur(A,B,k);
 B.copyTo(A);
 #pragma omp parallel for
 for(unsigned long i=0; i<x*y; i++)
   buffer[i]=(T) tmpdata[i];
}

template <class T>
void Image<T>::gaussianBlur(int k, double s)
{
 cerr<<"ERROR: gaussianBlur only defined for short and unsigned char...\n";
}

//=> https://stackoverflow.com/questions/11716268/point-in-polygon-algorithm
template <class T>
bool Image<T>::isInsidePoly(int x, int y, vector<cv::Point> ppp) {
 if(ppp.size()<3) return false;
  int i, j, nvert = ppp.size();
  bool c = false;
  for(i = 0, j = nvert - 1; i < nvert; j = i++) {
    if( ( (ppp[i].y >= y ) != (ppp[j].y >= y) ) &&
        (x <= (ppp[j].x - ppp[i].x) * (y - ppp[i].y) / (ppp[j].y - ppp[i].y) + ppp[i].x)
        )
      c = !c;
  }
  return c;
}

template <class T>
void Image<T>::Abs()
{
  if(has_buffer==1)
  {
    #pragma omp parallel for
    for(unsigned int i=0; i<x*y; i++)
    {
      buffer[i]=abs(buffer[i]);
    }
  }
}

template <class T>
void Image<T>::bwareaopen(int lowerCutOff, int upperCutOff)
{
  if(has_buffer==1)
  {
    unsigned char* tmpdata;
    tmpdata=new unsigned char[x*y];
    #pragma omp parallel for
    for(unsigned long i=0; i<x*y; i++)
    {
     if(buffer[i]==0)
      tmpdata[i]=0;
     else
      tmpdata[i]=255;
    }
    cv::Mat A = cv::Mat(y, x, CV_8UC1, &tmpdata[0]);
    cv::Mat labels;
    cv::Mat stats;
    cv::Mat centroids;
    int nLabels = connectedComponentsWithStats(A, labels, stats, centroids, 8, CV_32S);
    #pragma omp parallel for
    for(unsigned long i=0; i<x*y; i++)
    {
      int iy=int(i/x);
      int ix=i-x*iy;
      int label=labels.at<int>(iy,ix);
      int area=stats.at<int>(label,cv::CC_STAT_AREA);
      if(area<lowerCutOff){buffer[i]=0;}                 
      if(area>upperCutOff){buffer[i]=0;}                 
    }
    delete[] tmpdata;
  }
}

template<class T>
int Image<T>::saveToSeq(fstream& seqFile, Header header)
{
    if(has_buffer)
    {
        if(seqFile)
        {
            seqFile.seekp(header.getOffset(), seqFile.beg);
            seqFile.write((char*)buffer, sizeof(unsigned char)*header.height()*header.width());
            seqFile.write(reinterpret_cast<char*>(&sec), sizeof(unsigned int));
            seqFile.write(reinterpret_cast<char*>(&ms), sizeof(unsigned short));
            if(header.streampix6())
                seqFile.write(reinterpret_cast<char*>(&us), sizeof(unsigned short));
            unsigned char x=0;
            seqFile.seekp(header.getOffset()+header.trueImageSize()-1, seqFile.beg);
            seqFile.write(reinterpret_cast<char*>(&x), sizeof(unsigned char));
        }
    }
    else
        cerr << "Error: Image has no DATA"<<endl;
    return 0;
}

template<class T>
int Image<T>::saveToSeq(fstream* seqFile, Header header, unsigned int i)
{
    if(has_buffer)
    {
        if(seqFile->is_open())
        {
            seqFile->seekp(header.imageStartOffset(i), seqFile->beg);
            seqFile->write((char*)buffer, sizeof(unsigned char)*header.height()*header.width());
            seqFile->write(reinterpret_cast<char*>(&sec), sizeof(unsigned int));
            seqFile->write(reinterpret_cast<char*>(&ms), sizeof(unsigned short));
            if(header.streampix6())
                seqFile->write(reinterpret_cast<char*>(&us), sizeof(unsigned short));
            unsigned char x=0;
            seqFile->seekp(header.imageStartOffset(i)+header.trueImageSize()-1, seqFile->beg);
            seqFile->write(reinterpret_cast<char*>(&x), sizeof(unsigned char));
        }
        else
        cerr << "Error: No seqFile"<<endl;        
    }
    else
        cerr << "Error: Image has no DATA"<<endl;
    return 0;
}


template <>
template <>
int Image<unsigned char>::insert(Image<unsigned char>& a,int ofx, int ofy);

template <>
template <>
int Image<unsigned char>::insert(Image<unsigned char>* a,int ofx, int ofy);

template<>
int Image<unsigned char>::loadFromSeq(fstream *seqFile, Header header, unsigned long i);
template <>
int Image<unsigned char>::save(string filename);

template <>
vector<vector<int> > Image<short>::dHistogram(int* maxi);

template <>
vector<vector<int> > Image<double>::dHistogram(int* maxi);

template <>
vector<vector<int> > Image<unsigned char>::dHistogram(int* maxi);

template <>
vector<vector <double> >  Image<short>::removeSmallBlobs(double minsize,double maxsize,short minV,short maxV,bool filterByArea);

template <>
vector<vector <double> >  Image<short>::removeSmallBlobsNew(double minsize,double maxsize,short minV,short maxV);

template <>
vector<std::vector<cv::Point> >  Image<short>::removeSmallBlobsContours(double minsize,double maxsize,short minV,short maxV);

template <>
void Image<unsigned char>::medianBlur(int k);

template <>
void Image<double>::medianBlur(int k);

template <>
void Image<short>::medianBlur(int k);

template <>
void Image<unsigned char>::gaussianBlur(int k,double s);

template <>
void Image<double>::gaussianBlur(int k,double s);

template <>
void Image<short>::gaussianBlur(int k, double s);

template <>
int Image<double>::erodeAndDilate(int i, int j);

template <>
int Image<unsigned char>::erodeAndDilate(int i, int j);

#endif
