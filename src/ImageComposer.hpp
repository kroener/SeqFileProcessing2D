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
#ifndef IMAGECOMPOSER_HPP
#define IMAGECOMPOSER_HPP
#include "iSeq.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoRegistry.hpp"
#include "Viewer.h"
#include "Image.hpp"
#include "VideoPlayerWidget.h"
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

class ImageComposer
{
public:
    ImageComposer();
    int setC(int c);
    int setR(int r);
    int setSeq(iSeq* s, int c, int r);
    int setP(MosquitoesInSeq*  p, int c, int r);
    int setT(MosquitoRegistry* t, int c, int r);
    MosquitoesInSeq* currMosqPos();
    MosquitoRegistry* currTracks();
    int getMinTrigger();
    int getMaxTrigger();

    Image<unsigned char>* getImage(int i);
    bool haveImage(int t);
    bool setTimeBasedOnCurrentSeq(int nnn=0, int mmm=0, int takeN=100);
    vector<int> getTime(int t);

private:
    int updateCombinedPositions();
    void update();
    int getOffX(int i, int j);
    int getOffY(int i, int j);
    iSeq *seq[4][4];
    MosquitoesInSeq *MosqInSeq[4][4];
    MosquitoRegistry *MosqReg[4][4];
    bool isOpen[4][4];
    bool have_positions[4][4];
    bool have_tracks[4][4];
    unsigned long frame[4][4];
    int width;
    int height;
    int vh;
    int vw;
    unsigned char background;
    int ox[4];
    int oy[4];
    int wx[4];
    int wy[4];
    int ready;
    Image<unsigned char> current;
    MosquitoesInSeq currentPositions;
    MosquitoRegistry currentTracks;
    int minTrigger;
    int maxTrigger;
    double mm2px;
    bool timeBasedOnSeq;
    double t0;
    double dt;
};
#endif // IMAGECOMPOSER_HPP 
