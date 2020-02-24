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
#ifndef MOSQUITOESINFRAME_hpp
#define MOSQUITOESINFRAME_hpp
#include "MosquitoPosition.hpp"
#include <vector>
#include <iostream>
#include <iomanip> 
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

/**
  * \brief class to organize MosquitoPositions per frame
  */
class MosquitoesInFrame
{
public:
    MosquitoesInFrame();
    MosquitoesInFrame(const MosquitoesInFrame& a);
    ~MosquitoesInFrame() {};
    int addMosquito(MosquitoPosition impos,int ofx, int ofy);
    int addMosquito(double ix, double iy, double iarea, double imaxInt, double circularity, double inertia, double convexity, double radius, vector<cv::Point> c = vector<cv::Point>());
    int addMosquito(double ix, double iy, double iarea, double imaxInt);
    int removeMosquito(unsigned long i);
    int removeMosquito(double ix, double iy, int searchRadius = 8, bool onlyOne = true);
    int removeAllMosquito();
    int clean(double minDist);
    int clean(double minArea,double maxArea);
    int setMaxDiff(double v)
    {
        maxDiff=v;
        return 0;
    };
    int numOfMosquitoes();
    MosquitoPosition* mosqi(unsigned long i);
    MosquitoPosition* mosqi(double ix, double iy,double searchRadius=3);
    int mosqIndex(double ix, double iy);
    bool getNearest(double ix, double iy, double searchRadius,int &j, double &odist);
    int setMinArea(double v)
    {
        minA=v;
        return 0;
    };
    int setMaxArea(double v)
    {
        maxA=v;
        return 0;
    };
    int setThreshold(double v)
    {
        th=v;
        return 0;
    };
    int setMinThreshold(double v)
    {
        minTh=v;
        return 0;
    };
    int setIth(int v)
    {
        ith=v;
        return 0;
    };
    int setFrameNo(unsigned long i)
    {
        frameNo=i;
        return 0;
    };
    int setProcessed(int i)
    {
        processed=i;
        return 0;
    };
    bool setTime(vector<int> i);
    vector<int> getTime();
    int setTsSec(int i)
    {
        tsSec=i;
        return 0;
    };
    int setTsMSec(unsigned short i)
    {
        tsMSec=i;
        return 0;
    };
    int setTsUSec(unsigned short i)
    {
        tsUSec=i;
        return 0;
    };
    int getTsSec()
    {
        return tsSec;
    };
    unsigned short getTsMSec()
    {
        return tsMSec;
    };
    unsigned short getTsUSec()
    {
        return tsUSec;
    };
    double getMinArea()
    {
        return maxA;
    };
    double getMaxArea()
    {
        return minA;
    };
    double getThreshold()
    {
        return th;
    };
    unsigned long getF()
    {
        return frameNo;
    };
    double getMaxDiff()
    {
        return maxDiff;
    };
    double getMinThreshold()
    {
        return minTh;
    };
    int getIth()
    {
        return ith;
    };
    unsigned long getFrameNo()
    {
        return frameNo;
    };
    int getProcessed()
    {
        return processed;
    };
    bool removeAndClean(int i);

    int clear();

    int appendToBackup(ostream* o);
    bool PointInPolygon(MosquitoPosition m, vector<QPointF>  poly);
    int removePointsInsideOfPolygon(vector<QPointF> poly, bool inverse = false);
    vector<cv::Point2f> pointsInsideOfPolygon(vector<QPointF> poly);
    vector<vector<double> > pointsInsideOfPolygonAsVector(vector<QPointF> poly);
    int removeMarkedMosquitoes();
    int cleanClusters(double minD, int minN);
    vector<vector<int> > getClusters(double minD, int minN);
    void contourStatistics();     
    int filterByArea(double A, bool invert = false);
    int filterByMaxInt(double I, bool invert = false);
    int filterByCircularity(double I, bool invert = false);
    int filterByInertia(double I, bool invert = false);
    int filterByConvexity(double I, bool invert = false);
    void setKeepMeFlag(bool b);

private:
    vector<MosquitoPosition> mpos;
    double maxDiff;
    unsigned long frameNo;
    int processed;
    int ith;
    double minTh;
    double th;
    double minA;
    double maxA;
    int tsSec;
    unsigned short tsMSec;
    unsigned short tsUSec;
};
#endif //MOSQUITOESINFRAME_hpp
