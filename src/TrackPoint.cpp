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
#include "TrackPoint.hpp"
#include "MosquitoPosition.hpp"

#include <string>
using namespace std;

TrackPoint::TrackPoint(double ix, double iy, int iframe, int itsec, int itmsec, int itusec, 
               double iintensity, double iarea, double imaxDiff,
               double iminA, double imaxA, double ithreshold, double iminthreshold, int iith, 
               MosquitoPosition* in /*= nullptr*/)
{
    x=ix;
    y=iy;
    frame=iframe;
    tsec=itsec;
    tmsec=itmsec;
    tusec=itusec;
    intensity=iintensity;
    area=iarea;
    maxDiff=imaxDiff;
    minA=iminA;
    maxA=imaxA;
    th=ithreshold;
    minTh=iminthreshold;
    ith=iith;
    if(in)
     mosquitoPointer=in;
    flag=0;
}

TrackPoint::TrackPoint(const TrackPoint &other)
{
    x=other.x;
    y=other.y;
    frame=other.frame;
    tsec=other.tsec;
    tmsec=other.tmsec;
    tusec=other.tusec;
    intensity=other.intensity;
    area=other.area;
    maxDiff=other.maxDiff;
    minA=other.minA;
    maxA=other.maxA;
    th=other.th;
    minTh=other.minTh;
    ith=other.ith;
    mosquitoPointer=other.mosquitoPointer;
    flag=other.flag;
}

int TrackPoint::move(int ofx, int ofy)
{
    x+=ofx;
    y+=ofy;
    return 0;
}

bool TrackPoint::setMosquitoPointer(MosquitoPosition* in)
{
 if(in)
 {
  mosquitoPointer=in;
  return true;
 }
 else
  return false;
}

MosquitoPosition* TrackPoint::getMosquitoPointer()
{
  return mosquitoPointer;
}

double TrackPoint::time()
{
 double t=(double)tsec;
 t+=double(tmsec)/1000.;
 t+=double(tusec)/1000000.;
 return t;
}
