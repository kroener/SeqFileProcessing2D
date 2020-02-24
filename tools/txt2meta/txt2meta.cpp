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
  Metadata metaOut;
  ifstream input( argv[1]);
  bool metaCreated=false;
  for( string line; getline( input, line ); )
  {
   long i,n;
   int t1,t2,t3;
   int ret=sscanf(line.c_str(),"%ld %ld %d %d %d",&i,&n,&t1,&t2,&t3);
   if(ret==5)
   {
    if(metaCreated==false)
    {
     int ret=metaOut.create(argv[2],1);
     if(ret==0)
     {
      metaCreated=true;
      vector<int> t;
      t.push_back(t1);
      t.push_back(t2);
      t.push_back(t3);
      Metadata::timeValue tv;
      if(t.size()==3)
      {   
       tv.sec=t[0];
       tv.msec=t[1];
       tv.usec=t[2];
      }
      ret=metaOut.write(i,n,tv);
      if(ret!=0)
      {
       cerr<<"Error: Can't write Metadata to file\n";
       break;
      }     
     }
     else
     {
      cerr<<"Error: Can't write Metadata to file\n";
      return 1;
     }
    }
    else
    {
     vector<int> t;
     t.push_back(t1);
     t.push_back(t2);
     t.push_back(t3);
     Metadata::timeValue tv;
     if(t.size()==3)
     {   
      tv.sec=t[0];
      tv.msec=t[1];
      tv.usec=t[2];
     }
     ret=metaOut.write(i,n,tv);
     if(ret!=0)
     {
      cerr<<"Error: Can't write Metadata to file\n";
      break;
     }
    }
   }
   else if(ret==2)
   {
    if(metaCreated==false)
    {
     int ret=metaOut.create(argv[2],0);
     if(ret==0)
     {
      metaCreated=true;
      ret=metaOut.write(i,n);
      if(ret!=0)
      {
       cerr<<"Error: Can't write Metadata to file\n";
       break;
      }         
     }
     else
     {
      cerr<<"Error: Can't write Metadata to file\n";
      return 1;
     }
    }
    else
    {
     ret=metaOut.write(i,n);
     if(ret!=0)
     {
      cerr<<"Error: Can't write Metadata to file\n";
      break;
     }   
    }
   }
   else
   {
    cerr<<"Problem reading line:\n"<<line<<"\n";
   }
  }
  return 0;
 }
 else
 {
  cerr<<"I need intput and output filenames!\n";
 }
}
