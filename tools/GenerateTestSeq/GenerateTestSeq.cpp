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
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include "iSeq.hpp"
#include "oSeq.hpp"
#include <string.h>
#include "Image.hpp"
#include "MosquitoesInFrame.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoPosition.hpp"
//#define __STDC_FORMAT_MACROS

using namespace std;

int main(int argc , char* argv[])
{
 if(argc==10){
  unsigned int x=atoi(argv[1]);
  unsigned int y=atoi(argv[2]);
  double dt=atof(argv[3]);
  unsigned int nf=atoi(argv[4]);
  unsigned int nc=atoi(argv[5]);
  double minr=atof(argv[6]);
  double maxr=atof(argv[7]);
  double sigma=atoi(argv[8]);
  
  vector<vector<double> > part_pos;
  double max_step=10.;
  cv::RNG rng;
  for(int i=0;i<nc;i++)
  {
    double xc = rng.uniform( 0, x );
    double yc = rng.uniform( 0, y );
    double rc = rng.uniform(minr,maxr);
    vector<double> tmp;
    tmp.push_back(xc);
    tmp.push_back(yc);
    tmp.push_back(rc);
    part_pos.push_back(tmp);
  }
  oSeq seq;
  seq.open(argv[9]);
  MosquitoesInSeq out;
  out.setNoOfFrames(nf);
  MosquitoesInFrame *mif;

  double t=0.;
  Image<unsigned char>* timg;
   timg=new Image<unsigned char>;
   timg->create(x,y);
   unsigned char* buffer=timg->bufferPointer();
  for(unsigned i=0;i<nf;i++)
  {
   mif=out.mosqInFrameI(i);

   for(unsigned int k=0; k<x*y;k++)buffer[k]=0;
   for(unsigned j=0; j<nc;j++)
   {
    cv::Point center;
    center.x = part_pos[j][0];
    center.y = part_pos[j][1];
    double r = part_pos[j][2];
    cv::Mat m(y, x, CV_8UC1, cv::Scalar(0));
    cv::circle(m,center, r, cv::Scalar(255), -1); 
    cv::GaussianBlur(m, m, cv::Size(0,0),sigma);
    for(unsigned int k=0; k<x*y;k++)
     buffer[k]+=m.data[k];    
    cout << i<<" "<<j<<" "<< part_pos[j][0]<<" "<<part_pos[j][1]<<" "<<part_pos[j][2]<<endl;
    MosquitoPosition tmp(part_pos[j][0],part_pos[j][1],r*r*3.142,255,1,1,-1,r);
    mif->addMosquito(tmp,0,0);
    part_pos[j][0] = part_pos[j][0] + rng.uniform( -max_step, max_step );
    part_pos[j][1] = part_pos[j][1] + rng.uniform( -max_step, max_step );  
   }
   for(unsigned int k=0; k<x*y;k++)
     buffer[k]=255-buffer[k];  
   timg->setT(t);
   seq.saveImage(timg);
   t+=dt;
  }
  out.saveToFile(string(argv[9])+"_pos.txt"); 
 }
 else{
  cerr << "use:" << argv[0] <<" x y dt nf nc minr maxr sigma outf"<<endl;
 }
 return 0;
}
