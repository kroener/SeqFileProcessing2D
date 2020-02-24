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
#ifndef iSeq_hpp
#define iSeq_hpp
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include "Image.hpp"
#include "MyVideoReader.hpp"
#include "Header.hpp"
#include "Metadata.hpp"
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
#include "MosquitoesInFrame.hpp"
using namespace std;

/**
  * \brief Class to read *.seq files
  */
class iSeq
{
public:
    iSeq();
    iSeq(const iSeq& a) = delete;
    ~iSeq();
    iSeq(iSeq&& other);
    iSeq& operator=(iSeq&& other);
    string filename();
    string posfilename();
    string trackfilename();
    void setPosfilename(string s);
    void setTrackfilename(string s);
    int open(string filename);
    int loadImage(unsigned long i);
    Image<short int> diffImage(unsigned long i, unsigned long j);

    unsigned long allocatedFrames();
    unsigned long currentFrame();
    Image<unsigned char>* getImagePointer(unsigned long i);
    Header* pointerToHeader()
    {
        return &header;
    };
    int sec(unsigned long i);
    unsigned short ms(unsigned long i);
    unsigned short us(unsigned long i);
    cv::Mat image(unsigned long n, unsigned long i);
    int setRequired(int n);
    int getRequired(){return requiredImages;};
    vector<int>  timeOfFrameI(unsigned long n);
    void openBackupFile();
    void closeBackupFile();
    void backupCurrentPositions(MosquitoesInFrame* currentFrame);
    string settingsName();
    string backupName();
    string metaName();
    int hasMeta();
    int metaData(unsigned long i);
    int lookUpTrigger(unsigned int i);
    int lookUpValidTrigger(unsigned int i);
    bool isImage();
    cv::Rect* getRoi();
    vector<cv::Point> getPolyRoi();
    bool gotPolyRoi();
    bool setRoi(int rx,int ry,int rw,int rh);
    bool setPolyRoi(vector<cv::Point> poly);
private:
    int loadMeta();
    Header header;
    bool has_suffix(const std::string &str, const std::string &suffix);
    int reloadImages(unsigned long i);
    Image<unsigned char>* InMemory;
    Image<double>* DoubleWorkImages;
    Image<short>* ShortWorkImages;
    int loadFromVid(Image<unsigned char> &img,unsigned long i);
    int loadFromVid(Image<unsigned char> &img);
    unsigned long requiredImages;
    unsigned long nWorkImages;
    unsigned long nShortImages;
    fstream* seqFile;
    ofstream* backupFile;
    int haveBackupFile;
    int backupInterval;
    int* framePointer;
    int loadNext;
    int thisFrame;
    unsigned long nextImage;
    string seqfilename;
    int has_buffer;
    Metadata meta;
    int have_meta;
    int isVid;
    MyVideoReader* vid;
    string positionFileName;
    string trackFileName;
    cv::Rect roi;
    vector<cv::Point> polyRoi;
    bool hasRoi;
    bool hasPolyRoi;
    int isImg;
    cv::Mat img;
    Image<unsigned char> Img;
};
#endif
