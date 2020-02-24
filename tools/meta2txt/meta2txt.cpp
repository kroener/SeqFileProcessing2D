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
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <time.h>
#include "Metadata.hpp"
using namespace std;

int main ( int argc, char *argv[])
{
 if(argc==3)
 {
  Metadata metaIn;
  metaIn.setInMemory(1);
  metaIn.load(argv[1]);
  unsigned int nf=metaIn.getNo();
  ofstream myfile;
  myfile.open(argv[2]);
  for(unsigned long i=0;i<nf;i++)
  {
   long n=metaIn.getNum(i);
   if(metaIn.timeIncluded())
   {
    vector<int> t=metaIn.getT(i);
    myfile<< i<<" "<<n<<" "<<t[0]<<" "<<t[1]<<" "<<t[2]<<"\n";
   }
   else
   {
    myfile<< i<<" "<<n<<"\n";    
   }
  }
  myfile.close();
  return 0;
 }
 else
 {
  cerr<<"I need intput and output filenames!\n";
 }
}
