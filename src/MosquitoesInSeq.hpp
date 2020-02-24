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
#ifndef MOSQUITOESINSEQ_hpp
#define MOSQUITOESINSEQ_hpp
#include "MosquitoesInFrame.hpp"
#include <vector>
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
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include "iSeq.hpp"
#include <QPointF>

using namespace std;

/**
  * \brief class to organize all MosquitoPosition in all frames
  */
class MosquitoesInSeq
{
public:
    MosquitoesInSeq();
    MosquitoesInSeq(const MosquitoesInSeq& a);
    MosquitoesInSeq(MosquitoesInSeq&& other);
    ~MosquitoesInSeq();
    MosquitoesInSeq& operator=(MosquitoesInSeq&& other);
    MosquitoesInSeq& operator=(MosquitoesInSeq& other);
    int setNoOfFrames(unsigned long frames);
    unsigned long getNoOfFrames(){return mframes.size();};
    unsigned long getLastFrameWithMosquito();
    unsigned long getFirstFrameWithMosquito();
    long getMaxTrigger();
    MosquitoesInFrame* mosqInFrameI(unsigned long i, bool directAccess=false, bool isTrigger = false );
    int clean(unsigned long i, double minDist, bool isTrigger = false);
    int clean(unsigned long i, double minArea, double maxArea, bool isTrigger = false);
    vector<cv::Point2f> getAllPos(unsigned long i, bool isTrigger = false);
    vector<cv::Point2f> getAllPos(unsigned long n,unsigned long m, bool isTrigger = false);
    vector<double> getPointMeta(unsigned long n,unsigned long m, bool isTrigger = false);
    vector<double> getPointMeta(unsigned long n, bool isTrigger = false);
    vector<vector<double> > getAllPosAsVector(unsigned long i, int select=1, bool isTrigger = false);
    int getNumOfAllPos();
    int loadFromFile(string filename, bool enableDynamic = false, bool displayStatus  = false );
    int loadFromFile(string filename, unsigned long rstart, unsigned long rstop , bool enableDynamic = false, bool displayStatus = false );
    int saveToFile(string filename, int saveFrom = -1, int saveTo = -1);
    int MosqsInFrame(unsigned long f, bool isTrigger = false);
    int giveSeqPointer(iSeq* iseq);
    int getTsSec(unsigned long i, bool isTrigger = false);
    unsigned short getTsMSec(unsigned long i, bool isTrigger = false);
    unsigned short getTsUSec(unsigned long i, bool isTrigger = false);
    int removeAll(); 
    int removePointsInsideOfPolygon(vector<QPointF> poly,unsigned long n, unsigned long m, bool inverse = false, bool isTrigger = false );
    int removePoints(double x, double y ,unsigned long n, unsigned long m, bool isTrigger = false, double searchRadius =8, bool onlyOne =true);
    int removePoints(unsigned long n, unsigned long m, bool isTrigger = false);
    vector<cv::Point2f> pointsInsideOfPolygon(vector<QPointF> poly,unsigned long n, unsigned long m, bool isTrigger = false);
    vector<vector<double> > pointsInsideOfPolygonAsVector(vector<QPointF> poly,unsigned long n, unsigned long m, bool isTrigger = false);
    vector<unsigned long> findMosquito(QPointF p,unsigned long n, unsigned long m, bool isTrigger = false);
    bool copyTo(MosquitoesInSeq& a); 
    bool resetFlags();
    vector<vector<cv::Point> > getContours(unsigned long i, unsigned long j=0, bool isTrigger = false);
    bool isDynamic();
    bool isTriggerBased();
    vector<unsigned long> getTriggerRange();
    bool dataForTrigger(int t);
    int triggerOfIdx(unsigned long i);
    bool fixTime();
    int removeMarkedMosquitoes(unsigned long n, unsigned long m, bool isTrigger = false, bool directAccess = false);
    void contourStatistics();     
    int filterByArea(double A, bool invert = false);
    int filterByMaxInt(double I, bool invert = false);
    int filterByCircularity(double I, bool invert = false);
    int filterByInertia(double I, bool invert = false);
    int filterByConvexity(double I, bool invert = false);
    int filterByRadius(double I, bool invert = false);
    void setKeepMeFlag(bool b);
private:
    void sortFrames();
    long findTrigger(unsigned long tmp);
    long findTriggerNearest(unsigned long tmp, bool next=true);
    long getTriggerFromSeq(unsigned long i);
    vector<MosquitoesInFrame> mframes;
    unsigned long nFrames;
    int initialized;
    iSeq* currSeq;
    int haveSeq;
    bool dynamic;
    bool triggerBased;
    vector<unsigned long> FrameLookup;
};
#endif //MOSQUITOESINSEQ_hpp
