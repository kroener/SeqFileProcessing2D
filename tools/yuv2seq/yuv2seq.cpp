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
#include "Metadata.hpp"

using namespace std;

int main(int argc , char* argv[])  
{
 if(argc==5){
  unsigned int x=atoi(argv[1]);
  unsigned int y=atoi(argv[2]);
  Metadata metaIn;
  metaIn.setInMemory(1);
  metaIn.load(argv[4]);
  if(!metaIn.timeIncluded())
  {
   cerr<<"Metadata does not contain time...\n";
   return 1;
  }
  unsigned int nf=metaIn.getNo();
  oSeq seq;
  seq.open(argv[3]);
  seq.setFramerate(50);
  Image<unsigned char>* timg;
  timg=new Image<unsigned char>;
  timg->create(x,y);
  unsigned char* buffer=timg->bufferPointer();
  unsigned char* uv;
  uv=new unsigned char[x*y/2];
  int running=1;
  int frame=0;
  while(running)
  {
  if(fread(buffer, 1, x*y, stdin))
  {
   fread(uv, 1, x*y/2, stdin);
   if(frame<nf)
   {
   vector<int> t=metaIn.getT(frame);
   timg->setTsec(t[0]);
   timg->setTmsec(t[1]);
   timg->setTusec(t[2]);
   }
   seq.saveImage(timg);
   frame++;
  }  
  else running=0;   
  }
  cerr<<"Have written "<< frame<<" images...\n"; 
  
 }
 else{
  cerr << "use:" << argv[0] <<" x y outfile metadata"<<endl;
 }
 return 0;
}
