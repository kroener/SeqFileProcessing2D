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
#ifndef MOSQUITOREGISTRY_hpp
#define MOSQUITOREGISTRY_hpp
#include "TrackPoint.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoesInFrame.hpp"
#include "MosquitoPosition.hpp"
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

using namespace std;

/**
  * \brief Book keeping class for the tracking algorithm 
  */
class MosquitoRegistry
{
public:
    MosquitoRegistry();
    ~MosquitoRegistry() {};
    MosquitoRegistry(const MosquitoRegistry& a);
    int registerNewMosquito();
    int TrackLength(int ID);
    int updateTrack(unsigned long ID,TrackPoint tmp);
    vector<vector<double> > TrackMotionVector(int ID,int m);
    vector<vector<cv::Point2f> > getAllTracks(unsigned long frame, int lastn, int mint=0);
    vector<vector<cv::Point2f> > getAllTracksInInterval(unsigned long formFrame, unsigned long toFrame, int mint = 0);
    vector<vector<cv::Point2f> > getSelectedTracks(vector<int> TrackIDs, int mint =0);

    vector<double> getTrackMeta(unsigned long frame,int lastn, int mint=0,bool random = false, bool colorByFlag =false);
    vector<double> getTrackMetaInInterval(unsigned long fromFrame, unsigned long toFrame, int mint = 0,bool random = false, bool colorByFlag =false);
    vector<double> getSelectedTracksMeta(vector<int> TrackIDs, int mint=0,bool random =false, bool colorByFlag =false);
    int saveToFile(string filename);
    int loadFromFile(string filename, bool showProgress = false);
    int reset();
    vector<TrackPoint> getVectorOfTrackPoints(int ID);
    int addVectorOfTrackPoints(vector<TrackPoint> t, int ofx, int ofy);
    int numOfTracks();
    vector<vector<TrackPoint> > getRegistry();
    vector<int> findTrackID(double xi, double yi, double minDist, int fromFrame, int toFrame);
    vector<cv::Point2f> getSingleTrack(int ID);
    bool removePointFromTrack(int n, int m, bool split_here = false);
    bool addPointToTrack(int n, TrackPoint a, bool replace = false);
    bool sortTrack(int n);
    bool deleteTrack(int n);
    bool joinTracks(int n,int m);
    bool splitTrack(int n,int m);
    bool addVectorOfTrackPointsToTrack(int n, vector<TrackPoint> t, int ofx = 0, int ofy = 0);
    bool autoJoinTracks(double maxDist, unsigned long maxTempDist, int remTooShort = 3, double angle = 0.5);
    double trackLength(vector<TrackPoint> t);
    double calcDistFromLine(vector<TrackPoint> t);
    vector<TrackPoint> removeSpikes(vector<TrackPoint> t, double angle = 0.5);
    int removeTooShort(int x);
    vector<double> lineLineDistance(unsigned int n, unsigned int m);
    bool updateMosquitoPointer(MosquitoesInSeq* in);
    bool connectRestingTracks(int maxDT, double maxDX);
    vector<vector<double> > getDataForTrack(unsigned long n, int dataType);
    bool fixTime(MosquitoesInSeq* in);
    int markMosquitoesToKeep();
    bool estimateNextPosition(int i,double &ox, double &oy, unsigned long &on, double scalefactor = 1.0);
    bool estimatePrevPosition(int i,double &ox, double &oy, unsigned long &on, double scalefactor = 1.0);
    vector<vector<TrackPoint> > getMissingPieces(int n);
    bool copyTo(MosquitoRegistry& a);
    vector<vector<cv::Scalar> > getTrackMetaForAni(unsigned long frame,int lastn, int mint, int colorByFlag, double startFrame, double endFrame);
    cv::Scalar getRandomColor(int i);
    cv::Scalar getColor(double i, double from, double to);
    vector<vector<double> > trackStatistics(vector<TrackPoint> cT);
    bool joinBasedOnStatistics(int n, int m);
    
private:
    vector<vector<TrackPoint> > Registry;
};
#endif //MOSQUITOREGISTRY_hpp
