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
#include "MosquitoPosition.hpp"
#include "MosquitoesInFrame.hpp"
#include <vector>
#include <iostream>
#include <iomanip> 
#include <algorithm>
#include <cmath>
#include <QPointF>
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

MosquitoesInFrame::MosquitoesInFrame()
{
    maxDiff=0;
    frameNo=0;
    processed=0;
    ith=0;
    minTh=0;
    th=0;
    minA=0;
    maxA=0;
    tsSec=0;
    tsMSec=0;
    tsUSec=0;
}

MosquitoesInFrame::MosquitoesInFrame(const MosquitoesInFrame& a)
{
    processed=a.processed;
    maxDiff=a.maxDiff;
    frameNo=a.frameNo;
    ith=a.ith;
    minTh=a.minTh;
    th=a.th;
    minA=a.minA;
    maxA=a.maxA;
    mpos=a.mpos;
    tsSec=a.tsSec;
    tsMSec=a.tsMSec;
    tsUSec=a.tsUSec;
}

int MosquitoesInFrame::addMosquito(MosquitoPosition impos,int ofx, int ofy)
{
    impos.move(ofx,ofy);
    mpos.push_back(impos);
    return 0;
}

int MosquitoesInFrame::addMosquito(double ix, double iy, double iarea, double imaxInt, double circularity, double inertia, double convexity, double radius, vector<cv::Point> c /* =vector<cv::Point>() */)
{
    MosquitoPosition tmp(ix, iy, iarea, imaxInt, circularity, inertia, convexity, radius,c);
    mpos.push_back(tmp);
    if(iarea==-1 && imaxInt==-1)
       cout << "Adding point at : " << ix<<" "<<iy<<endl;    
    return 0;
}

int MosquitoesInFrame::addMosquito(double ix, double iy, double iarea, double imaxInt)
{
    MosquitoPosition tmp(ix, iy, iarea, imaxInt, 0, 0, 0, 0);
    mpos.push_back(tmp);
    if(iarea==-1 && imaxInt==-1)
       cout << "Adding point at : " << ix<<" "<<iy<<endl;    
    return 0;
}

int MosquitoesInFrame::removeMosquito(unsigned long i)
{
    if(i<mpos.size())
    {
        removeAndClean(i);
    }
    return 0;
}

int MosquitoesInFrame::removeMosquito(double ix, double iy, int searchRadius /*=8*/ , bool onlyOne /*=true*/)
{
    int n=-1;
    double mindist=searchRadius;
    if(onlyOne)
    {
    for(int i=0; i<mpos.size();i++)
    {
      double dist=sqrt(pow(mpos[i].getX()-ix,2)+pow(mpos[i].getY()-iy,2));
      if(dist<mindist)
      {
        n=i;
        mindist=dist;
      }
    }
    
    if(n>=0 && n<mpos.size() && mindist<searchRadius)
    {
        removeAndClean(n);

    }
    }
    else
    {
    for(int i=0; i<mpos.size();i++)
    {
      double dist=sqrt(pow(mpos[i].getX()-ix,2)+pow(mpos[i].getY()-iy,2));
      if(dist<mindist)
      {
        removeAndClean(i);
        i--;
      }
    }
    }
    return 0;
}

int MosquitoesInFrame::removeAllMosquito()
{
    vector<MosquitoPosition>().swap(mpos);
    return 0;
}

MosquitoPosition* MosquitoesInFrame::mosqi(unsigned long i)
{
    MosquitoPosition* tmp=nullptr;
    if(i<mpos.size())
    {
        tmp = &mpos[i];
    }
    return tmp;
}

MosquitoPosition* MosquitoesInFrame::mosqi(double ix, double iy, double searchRadius /*=3*/)
{
    MosquitoPosition* tmp=nullptr;
    int n=-1;
    double mindist=searchRadius;
    for(int i=0; i<mpos.size();i++)
    {
      double dist=sqrt(pow(mpos[i].getX()-ix,2)+pow(mpos[i].getY()-iy,2));
      if(dist<mindist)
      {
        n=i;
        mindist=dist;
      }
    }
    if(n>=0 && n<mpos.size() && mindist<searchRadius)
     tmp = &mpos[n];
    return tmp;
}

int MosquitoesInFrame::mosqIndex(double ix, double iy)
{
    int n=-1;
    double mindist=1000000;
    for(int i=0; i<mpos.size();i++)
    {
      double dist=sqrt(pow(mpos[i].getX()-ix,2)+pow(mpos[i].getY()-iy,2));
      if(dist<mindist)
      {
        n=i;
        mindist=dist;
      }
    }
    if(n>=0 && n<mpos.size() && mindist<3)
     return n;
    else
     return -1;
}

bool MosquitoesInFrame::getNearest(double ix, double iy, double searchRadius,int &j, double &odist)
{
    int n=-1;
    double minDist=searchRadius;
    if(minDist<=0) minDist=8;
    for(int i=0; i<mpos.size();i++)
    {
      double dist=sqrt(pow(mpos[i].getX()-ix,2)+pow(mpos[i].getY()-iy,2));
      if(dist<minDist)
      {
       minDist=dist;
       n=i;
      }
    }
    if(n>-1)
    {
     j=n;
     odist=minDist;
     return true; 
    }
    else
     return false;
}

int MosquitoesInFrame::clear()
{
    maxDiff=0;
    frameNo=0;
    processed=0;
    ith=0;
    minTh=0;
    th=0;
    minA=0;
    maxA=0;
    mpos.clear();
    return 0;
}

int MosquitoesInFrame::clean(double minDist)
{
int removed=0;
vector<unsigned long> markForDeletion;
if(mpos.size()>1)
{
for(int i=0; i<mpos.size()-1;i++)
{
vector<unsigned long> cluster;
for(int j=i+1; j<mpos.size(); j++)
 if(sqrt(pow(mpos[j].getX()-mpos[i].getX(),2)+
    pow(mpos[j].getY()-mpos[i].getY(),2))<minDist)
  cluster.push_back(j);
double maxInt=mpos[i].getI();
unsigned long iMaxInt=i;
for(int j=0; j<cluster.size();j++)
 if(maxInt<mpos[cluster[j]].getI() || (maxInt==mpos[cluster[j]].getI() && mpos[cluster[j]].getA()>mpos[iMaxInt].getA()))
 {
 maxInt=mpos[cluster[j]].getI();
 iMaxInt=cluster[j];
 }
if(iMaxInt!=i)
 markForDeletion.push_back(i);
for(int j=0; j<cluster.size();j++)
 if(cluster[j]!=iMaxInt)
  markForDeletion.push_back(cluster[j]);
}
sort( markForDeletion.begin(), markForDeletion.end() );
markForDeletion.erase( unique( markForDeletion.begin(), markForDeletion.end() ), markForDeletion.end() );
for(int i=markForDeletion.size()-1; i>=0; i--)
{
  mpos[markForDeletion[i]] = mpos.back();
  mpos.pop_back();
  removed++;
}
}
return removed;
}

int MosquitoesInFrame::clean(double minArea,double maxArea)
{
int removed=mpos.size();
mpos.erase(std::remove_if(mpos.begin(), mpos.end(), 
                [&minArea,&maxArea](MosquitoPosition& x){
                    if (x.getA()<minArea || x.getA()>maxArea)
                        return true;
                    else
                        return false;
                }), mpos.end());
removed -= mpos.size();  
return removed;
}


int MosquitoesInFrame::appendToBackup(ostream* o)
{
        if(o->bad())
        {
            cerr << "Error backup file not opened!"<<endl;
        }
        else
        {
            MosquitoPosition* currentMosquito;
            for(int j=0; j<numOfMosquitoes(); j++)
            {
                currentMosquito=mosqi(j);
                int flag=currentMosquito->getFlag();
                *o<<std::setprecision(16) <<std::fixed
                  <<currentMosquito->getX()<<'\t'
                  <<currentMosquito->getY()<<'\t'
                  <<currentMosquito->getI()<<'\t'
                  <<currentMosquito->getA()<<'\t'
                  <<getMaxDiff()<<'\t'
                  <<static_cast<double>(getF())<<'\t'
                  <<static_cast<double>(getTsSec())<<'\t'
                  <<static_cast<double>(getTsMSec())<<'\t'
                  <<static_cast<double>(getTsUSec())<<'\t'
                  <<static_cast<double>(flag)<<'\n';
                 if(currentMosquito->MosquitoHasContour())
                 {
                  vector<cv::Point> c=currentMosquito->getContour();
                  *o<<"#contour "<<c.size();
                  for(int cc=0; cc<c.size(); cc++)
                   *o<<" "<<c[cc].x<<" "<<c[cc].y;
                  *o<<"\n";
                 }
            }
            o->flush();
        }
    return 0;
}

int MosquitoesInFrame::removePointsInsideOfPolygon(vector<QPointF> poly, bool inverse /* = false */)
{
    int num=0;
    for(int i=0; i<mpos.size();i++)
    {
      if(!inverse)
      {
       if(PointInPolygon(mpos[i],poly))
       {
        num++;
        removeAndClean(i);
        i--;
       }
      }
      else
      {
       if(!PointInPolygon(mpos[i],poly))
       {
        num++;
        removeAndClean(i);
        i--;
       }
      }
    }
    return num; 
}

vector<cv::Point2f> MosquitoesInFrame::pointsInsideOfPolygon(vector<QPointF> poly)
{
    vector<cv::Point2f> listOfPoints; 
    int num=0;
    for(int i=0; i<mpos.size();i++)
    {
      if(PointInPolygon(mpos[i],poly))
      {
       listOfPoints.push_back(cv::Point2f(mpos[i].getX(),mpos[i].getY()));
      }
    }
    return listOfPoints; 
}

vector<vector<double> > MosquitoesInFrame::pointsInsideOfPolygonAsVector(vector<QPointF> poly)
{
    vector<vector<double> > listOfPoints; 
    int num=0;
    for(int i=0; i<mpos.size();i++)
    {
      //inside polygon?
      if(PointInPolygon(mpos[i],poly))
      {
        vector<double> tmp2;
        tmp2.push_back(mpos[i].getX());
        tmp2.push_back(mpos[i].getY());
        tmp2.push_back((double)frameNo);
        tmp2.push_back((double)getTsSec());            
        tmp2.push_back((double)getTsMSec());            
        tmp2.push_back((double)getTsUSec()); 
        tmp2.push_back(mpos[i].getI());             
        tmp2.push_back(mpos[i].getA());
        tmp2.push_back(getMaxDiff());
        tmp2.push_back(getMinArea());
        tmp2.push_back(getMaxArea());
        tmp2.push_back(getThreshold());
        tmp2.push_back(getMinThreshold());
        tmp2.push_back((double)getIth());
        listOfPoints.push_back(tmp2);
      }
    }
    return listOfPoints; 
}

//=> https://stackoverflow.com/questions/11716268/point-in-polygon-algorithm
bool MosquitoesInFrame::PointInPolygon(MosquitoPosition m, vector<QPointF>  poly) {
  int i, j, nvert = poly.size();
  bool c = false;

  for(i = 0, j = nvert - 1; i < nvert; j = i++) {
    if( ( (poly[i].y() >= m.getY() ) != (poly[j].y() >= m.getY()) ) &&
        (m.getX() <= (poly[j].x() - poly[i].x()) * (m.getY() - poly[i].y()) / (poly[j].y() - poly[i].y()) + poly[i].x())
      )
      c = !c;
  }

  return c;
}

int MosquitoesInFrame::numOfMosquitoes()
{
   int n=0;
   for(int i=0; i<mpos.size(); i++)
   {
          n++;
   }
   return n;
};


bool MosquitoesInFrame::removeAndClean(int i)
{
    if(i<mpos.size())
    {
        mpos.erase(mpos.begin()+i);
        return true;
    }
   return false;
}

bool MosquitoesInFrame::setTime(vector<int> i)
{
 if(i.size()==3)
 {
   tsSec=i[0];
   tsMSec=i[1];
   tsUSec=i[2];
   return true;
 }
 else
  return false;
}

vector<int> MosquitoesInFrame::getTime()
{
  vector<int> tmp;
  tmp.push_back(tsSec);
  tmp.push_back(tsMSec);
  tmp.push_back(tsUSec);
  return tmp;
}

int MosquitoesInFrame::removeMarkedMosquitoes()
{
 for(int i=mpos.size()-1; i>=0; i--)
 {
  if(!(mpos[i].getFlag() & MosquitoPosition::flagType::KEEPME))
  {
        removeAndClean(i);  
  }
 }
 return 0;
}

int MosquitoesInFrame::cleanClusters(double minD, int minN)
{
 vector<vector<int> > clusters;
 for(int i=0; i<mpos.size();i++)
 {
  vector<int> possibleClusters;

  double xi=mpos[i].getX();
  double yi=mpos[i].getY();
  
  for(int j=0; j<clusters.size(); j++)
  {
   for(int k=0; k<clusters[j].size(); k++)
   {
    double dist=sqrt(pow(mpos[clusters[j][k]].getX()-xi,2)+pow(mpos[clusters[j][k]].getY()-yi,2));
    if(dist<minD)
    {
     possibleClusters.push_back(j);
     break;
    }
   }
  }
  if(possibleClusters.size()==0)
  {
   vector<int> tmp;
   tmp.push_back(i);
   clusters.push_back(tmp);
  }
  else if(possibleClusters.size()==1)
  {
   clusters[possibleClusters[0]].push_back(i);
  }
  else
  {
   sort( possibleClusters.begin(), possibleClusters.end() );
   for(int j=possibleClusters.size()-1; j>0; j--)
   {
    clusters[possibleClusters[0]].insert(clusters[possibleClusters[0]].end(), 
                                         clusters[possibleClusters[j]].begin(), 
                                         clusters[possibleClusters[j]].end());
    clusters.erase(clusters.begin()+possibleClusters[j]);
   }
   clusters[possibleClusters[0]].push_back(i);   
  }
 }
 cerr<<"I have put "<<mpos.size()<<" positions into "<<clusters.size()<<" clusters\n";
 for(int i=0; i<clusters.size(); i++)
  cerr<<"    Cluster["<<i<<"] has "<<clusters[i].size()<<" positions\n";
 
 for(int i=0; i<mpos.size(); i++) mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,true);
 int nRem=0;
 for(int i=0; i<clusters.size(); i++)
  if(clusters[i].size()>minN)
 {
  for(int j=0; j<clusters[i].size(); j++)
  {
   mpos[clusters[i][j]].setFlag(MosquitoPosition::flagType::KEEPME,false);
   nRem++;
  }
 } 
 removeMarkedMosquitoes(); 
 return nRem; 
}

vector<vector<int> >  MosquitoesInFrame::getClusters(double minD, int minN)
{
 vector<vector<int> > clusters;
 for(int i=0; i<mpos.size();i++)
 {
  vector<int> possibleClusters;

  double xi=mpos[i].getX();
  double yi=mpos[i].getY();
  
  for(int j=0; j<clusters.size(); j++)
  {
   for(int k=0; k<clusters[j].size(); k++)
   {
    double dist=sqrt(pow(mpos[clusters[j][k]].getX()-xi,2)+pow(mpos[clusters[j][k]].getY()-yi,2));
    if(dist<minD)
    {
     possibleClusters.push_back(j);
     break;
    }
   }
  }
  if(possibleClusters.size()==0)
  {
   cerr<<"Creating new cluster\n";
   vector<int> tmp;
   tmp.push_back(i);
   clusters.push_back(tmp);
  }
  else if(possibleClusters.size()==1)
  {
   cerr<<"Adding to existing cluster\n";
   clusters[possibleClusters[0]].push_back(i);
  }
  else
  {
   cerr<<"Joinig cluster\n";
   sort( possibleClusters.begin(), possibleClusters.end() );
   for(int j=possibleClusters.size()-1; j>0; j--)
   {
    clusters[possibleClusters[0]].insert(clusters[possibleClusters[0]].end(), 
                                         clusters[possibleClusters[j]].begin(), 
                                         clusters[possibleClusters[j]].end());
    clusters.erase(clusters.begin()+possibleClusters[j]);
   }
   clusters[possibleClusters[0]].push_back(i);
  }
 }
 return clusters; 
}

void MosquitoesInFrame::contourStatistics()
{
 for(int i=0; i<mpos.size(); i++)
  mpos[i].calculateContourStatistics();
}
    
int MosquitoesInFrame::filterByArea(double A, bool invert /*= false*/)
{
 for(int i=0; i<mpos.size(); i++) mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,true);
 int nRem=0;
 for(int i=0; i<mpos.size(); i++)
 {
  if(invert)
  {
   if(mpos[i].getA()>A)
   {
    mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,false);
    nRem++;
   }
  }
  else
  {
   if(mpos[i].getA()<A)
   {
    mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,false);
    nRem++;
   }
  }
 }
 removeMarkedMosquitoes(); 
 return nRem; 
}

int MosquitoesInFrame::filterByMaxInt(double I, bool invert /*= false*/)
{
 for(int i=0; i<mpos.size(); i++) mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,true);
 int nRem=0;
 for(int i=0; i<mpos.size(); i++)
 {
  if(invert)
  {
   if(mpos[i].getI()>I)
   {
    mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,false);
    nRem++;
   }
  }
  else
  {
   if(mpos[i].getI()<I)
   {
    mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,false);
    nRem++;
   }
  }
 }
 removeMarkedMosquitoes(); 
 return nRem; 
}

int MosquitoesInFrame::filterByCircularity(double I, bool invert /*= false*/)
{
 for(int i=0; i<mpos.size(); i++) mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,true);
 int nRem=0;
 for(int i=0; i<mpos.size(); i++)
 {
  if(invert)
  {
   if(mpos[i].getCircularity()>I && mpos[i].hasCont())
   {
    mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,false);
    nRem++;
   }
  }
  else
  {
   if(mpos[i].getCircularity()<I && mpos[i].hasCont())
   {
    mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,false);
    nRem++;
   }
  }
 }
 removeMarkedMosquitoes(); 
 return nRem; 
}

int MosquitoesInFrame::filterByInertia(double I, bool invert /*= false*/)
{
 for(int i=0; i<mpos.size(); i++) mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,true);
 int nRem=0;
 for(int i=0; i<mpos.size(); i++)
 {
  if(invert)
  {
   if(mpos[i].getInertia()>I && mpos[i].hasCont())
   {
    mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,false);
    nRem++;
   }
  }
  else
  {
   if(mpos[i].getInertia()<I && mpos[i].hasCont())
   {
    mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,false);
    nRem++;
   }
  }
 }
 removeMarkedMosquitoes(); 
 return nRem; 
}

int MosquitoesInFrame::filterByConvexity(double I, bool invert /*= false*/)
{
 for(int i=0; i<mpos.size(); i++) mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,true);
 int nRem=0;
 for(int i=0; i<mpos.size(); i++)
 {
  if(invert)
  {
   if(mpos[i].getConvexity()>I && mpos[i].hasCont())
   {
    mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,false);
    nRem++;
   }
  }
  else
  {
   if(mpos[i].getConvexity()<I && mpos[i].hasCont())
   {
    mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,false);
    nRem++;
   }
  }
 }
 removeMarkedMosquitoes(); 
 return nRem; 
}

void MosquitoesInFrame::setKeepMeFlag(bool b)
{
 for(int i=0; i<mpos.size(); i++) mpos[i].setFlag(MosquitoPosition::flagType::KEEPME,b);
}

