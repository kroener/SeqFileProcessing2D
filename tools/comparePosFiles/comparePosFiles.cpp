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
#include <vector>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <string>
#include <unistd.h>
#include "MosquitoesInFrame.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoPosition.hpp"

using namespace std;

int main ( int argc, char *argv[])
{
 if(argc==4)
 {
  int nFrames=atoi(argv[3]);
  MosquitoesInSeq pos1;
  pos1.setNoOfFrames(nFrames);
  pos1.loadFromFile(argv[1],false,false);
  MosquitoesInSeq pos2;
  pos2.setNoOfFrames(nFrames);
  pos2.loadFromFile(argv[2],false,false);
  MosquitoesInSeq A;
  A.setNoOfFrames(nFrames);
  MosquitoesInSeq B;
  B.setNoOfFrames(nFrames);
  MosquitoesInSeq AB;
  AB.setNoOfFrames(nFrames);
  MosquitoesInFrame *a;
  MosquitoesInFrame *b;
  MosquitoesInFrame *ab;
  MosquitoesInFrame *p1;
  MosquitoesInFrame *p2;
  for(unsigned int i=0; i<nFrames; i++)
  {
   p1=pos1.mosqInFrameI(i);
   p2=pos2.mosqInFrameI(i);
   
   a=A.mosqInFrameI(i);
   ab=AB.mosqInFrameI(i);
   b=B.mosqInFrameI(i);
   if(p1 && p2 && a && ab && b)
   {
    int np1=p1->numOfMosquitoes();
    for(int j=0; j<np1; j++)
    {
     MosquitoPosition* tmp=p1->mosqi(j);
     int id2=p2->mosqIndex(tmp->getX(),tmp->getY());
     if( id2==-1)
     {
      a->addMosquito(*tmp,0,0);
      a->setTime(p1->getTime());
      a->setMaxDiff(p1->getMaxDiff());
      a->setFrameNo(p1->getFrameNo());
     }
     else
     {
       ab->addMosquito(*tmp,0,0);
       p2->removeMosquito(id2);
       ab->setTime(p1->getTime());
       ab->setMaxDiff(p1->getMaxDiff());
       ab->setFrameNo(p1->getFrameNo());
     } 
    }
    int np2=p2->numOfMosquitoes();
    for(int j=0; j<np2; j++)
    {
     MosquitoPosition* tmp=p2->mosqi(j);
     b->addMosquito(*tmp,0,0);
     b->setTime(p2->getTime());
     b->setMaxDiff(p2->getMaxDiff());
     b->setFrameNo(p2->getFrameNo());
    }        
   }
  }
  A.saveToFile("onlyInA.txt");
  AB.saveToFile("bothInAAndB.txt");
  B.saveToFile("onlyInB.txt");
 }
 else
 {
   cerr<<"Usage: "<<argv[0]<<" pos1 pos2 nFrames\n";
 }
 return 0;
}
