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
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>
#include "MosquitoesInFrame.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoPosition.hpp"
#include "MosquitoRegistry.hpp"
#include "TrackPoint.hpp"

using namespace std;

struct costOfConnection {
 double cost;
 int source;
 int target;
};

bool sortCostOfConnectionByCost(const costOfConnection &a, const costOfConnection &b)
{
 return a.cost < b.cost;
}

int main(int argc, char *argv[])
{
 if(argc==12)
 {
  unsigned long nFrames = atoi(argv[2]);
  double TrackMinD = atof(argv[3]);
  double TrackMaxD = atof(argv[4]);
  int TrackMaxG = atoi(argv[5]);
  double TrackMinA = atof(argv[6]);
  double TrackMaxA = atof(argv[7]);
  int TrackMaxN = atoi(argv[8]);
  int TrackMaxCL = atoi(argv[9]);
  int TrackMinTD = atoi(argv[10]);
  
  MosquitoesInSeq tmpP;
  tmpP.setNoOfFrames(nFrames);
  
  tmpP.loadFromFile(argv[1],false,false);
  MosquitoRegistry Reg;
  vector<int> ids;
  vector<vector<double> > LostMosquitoes;
  vector<int> LostMosquitoesIds;

  for(unsigned long i=0; i<tmpP.getNoOfFrames();i++)
  {
   unsigned long trigger=i;
   if(tmpP.isDynamic())
    trigger=tmpP.triggerOfIdx(i);
   unsigned int n=tmpP.MosqsInFrame(trigger,tmpP.isTriggerBased());
   if(i==0)
   {
    int cleaned=tmpP.clean(trigger,TrackMinD,tmpP.isTriggerBased());
    cleaned+=tmpP.clean(trigger,TrackMinA,TrackMaxA,tmpP.isTriggerBased());
    n=tmpP.MosqsInFrame(trigger,tmpP.isTriggerBased());
    for(unsigned int ii=0;ii<n;ii++)
     ids.push_back(-1);
   }
   unsigned long trigger2=i+1;
   if(tmpP.isDynamic())
    trigger2=tmpP.triggerOfIdx(i+1);

   int cleaned=tmpP.clean(trigger2,TrackMinD,tmpP.isTriggerBased());
   cleaned+=tmpP.clean(trigger2,TrackMinA,TrackMaxA,tmpP.isTriggerBased());
   vector<vector<double> > a=tmpP.getAllPosAsVector(trigger,tmpP.isTriggerBased());
   n=a.size();
   int starts=0;
   int added=0;
   for(unsigned long ii=0; ii<n;ii++)
   {
    if(ids[ii]==-1)
    {
     ids[ii]=Reg.registerNewMosquito();
     starts++;
    }
    
    TrackPoint NewTrackPoint(a[ii][0],a[ii][1],a[ii][2],a[ii][3],a[ii][4],a[ii][5],
                             a[ii][6],a[ii][7],a[ii][8],a[ii][9],a[ii][10],a[ii][11],a[ii][12],a[ii][13]);
    Reg.updateTrack(ids[ii],NewTrackPoint);
    added++;
   }

   if(trigger2!=trigger+1)
   {
    LostMosquitoes.insert(end(LostMosquitoes), begin(a), end(a));
    LostMosquitoesIds.insert(end(LostMosquitoesIds), begin(ids), end(ids));
    vector<int>().swap(ids);
    vector<vector<double> >().swap(a);
   }

   if(i<tmpP.getNoOfFrames()-1)
   {
    int m=tmpP.MosqsInFrame(trigger2,tmpP.isTriggerBased());
    vector<vector<double> > tmpLostMosquitoes;
    vector<int> tmpLostMosquitoesIds;
    int stillLost=0;
    int completelyLost=0;
    for(unsigned int iii=0; iii<LostMosquitoes.size(); iii++)
    {
     if(trigger2-LostMosquitoes[iii][2]-1<TrackMaxG)
     {
      tmpLostMosquitoes.push_back(LostMosquitoes[iii]);
      tmpLostMosquitoesIds.push_back(LostMosquitoesIds[iii]);
      stillLost++;
     }
     else
     {
      completelyLost++;
     }
    }
    a.insert(end(a), begin(tmpLostMosquitoes), end(tmpLostMosquitoes));
    ids.insert(end(ids), begin(tmpLostMosquitoesIds), end(tmpLostMosquitoesIds));
    n=a.size();
    LostMosquitoes=tmpLostMosquitoes;
    LostMosquitoesIds=tmpLostMosquitoesIds;

    if(n>0 && m>0)
    {
     unsigned int nrows = n;
     unsigned int ncols = m;
     vector<vector<double> >  matrix(n,vector<double>(m));
     vector<vector<size_t> >  idx(n,vector<size_t>(m));
     vector<vector<double> > b=tmpP.getAllPosAsVector(trigger2,tmpP.isTriggerBased());
     for ( unsigned int row = 0 ; row < nrows ; row++ ) {
      for ( unsigned int col = 0 ; col < ncols ; col++ ) {
       idx[row][col] = col;
       double dt=b[col][2]-a[row][2];
       matrix[row][col] = (double)sqrt(pow(b[col][0]-a[row][0],2)+pow(b[col][1]-a[row][1],2)/dt);
      }
     }
     double maxTD=TrackMaxD;
     for ( unsigned int row = 0 ; row < nrows ; row++ ) {
      idx[row].erase(std::remove_if(idx[row].begin(), idx[row].end(),
       [&matrix,&row,&maxTD](unsigned long const& x){
        if (matrix[row][x]<maxTD)
         return false;
        else
         return true;
       }), idx[row].end());
       sort(idx[row].begin(), idx[row].end(),
        [&matrix,&row](size_t i1, size_t i2) {return matrix[row][i1] < matrix[row][i2];});
       if(idx[row].size()>TrackMaxN)
        idx[row].resize(TrackMaxN);
     }
     vector<vector<double> >  cost;
     for ( unsigned int row = 0 ; row < nrows ; row++ ) {
      vector<double> tmpc;
      vector<vector<double> > move_vector=Reg.TrackMotionVector(row,TrackMaxCL);
      for ( unsigned int col = 0; col < idx[row].size(); col++)
      {
       double dt=b[idx[row][col]][2]-a[row][2];
       if(dt>1)
        dt=1.1*dt;
       if(move_vector.size()>TrackMinTD)
       {
        double dx=(b[idx[row][col]][0]-a[row][0])/dt;
        double dy=(b[idx[row][col]][1]-a[row][1])/dt;
        double tmpval=0;
        for(unsigned int nn=0; nn<move_vector.size();nn++)
         tmpval+=sqrt(pow(move_vector[nn][0]-dx,2)+pow(move_vector[nn][1]-dy,2));
        if(move_vector.size()>0)
         tmpval/=move_vector.size();
        tmpc.push_back(tmpval);
       }
       else
       {
        tmpc.push_back(sqrt(pow(b[idx[row][col]][0]-a[row][0],2)
                                +pow(b[idx[row][col]][1]-a[row][1],2))/dt);
       }
      }
      cost.push_back(tmpc);
     }
     vector<costOfConnection> coc;
     for (unsigned int row = 0 ; row < nrows ; row++ ) {
      for ( unsigned int col = 0 ; col < cost[row].size() ; col++ ) {
       costOfConnection tmpCost;
       tmpCost.cost=cost[row][col];
       tmpCost.source=row;
       tmpCost.target=idx[row][col];
       coc.push_back(tmpCost);
      }
     }
     sort(coc.begin(), coc.end(), sortCostOfConnectionByCost);
     vector<costOfConnection> selectedConnections;
     for(unsigned int nn=0; nn< coc.size(); nn++)
     {
      int valid=0;
      for(unsigned int mm=0;mm<selectedConnections.size();mm++)
      {
       if (coc[nn].source == selectedConnections[mm].source ||
       coc[nn].target == selectedConnections[mm].target)
       {
        valid=1;
        break;
       }
      }
      if(valid==0)
       selectedConnections.push_back(coc[nn]);
     }
     vector<int> tmpVec;
     for (unsigned int col = 0 ; col < ncols ; col++ )
      tmpVec.push_back(-1);
     vector<int> tmprows;
     for (unsigned int row = 0 ; row < nrows ; row++ )
      tmprows.push_back(-1);  
     for ( unsigned int nn = 0 ; nn < selectedConnections.size() ; nn++ )
     {
      tmpVec[selectedConnections[nn].target]=ids[selectedConnections[nn].source];
      tmprows[selectedConnections[nn].source]=selectedConnections[nn].target;
     }
     vector<vector<double> > lost;
     vector<int> lostIds;
     for (unsigned int row = 0 ; row < nrows ; row++ )
     {
      if(tmprows[row]==-1)
      {
       lost.push_back(a[row]);
       lostIds.push_back(ids[row]);
      }
     }
     LostMosquitoes=lost;
     LostMosquitoesIds=lostIds;
     ids=tmpVec;
    }
    else{
     LostMosquitoes=a;
     LostMosquitoesIds=ids;
     ids.clear();
     for(int jj=0;jj<m;jj++)
      ids.push_back(-1);
    }
   }
  }
  Reg.saveToFile(argv[11]);  
 }
 else
 {
   cerr<<"Usage: "<<argv[0]<<" posFile nFrames TrackMinD TrackMaxD TrackMaxG TrackMinA TrackMaxA TrackMaxN TrackMaxCL TrackMinTD trackFile\n";
 }
 return 0;
}
