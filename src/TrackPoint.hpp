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
#ifndef TRACKPOINT_hpp
#define TRACKPOINT_hpp
#include <string>
using namespace std;

/**
  * \brief Storage for points in tracks
  */
class TrackPoint
{
public:
    TrackPoint(double ix, double iy, int iframe, int itsec, int itmsec, int itusec, 
               double iintensity, double iarea, double imaxDiff,
               double iminA, double imaxA, double ithreshold, double iminthreshold, int iith,
               MosquitoPosition* in = nullptr);
    TrackPoint(const TrackPoint &other);
    int move(int ofx, int ofy);
    double time();
    bool setMosquitoPointer(MosquitoPosition* in);
    MosquitoPosition* getMosquitoPointer();
    double x;
    double y;
    int frame;
    int tsec;
    int tmsec;
    int tusec;
    double intensity;
    double area;
    double maxDiff;
    double minA;
    double maxA;
    double th;
    double minTh;
    int ith;
    MosquitoPosition* mosquitoPointer;
    int flag;
    
};
#endif //TRACKPOINT_hpp
