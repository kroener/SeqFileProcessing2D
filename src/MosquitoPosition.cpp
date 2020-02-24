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
#include <string>
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

MosquitoPosition::MosquitoPosition(double ix, double iy, double iarea, double imaxInt,double icircularity, double iinertia, double iconvexity, double iradius, vector<cv::Point> ic/* = vector<cv::Point>() */)
{
    flag=1;
    x=ix;
    y=iy;
    area=iarea;
    maxInt=imaxInt;
    ID="";
    circularity=icircularity;
    inertia=iinertia;
    convexity=iconvexity; 
    radius=iradius;
    if(ic.size()>0)
    {
     for(unsigned int j=0; j<ic.size(); j++)
      c.push_back(ic[j]);
     hasContour=true;
    }
    else
    {
     hasContour=false;    
    }
}

MosquitoPosition::MosquitoPosition(const MosquitoPosition& a)
{
    flag=a.flag;
    x=a.x;
    y=a.y;
    area=a.area;
    maxInt=a.maxInt;
    ID=a.ID;
    circularity=a.circularity;
    inertia=a.inertia;
    convexity=a.convexity; 
    radius=a.radius;
    if(a.c.size()>0)
    {
     for(unsigned int j=0; j<a.c.size(); j++)
      c.push_back(a.c[j]);
     hasContour=true;
    }
    else
    {
     hasContour=false;    
    }
}

double MosquitoPosition::getX()
{
    return x;
}

double MosquitoPosition::getY()
{
    return y;
}

double MosquitoPosition::getA()
{
    return area;
}

double MosquitoPosition::getI()
{
    return maxInt;
}

int MosquitoPosition::getFlag()
{
    return flag;
}

int MosquitoPosition::setFlag(flagType i, bool b /*=true*/)
{
    if(b)
     flag |= i;
    else
     flag &= ~i;
    return 0;
}

int MosquitoPosition::setFlag(int i, bool replace /*= true */)
{
    if(replace)
    {
     flag = i;
    }
    else
    {
     flag ^= i;
    }
    return 0;
}

double MosquitoPosition::getInertia()
{
    return inertia;
}
double MosquitoPosition::getConvexity()
{
    return convexity;
}

double MosquitoPosition::getCircularity()
{
    return circularity;
}

int MosquitoPosition::move(int ofx,int ofy)
{
    x+=ofx;
    y+=ofy;
    return 0;
}

bool MosquitoPosition::hasCont()
{
    return hasContour;
}

bool MosquitoPosition::MosquitoHasContour()
{
  return hasContour;
}

vector<cv::Point> MosquitoPosition::getContour()
{
  return c;
}

bool MosquitoPosition::setContour(vector<cv::Point> ic, bool ignoreError /*= false*/)
{
  if(ic.size()>0)
  {
   hasContour=true;
   ic.swap(c);
   double area = cv::contourArea(c);
   cv::Moments mu=moments(c,false);
   double xp=mu.m10/mu.m00;
   double yp=mu.m01/mu.m00;
   if(!ignoreError)
    if(!(x==xp && y==yp))
   {
    cerr<<"error: contour does not fit to coordinate!\n";
    cerr<<"("<<x<<","<<y<<") -> ("<<xp<<","<<yp<<")\n";
   }
   double ratio;
   double perimeter = cv::arcLength(cv::Mat(c), true);
   ratio = 4 * CV_PI * mu.m00 / (perimeter * perimeter);
   circularity=ratio;
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
   inertia=ratio;
   vector < cv::Point > hull;
   convexHull(cv::Mat(c), hull);
   double hullArea = cv::contourArea(cv::Mat(hull));
   ratio = area / hullArea;
   convexity=ratio;
   vector<double> dists;
   for (size_t j = 0; j < c.size(); j++)
   {
     cv::Point2d pt = c[j];
     dists.push_back(cv::norm(cv::Point2d(xp,yp) - pt));
   }
   std::sort(dists.begin(), dists.end());
   ratio = (dists[(dists.size() - 1) / 2] + dists[dists.size() / 2]) / 2.;
   radius=ratio;

   return true;
  }
  return false;
}

bool MosquitoPosition::calculateContourStatistics()
{
 if(hasContour)
 {
  cv::Moments mu=cv::moments(c,false);
  double ratio;
  double perimeter = arcLength(cv::Mat(c), true);
  ratio = 4 * CV_PI * mu.m00 / (perimeter * perimeter);
  circularity=ratio;
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
  inertia=ratio;
  vector < cv::Point > hull;
  convexHull(cv::Mat(c), hull);
  double hullArea = contourArea(cv::Mat(hull));
  ratio = area / hullArea;
  convexity=ratio;
  vector<double> dists;
  for (size_t j = 0; j < c.size(); j++)
  {
      cv::Point2d pt = c[j];
      dists.push_back(cv::norm(cv::Point2d(x,y) - pt));
  }
  std::sort(dists.begin(), dists.end());
  ratio = (dists[(dists.size() - 1) / 2] + dists[dists.size() / 2]) / 2.;
  radius=ratio;
  return true;  
 }
 else
  return false;
}