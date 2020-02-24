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
#include "TrackPoint.hpp"
#include "MosquitoRegistry.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoesInFrame.hpp"
#include "MosquitoPosition.hpp"
#include <vector>
#include <iostream>
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
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define PI 3.14159265
#include <QProgressDialog>
#include <QMessageBox>

using namespace std;

MosquitoRegistry::MosquitoRegistry()
{
    srand (static_cast <unsigned> (time(0)));
}

MosquitoRegistry::MosquitoRegistry(const MosquitoRegistry& a)
{
    srand (static_cast <unsigned> (time(0)));
    for(unsigned int i=0; i<a.Registry.size(); i++)
    {
     vector<TrackPoint> track;
     for(unsigned int j=0; j<a.Registry[i].size(); j++)
     {
      track.push_back(a.Registry[i][j]);
     }
     Registry.push_back(track);
    }
}

int MosquitoRegistry::registerNewMosquito()
{
    vector<TrackPoint> tmp;
    int ID=Registry.size();
    Registry.push_back(tmp);
    return ID;
}

int MosquitoRegistry::updateTrack(unsigned long ID, TrackPoint tmp)
{
    if(ID<Registry.size())
    {
     Registry[ID].push_back(tmp);
     return 0;
    }
    else
     return -1;
}

int MosquitoRegistry::TrackLength(int ID)
{
     int length=0;
     if(ID>=0 && ID<Registry.size())
      length=Registry[ID].size();
     return length;
}


vector<vector<double> >  MosquitoRegistry::TrackMotionVector(int ID, int m)
{
     vector<vector<double> > temp;
     if(ID<Registry.size() && ID>=0)
     {
      if(Registry[ID].size()>1)
      for(int j=Registry[ID].size()-1;j>=max(1,((int)Registry[ID].size()-1-m));j--)
      {
        double dx=Registry[ID][j].x-Registry[ID][j-1].x;
        double dy=Registry[ID][j].y-Registry[ID][j-1].y;
        vector<double> ntemp;
        ntemp.push_back(dx);
        ntemp.push_back(dy);
        temp.push_back(ntemp);  
      }
     }
     return temp;
}

vector<vector<cv::Point2f> > MosquitoRegistry::getAllTracks(unsigned long frame,int lastn, int mint/*=0*/)
{
    vector<vector<cv::Point2f> > tmptmp;
    for(unsigned long i=0; i<Registry.size();i++)
    {
        vector<cv::Point2f> tmp;
        for(unsigned long j=0; j<Registry[i].size(); j++)
        if(Registry[i][j].frame>(int)frame-(int)lastn && Registry[i][j].frame<=frame)
        {
        cv::Point2f m(Registry[i][j].x,Registry[i][j].y);
        tmp.push_back(m);
        }
        if(tmp.size()>=mint)
         tmptmp.push_back(tmp);
    }  
    return tmptmp;
}

vector<vector<cv::Point2f> > MosquitoRegistry::getAllTracksInInterval(unsigned long fromFrame, unsigned long toFrame, int mint/*=0*/)
{
    vector<vector<cv::Point2f> > tmptmp;
    for(unsigned long i=0; i<Registry.size();i++)
    {
        vector<cv::Point2f> tmp;
        for(unsigned long j=0; j<Registry[i].size(); j++)
        if(Registry[i][j].frame>=(int)fromFrame && Registry[i][j].frame<toFrame)
        {
        cv::Point2f m(Registry[i][j].x,Registry[i][j].y);
        tmp.push_back(m);
        }
        if(tmp.size()>=mint)
         tmptmp.push_back(tmp);
    }  
    return tmptmp;
}

vector<vector<cv::Point2f> > MosquitoRegistry::getSelectedTracks(vector<int> TrackIDs, int mint/*=0*/)
{
    vector<vector<cv::Point2f> > tmptmp;
    for(unsigned long i=0; i<TrackIDs.size();i++)
    {
        vector<cv::Point2f> tmp;
        if(TrackIDs[i]<Registry.size() && TrackIDs[i]>=0)
        {
         for(unsigned long j=0; j<Registry[TrackIDs[i]].size(); j++)
         {
         cv::Point2f m(Registry[TrackIDs[i]][j].x,Registry[TrackIDs[i]][j].y);
         tmp.push_back(m);
         }
         if(tmp.size()>=mint)
          tmptmp.push_back(tmp);
        }
    }  
    return tmptmp;
}

vector<double> MosquitoRegistry::getTrackMeta(unsigned long frame,int lastn, int mint/*=0*/,bool random /*=false*/, bool colorByFlag /*=false*/)
{
    vector<double> tmp;
    srand (static_cast <unsigned> (time(0)));
    for(unsigned long i=0; i<Registry.size();i++)
    {
        int displayTrack=0;
        for(unsigned long j=0; j<Registry[i].size(); j++)
        if(Registry[i][j].frame>(int)frame-(int)lastn && Registry[i][j].frame<=frame)
        {
           displayTrack++;
        }
        if(displayTrack>=mint)
        {
         if(random)
         {
          double r = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
          tmp.push_back(r);
         }
         else if(colorByFlag)
         {
          int foo=0;
          if(Registry[i][0].flag>0 && Registry[i][Registry[i].size()-1].flag>0) foo=1;
          else if(Registry[i][Registry[i].size()-1].flag>0) foo=2;
          else if(Registry[i][0].flag>0) foo=3;
          if(foo==0) tmp.push_back(1./8.);
          else if(foo==1) tmp.push_back(3./8.);
          else if(foo==2) tmp.push_back(5./8.);
          else if(foo==3) tmp.push_back(7./8.);
         }
         else
         {
          tmp.push_back(i/double(Registry.size()));
         }
        }
    }  
    return tmp;
}

vector<double> MosquitoRegistry::getTrackMetaInInterval(unsigned long fromFrame, unsigned long toFrame, int mint/*=0*/,bool random /*=false*/, bool colorByFlag /*=false*/)
{
    vector<double> tmp;
    for(unsigned long i=0; i<Registry.size();i++)
    {
        int displayTrack=0;
        for(unsigned long j=0; j<Registry[i].size(); j++)
        if(Registry[i][j].frame>=(int)fromFrame && Registry[i][j].frame<toFrame)
        {
           displayTrack++;
        }
        if(displayTrack>=mint)
        {
         if(Registry[i].size()>0)
         {
          if(Registry[i][0].frame>=fromFrame)
          {
           if(random)
           {
            double r = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
            tmp.push_back(r);
           }
           else if(colorByFlag)
           {
            int foo=0;
            if(Registry[i][0].flag>0 && Registry[i][Registry[i].size()-1].flag>0) foo=1;
            else if(Registry[i][Registry[i].size()-1].flag>0) foo=2;
            else if(Registry[i][0].flag>0) foo=3;
            if(foo==0) tmp.push_back(1./8.);
            else if(foo==1) tmp.push_back(3./8.);
            else if(foo==2) tmp.push_back(5./8.);
            else if(foo==3) tmp.push_back(7./8.);
           }
           else
           {
            tmp.push_back((Registry[i][0].frame-fromFrame)/double(toFrame));
           }
          }
          else
           tmp.push_back(0.0);          
         }
         else
          tmp.push_back(0.0);
        }
    }  
    return tmp;
}

vector<double> MosquitoRegistry::getSelectedTracksMeta(vector<int> TrackIDs, int mint/*=0*/,bool random /*=false*/, bool colorByFlag /*=false*/)
{
    vector<double> tmp;
    for(unsigned long i=0; i<TrackIDs.size();i++)
    {
        int displayTrack=0;
        if(TrackIDs[i]<Registry.size() && TrackIDs[i]>=0)
        {
         for(unsigned long j=0; j<Registry[TrackIDs[i]].size(); j++)
         {
           displayTrack++;
         }
         if(displayTrack>=mint)
         {
          if(Registry[TrackIDs[i]].size()>0)
          {
           {
            if(random)
            {
             double r = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
             tmp.push_back(r);
            }
            else if(colorByFlag)
            {
             int foo=0;
             if(Registry[TrackIDs[i]][0].flag>0 && Registry[TrackIDs[i]][Registry[TrackIDs[i]].size()-1].flag>0) foo=1;
             else if(Registry[TrackIDs[i]][Registry[TrackIDs[i]].size()-1].flag>0) foo=2;
             else if(Registry[TrackIDs[i]][0].flag>0) foo=3;
             if(foo==0) tmp.push_back(1./8.);
             else if(foo==1) tmp.push_back(3./8.);
             else if(foo==2) tmp.push_back(5./8.);
             else if(foo==3) tmp.push_back(7./8.);
            }
            else
            {
             tmp.push_back((double)i/double(TrackIDs.size()));
            }
           }
          }
          else
           tmp.push_back(0.0);
         }
        }
    }  
    return tmp;
}

int MosquitoRegistry::saveToFile(string filename)
{
    FILE * pFile;
    if(!( access( filename.c_str(), F_OK ) != -1 ))
    {
        pFile = fopen (filename.c_str(),"w");
        if(pFile == NULL)
        {
            QMessageBox msgBox;
            msgBox.setText(QString("Error: File already exists, choose another file...\n"));
            msgBox.exec();

            cerr << "Error file "<<filename<< " does exists!"<<endl;
        }
        else
        {
            for(unsigned long i=0; i<Registry.size();i++)
            {
                double minfr=Registry[i][0].frame;
                double maxfr=Registry[i][Registry[i].size()-1].frame;
                for(unsigned long j=0; j<Registry[i].size(); j++)
                {
                    fprintf(pFile,"%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\t%.16f\n",
                        Registry[i][j].x,
                        Registry[i][j].y,
                        Registry[i][j].intensity,
                        Registry[i][j].area,
                        Registry[i][j].maxDiff,
                        static_cast<double>(Registry[i][j].frame),
                        minfr,
                        maxfr,
                        static_cast<double>(i),
                        static_cast<double>(Registry[i][j].tsec),
                        static_cast<double>(Registry[i][j].tmsec),
                        static_cast<double>(Registry[i][j].tusec));
                }
            }
        }
        fclose(pFile);        
    }
    return 0;
}

int MosquitoRegistry::loadFromFile(string filename, bool showProgress /*=false*/)
{
    vector<vector<int> > idMap;
    FILE * pFile;
    pFile = fopen (filename.c_str(),"r");
    fseek( pFile, 0, SEEK_END );
    int Size = ftell( pFile );
    rewind( pFile );
    QProgressDialog* DisplayProgress;
    bool haveDialog=false;
    if(showProgress)
    {
     DisplayProgress=new QProgressDialog("Loding Tracks...", "Cancel", 0, Size);
     DisplayProgress->setWindowModality(Qt::WindowModal);
     haveDialog=true;
    }
    double rx,ry,ri,ra,rd,rf,rfmin,rfmax,rid,rs,rm,ru;
    char line[512];
    while(fgets(line, sizeof(line), pFile))
    {
        int fields=sscanf(line,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                 &rx,
                 &ry,
                 &ri,
                 &ra,
                 &rd,
                 &rf,
                 &rfmin,
                 &rfmax,
                 &rid,
                 &rs,
                 &rm,
                 &ru);
        if(fields==12)         
        {
            int id=-1;
            for(int n=0;n<idMap.size();n++)
            if(idMap[n][0]==(int) rid)
            {
                id=idMap[n][1];
            }
            if(id==-1)
            {
                id=registerNewMosquito();
                vector<int> tmp;
                tmp.push_back((int) rid);
                tmp.push_back(id);
                idMap.push_back(tmp);
            }
            TrackPoint NewTrackPoint(rx,ry,rf,rs,rm,ru,
                                     ri,ra,rd,0,0,0,0,0);
            updateTrack(id,NewTrackPoint);
        }
        else
        {
         cerr<<"Error reading Track file! " <<fields<<" fields but I need 12!"<<endl;
        }
        if(showProgress)
        {
         int cSize = ftell( pFile );
         DisplayProgress->setValue(cSize);
         if (DisplayProgress->wasCanceled())
         {
          DisplayProgress->setValue(Size);
          delete DisplayProgress;
          haveDialog=false;
          break;
         }
        } 
    }
    if(haveDialog)
    {
      DisplayProgress->setValue(Size);
      delete DisplayProgress;
      haveDialog=false;    
    }

    fclose(pFile);
    return 0;
}

int MosquitoRegistry::reset()
{
    Registry.clear();
    return 0;
}

vector<TrackPoint> MosquitoRegistry::getVectorOfTrackPoints(int ID)
{
  vector<TrackPoint> idTrack;
  if(ID<Registry.size() && ID>=0) 
    idTrack=Registry[ID];
  return idTrack;   
}

int MosquitoRegistry::addVectorOfTrackPoints(vector<TrackPoint> t, int ofx, int ofy)
{

    if(!(ofx==0 && ofy==0))
     for(int i=0; i<t.size(); i++)
      t[i].move(ofx,ofy);
      
    Registry.push_back(t);
    int ID=Registry.size();
    return ID;
}

bool MosquitoRegistry::addVectorOfTrackPointsToTrack(int n, vector<TrackPoint> t, int ofx /*= 0*/, int ofy /*= 0*/)
{
    if(n<Registry.size())
    {
    if(!(ofx==0 && ofy==0))
     for(int i=0; i<t.size(); i++)
      t[i].move(ofx,ofy);
    for(int i=0; i<t.size(); i++)
     Registry[n].push_back(t[i]);
    sortTrack(n);
    return true;
    }
    return false;
}

int MosquitoRegistry::numOfTracks()
{
  return Registry.size();   
}

vector<vector<TrackPoint> > MosquitoRegistry::getRegistry()
{
  return Registry;
}

vector<int> MosquitoRegistry::findTrackID(double xi, double yi, double minDist, int fromFrame, int toFrame)
{
    vector<int> ret;
    double dist=minDist;
    int n=-1;
    int m=-1;
    for(unsigned long i=0; i<Registry.size();i++)
    {
        for(unsigned long j=0; j<Registry[i].size(); j++)
        {
            if(Registry[i][j].frame>=fromFrame && Registry[i][j].frame<toFrame)
            {
                double d=sqrt(pow(xi-Registry[i][j].x,2)+pow(yi-Registry[i][j].y,2));
                if(d<dist)
                {
                    n=i;
                    m=j;
                    dist=d;
                }
            }
        }  
    }
    if(n!=-1 && m!=-1)
    {
     ret.push_back(n);
     ret.push_back(m);
    }
    return ret;
}

vector<cv::Point2f>  MosquitoRegistry::getSingleTrack(int ID)
{
     vector<cv::Point2f> temp;
     if(ID<Registry.size() && ID>=0)
     {
      if(Registry[ID].size()>1)
      for(int j=0;j<Registry[ID].size();j++)
      {
        double x=Registry[ID][j].x;
        double y=Registry[ID][j].y;
        temp.push_back(cv::Point2f(x,y));  
      }
     }
     return temp;
}

bool  MosquitoRegistry::removePointFromTrack(int n, int m, bool split_here /*= false */)
{
 if(n<Registry.size())
 {
  if(m<Registry[n].size())
  {
   if(!split_here)
   {
    Registry[n].erase(Registry[n].begin()+m);
    if(Registry[n].size()==0)
     Registry.erase(Registry.begin()+n);
   }
   else
   {
    vector<TrackPoint> tmp=getVectorOfTrackPoints(n);
    Registry[n].erase(Registry[n].begin()+m,Registry[n].end());
    tmp.erase(tmp.begin(),tmp.begin()+m+1);
    if(tmp.size()>0)
    {
     int newID=addVectorOfTrackPoints(tmp, 0, 0);
    }
   }
  }
 }
}


bool  MosquitoRegistry::addPointToTrack(int n, TrackPoint a, bool replace /*= false*/)
{
 if(n<Registry.size())
 {
   bool newframe=true;
   int other=-1;
   for(unsigned int i=0; i<Registry[n].size(); i++)
    if(Registry[n][i].frame==a.frame)
    {
     newframe=false;
     other=i;
    }
   if(replace && !newframe)
   {
     Registry[n][other]=a;
     return true;
   }
   else if(newframe)
   {
     Registry[n].push_back(a);
     sortTrack(n);
   } 
 }
 return false;
}

bool MosquitoRegistry::joinTracks(int n,int m)
{
 if(n<Registry.size() && m<Registry.size() && m!=n && m>=0 && n>=0)
 {
  bool intersection=false;
  for(unsigned int i=0; i<Registry[n].size(); i++)
   for(unsigned int j=0; j<Registry[m].size(); j++)
    if(Registry[n][i].frame == Registry[m][j].frame)
    { 
      intersection=true;
      break;
    }
  if(!intersection)
  {
   vector<TrackPoint> tmp=getVectorOfTrackPoints(m);
   addVectorOfTrackPointsToTrack(n, tmp);
   Registry.erase(Registry.begin()+m);
   return true;
  }
  else
  {
   cerr<<"Error The two tracks intersect in time....\n";
   vector<unsigned long> fn;
   for(unsigned int i=0; i<Registry[n].size(); i++)
    fn.push_back(Registry[n][i].frame);
   for(unsigned int j=0; j<Registry[m].size(); j++)
    fn.push_back(Registry[m][j].frame);
   sort( fn.begin(), fn.end() );
   fn.erase( unique( fn.begin(), fn.end() ), fn.end() );
   cerr<<"Have "<<fn.size()<<" different time points\n";
   vector<vector<TrackPoint> > tmpT;
   for(unsigned int f=0; f<fn.size(); f++)
   {
    vector<TrackPoint> pt;
    tmpT.push_back(pt);
   }
   for(unsigned int i=0; i<Registry[n].size(); i++)
   {
    int cf=Registry[n][i].frame;
    int addr=-1;
    for(unsigned int j=0; j<fn.size(); j++)
     if(fn[j]==cf)
      {
        addr=j;
        break;
      }
      if(addr!=-1)
       tmpT[addr].push_back(Registry[n][i]);  
   }
   cerr<<"Filled tmp n\n";
   for(unsigned int i=0; i<Registry[m].size(); i++)
   {
    int cf=Registry[m][i].frame;
    int addr=-1;
    for(unsigned int j=0; j<fn.size(); j++)
     if(fn[j]==cf)
      {
        addr=j;
        break;
      }
    if(addr!=-1)
     tmpT[addr].push_back(Registry[m][i]);  
   }
   cerr<<"Filled tmp ,\n";
   int single=0;
   int multiple=0;
   for(unsigned int f=0; f<fn.size(); f++)
   {
    if(tmpT[f].size()==1) single++;
    else multiple++;
   }
   
   cerr<<"Tracks have "<<single<<" single timesteps and "<< multiple <<" unclear ones.\n";
   if(single<=2)
   {
    cerr<<"All points are intersecting, I think this are independent tracks!\n";
   }
   else
   {
    cerr<<"Trying to find best way:\n";
    if(tmpT[0].size()==1 && tmpT[fn.size()-1].size()==1)
    {
     vector<int> currentTrack;
     vector<int> selectedTrack;
     vector<bool> fixed;
     for(unsigned int f=0; f<fn.size(); f++)
     {
       if(tmpT[f].size()==1)
       {
        selectedTrack.push_back(0);
        fixed.push_back(true);
        currentTrack.push_back(0);
       }
       else 
       {
        selectedTrack.push_back(-1);
        currentTrack.push_back(-1);
        fixed.push_back(false);
       }
     }
     cerr<<"track finder initialized. Starting to loop...\n";
     bool notAtEnd=true;
     int pointer=0;
     while(notAtEnd)
     {
      int next=-1;
      for(unsigned int i=pointer+1; i<fn.size();i++)
       if(fixed[i]==true)
       {
        next=i;
        break;
       }
      if(next==pointer+1)
      {
       cerr<<"nothing to do for point "<<pointer<<" continue...\n";
      }
      else if(next==-1)
      {
       cerr<<"cant't find next fix point for "<<pointer<<" break...\n";
       notAtEnd=false;
      }
      else
      {
       cerr<<"working on "<<pointer<<" to "<<next<<"...\n";
       double minDist=-1;
       for(unsigned int i=pointer; i<=next; i++)
        if(!fixed[i])
         currentTrack[i]=0;
       vector<TrackPoint> t;
       for(unsigned int i=pointer; i<=next; i++)
        t.push_back(tmpT[i][currentTrack[i]]);
       double currDist=trackLength(t);
       minDist=currDist;
       cerr<<"MinDist="<<minDist<<"\n";
       for(unsigned int i=pointer; i<=next; i++)
        if(!fixed[i])
         selectedTrack[i]=currentTrack[i];
       bool x=true;
       while(x)
       {
        for(unsigned int i=pointer+1;i<next; i++)
        {
          currentTrack[i]++;
          if(currentTrack[i]>=tmpT[i].size())
          {
           cerr<<"I need to increase "<< i+1<<" \n";
           currentTrack[i]=0;
           if(i>next-2) {x=false;break;}
           currentTrack[i+1]++;           
          }
          else
          {
          cerr<<"I could successfully increase "<< i<<" \n";
          break;
          }
        }
        vector<TrackPoint>().swap(t);
        for(unsigned int i=pointer; i<=next; i++)
         t.push_back(tmpT[i][currentTrack[i]]);
        currDist=trackLength(t);
        if(minDist> currDist)
        {
         minDist=currDist;
         cerr<<"MinDist="<<minDist<<"\n";
         for(unsigned int i=pointer; i<=next; i++)
          if(!fixed[i])
           selectedTrack[i]=currentTrack[i];
        }
       }
       cerr<<"I have looped over all posibitlites for this segment...\n";
       if(minDist!=-1)
       {
        for(unsigned int i=pointer; i<=next; i++)
         if(!fixed[i])
          fixed[i]=true;
       }
      }      
      pointer=next;
      cerr<<"Pointer="<<pointer<<"\n";

      if(pointer>=tmpT.size()-1)
       notAtEnd=false;
     }
     bool allFixed=true;
     for(unsigned i=0; i<fn.size(); i++)
      if(!fixed[i])
      {
       allFixed=false;
       break;
      }
     if(allFixed)
     {
       vector<TrackPoint> t;
       for(unsigned int i=0; i<fn.size(); i++)
        t.push_back(tmpT[i][selectedTrack[i]]);
       if(m>n)
       {
        Registry.erase(Registry.begin()+m);
        Registry[n].swap(t);
       }
       else
       {
        Registry.erase(Registry.begin()+n);
        Registry[m].swap(t);
       }
       cerr<<"Track successfully joined!\n";
       return true;
     }
    }
    else
    {
     cerr<<"I need to have a fixed start and end point!\n!";
    }
   }
   cerr<<"Track NOT joined!\n";
   return false;
  }
 }
 cerr<<"Track NOT joined!\n";
 return false;
}

bool MosquitoRegistry::deleteTrack(int n)
{
 if(n<Registry.size())
 {
   Registry.erase(Registry.begin()+n);
   return true;
 }
 else
  return false;
}

bool MosquitoRegistry::splitTrack(int n,int m)
{
 if(n<Registry.size())
 {
  if(m<Registry[n].size()-1)
  {
    vector<TrackPoint> tmp=getVectorOfTrackPoints(n);
    Registry[n].erase(Registry[n].begin()+m+1,Registry[n].end());
    tmp.erase(tmp.begin(),tmp.begin()+m+1);
    if(tmp.size()>0)
    {
     int newID=addVectorOfTrackPoints(tmp, 0, 0);
    }
  }
 }
 return true;
}

bool MosquitoRegistry::sortTrack(int n)
{
 if(n<Registry.size())
 {
  sort(Registry[n].begin(), Registry[n].end(),
           [](TrackPoint a, TrackPoint b) { return a.frame < b.frame; });
 }
 return true;
}

bool MosquitoRegistry::autoJoinTracks(double maxiDistT, unsigned long maxTempDist, int remTooShort /*= 3*/, double angle /*= 0.5*/)
{
 cerr<<"Step1:\n";
 int n=0;
 QProgressDialog* DisplayProgress;
 bool haveDialog=true;
 DisplayProgress=new QProgressDialog("Autojoin Step 1 ...", "Cancel", 0, Registry.size());
 DisplayProgress->setWindowModality(Qt::WindowModal);

 for(unsigned int i=0; i<Registry.size(); i++)
 {
 DisplayProgress->setValue(i);
 if (DisplayProgress->wasCanceled())
 {
  DisplayProgress->setValue(Registry.size());
  delete DisplayProgress;
  haveDialog=false;
  break;
 }
 if(TrackLength(i)>1)
 {
  double startx=Registry[i][0].x;
  double starty=Registry[i][0].y;
  long int starti=Registry[i][0].frame;
  double stopx=Registry[i][Registry[i].size()-1].x;
  double stopy=Registry[i][Registry[i].size()-1].y;
  long int stopi=Registry[i][Registry[i].size()-1].frame;
  vector<int> candidates;
  vector<double> dist;
  vector<int> distT;
  for(unsigned int j=0; j<Registry.size(); j++)
   if(i!=j)
    if(TrackLength(j)>1)
  {
    if(double(Registry[j][0].frame)-stopi<maxTempDist && Registry[j][0].frame>stopi)
     if(sqrt(pow(Registry[j][0].x-stopx,2)+pow(Registry[j][0].y-stopy,2))<maxiDistT*double(Registry[j][0].frame-stopi))
    {
      candidates.push_back(j);
      dist.push_back(sqrt(pow(Registry[j][0].x-stopx,2)+pow(Registry[j][0].y-stopy,2)));
      distT.push_back((int)Registry[j][0].frame-stopi);
    }
    if(starti-double(Registry[j][Registry[j].size()-1].frame)<maxTempDist &&  starti>Registry[j][Registry[j].size()-1].frame) 
       if(sqrt(pow(Registry[j][Registry[j].size()-1].x-startx,2)+pow(Registry[j][Registry[j].size()-1].y-starty,2))<maxiDistT*double(starti-Registry[j][Registry[j].size()-1].frame))
    {
      candidates.push_back(j);
      dist.push_back(sqrt(pow(Registry[j][Registry[j].size()-1].x-startx,2)+pow(Registry[j][Registry[j].size()-1].y-starty,2)));
      distT.push_back((int)Registry[j][Registry[j].size()-1].frame-starti);
    }
  }
  double distend=99999;
  double diststart=99999;
  int iend=-1; 
  int istart=-1;
  
  for(unsigned j=0; j<candidates.size(); j++)
  {
   if(distT[j]>0)
   {
    if(dist[j]<distend)
    {
     distend=dist[j];
     iend=j;
    }
   }
   else
   {
    if(dist[j]<diststart)
    {
     diststart=dist[j];
     istart=j;
    }   
   }
  }
  
  if(istart!=-1 && iend!=-1)
  {
   if(candidates[istart]>candidates[iend])
   {
    if(i<candidates[istart])
    {
     bool ret=joinTracks(i,candidates[istart]);
     if(ret) n++;
     ret=joinTracks(i,candidates[iend]);
     if(ret) n++;
    }
    else
    {
     bool ret=joinTracks(i,candidates[istart]);
     if(ret)
     {
      n++;
      ret=joinTracks(i-1,candidates[iend]-1);
      if(ret)
       n++;
     }
     else
     {
      ret=joinTracks(i,candidates[iend]);
      if(ret)
       n++;
     }
    }
   }
   else
   {
    if(i<candidates[iend])
    {
     bool ret=joinTracks(i,candidates[iend]);
     if(ret) n++;
     ret=joinTracks(i,candidates[istart]);
     if(ret) n++;
    }
    else
    {
     bool ret=joinTracks(i,candidates[iend]);
     if(ret)
     {
      n++;
      ret=joinTracks(i-1,candidates[istart]-1);
      if(ret)n++;
     }
     else
     {
      ret=joinTracks(i,candidates[istart]);
      if(ret) n++;
     }
    }
   }
  }
  else if(istart!=-1)
  {
   bool ret=joinTracks(i,candidates[istart]);
   if(ret)n++;
  }
  else if(iend!=-1)
  {
   bool ret=joinTracks(i,candidates[iend]);
   if(ret)n++;
  }
 }
 }
 if(haveDialog)
 {
  DisplayProgress->setValue(Registry.size());
  delete DisplayProgress;
  haveDialog=false;
 }
 cerr<<"I have joined "<<n<<" Tracks!\n"; 

 cerr<<"Step1 (a):\n";
 n=0;
 haveDialog=true;
 DisplayProgress=new QProgressDialog("Autojoin Step 1a ...", "Cancel", 0, Registry.size());
 DisplayProgress->setWindowModality(Qt::WindowModal);

 for(unsigned int i=0; i<Registry.size(); i++)
 {
 DisplayProgress->setValue(i);
 if (DisplayProgress->wasCanceled())
 {
  DisplayProgress->setValue(Registry.size());
  delete DisplayProgress;
  haveDialog=false;
  break;
 }
 if(TrackLength(i)>2)
 {
  double stopx=Registry[i][Registry[i].size()-2].x;
  double stopy=Registry[i][Registry[i].size()-2].y;
  long int stopi=Registry[i][Registry[i].size()-2].frame;
  vector<int> candidates;
  vector<double> dist;
  vector<int> distT;
  for(unsigned int j=0; j<Registry.size(); j++)
   if(i!=j)
    if(TrackLength(j)>1)
  {
    if(double(Registry[j][0].frame)-stopi<maxTempDist && Registry[j][0].frame>stopi)
     if(sqrt(pow(Registry[j][0].x-stopx,2)+pow(Registry[j][0].y-stopy,2))<maxiDistT*double(Registry[j][0].frame-stopi))
    {
      candidates.push_back(j);
      dist.push_back(sqrt(pow(Registry[j][0].x-stopx,2)+pow(Registry[j][0].y-stopy,2)));
      distT.push_back((int)Registry[j][0].frame-stopi);
    }
  }
  double distend=99999;
  double diststart=99999;
  int iend=-1; 
  int istart=-1;
  
  for(unsigned j=0; j<candidates.size(); j++)
  {
   if(distT[j]>0)
   {
    if(dist[j]<distend)
    {
     distend=dist[j];
     iend=j;
    }
   }
   else
   {
    if(dist[j]<diststart)
    {
     diststart=dist[j];
     istart=j;
    }   
   }
  }
  
  if(istart!=-1 && iend!=-1)
  {
   if(candidates[istart]>candidates[iend])
   {
    if(i<candidates[istart])
    {
     bool ret=joinTracks(i,candidates[istart]);
     if(ret) n++;
     ret=joinTracks(i,candidates[iend]);
     if(ret) n++;
    }
    else
    {
     bool ret=joinTracks(i,candidates[istart]);
     if(ret)
     {
      n++;
      ret=joinTracks(i-1,candidates[iend]-1);
      if(ret) n++;
     }
     else
     {
      ret=joinTracks(i,candidates[iend]);
      if(ret) n++;     
     }
    }
   }
   else
   {
    if(i<candidates[iend])
    {
     bool ret=joinTracks(i,candidates[iend]);
     if(ret)n++;
     ret=joinTracks(i,candidates[istart]);
     if(ret)n++;
    }
    else
    {
     bool ret=joinTracks(i,candidates[iend]);
     if(ret)
     {
      n++;
      ret=joinTracks(i-1,candidates[istart]-1);
      if(ret) n++;
     }
     else
     {
      ret=joinTracks(i,candidates[istart]);
      if(ret) n++;
     }
    }
   }
  }
  else if(istart!=-1)
  {
   bool ret=joinTracks(i,candidates[istart]);
   if(ret)n++;
  }
  else if(iend!=-1)
  {
   bool ret=joinTracks(i,candidates[iend]);
   if(ret)n++;
  }
 }
 }
 if(haveDialog)
 {
  DisplayProgress->setValue(Registry.size());
  delete DisplayProgress;
  haveDialog=false;
 }

 cerr<<"I have joined "<<n<<" Tracks!\n"; 
 cerr<<"Step2:\n";
 n=0;
 haveDialog=true;
  DisplayProgress=new QProgressDialog("Autojoin Step 2 ...", "Cancel", 0, Registry.size());
 DisplayProgress->setWindowModality(Qt::WindowModal);

 for(unsigned int i=0; i<Registry.size(); i++)
 {
  DisplayProgress->setValue(i);
 if (DisplayProgress->wasCanceled())
 {
  DisplayProgress->setValue(Registry.size());
  delete DisplayProgress;
  haveDialog=false;
  break;
 }
 
  for(unsigned int j=0; j<Registry.size(); j++)
  {
   if(j!=i)
   {
    if(Registry[i][0].frame<=Registry[j][Registry[j].size()-1].frame && 
       Registry[i][Registry[i].size()-1].frame>=Registry[j][0].frame)
     {
      vector<double> distance=lineLineDistance(i,j);
      if(distance.size()==4)
       if(distance[3]>1)
      {
       if(distance[1]<maxiDistT)
        if(distance[0]<3)
        {
         if(joinTracks(i,j))
         {
          if(j<i)i--;
          n++;
          break;
         }
        }
      }
     }
   }
  } 
 }
 if(haveDialog)
 {
  DisplayProgress->setValue(Registry.size());
  delete DisplayProgress;
  haveDialog=false;
 }

 cerr<<"I have joined "<<n<<" overlapping Tracks!\n"; 
 
 cerr<<"Step3:\n";
 haveDialog=true;
 DisplayProgress=new QProgressDialog("Autojoin Step 3 ...", "Cancel", 0, Registry.size());
 DisplayProgress->setWindowModality(Qt::WindowModal);
 
 for(unsigned int i=0; i<Registry.size(); i++)
 {
 DisplayProgress->setValue(i);
 if (DisplayProgress->wasCanceled())
 {
  DisplayProgress->setValue(Registry.size());
  delete DisplayProgress;
  haveDialog=false;
  break;
 }
 if(TrackLength(i)>1)
 {
  vector<TrackPoint> filtered=removeSpikes(Registry[i],angle);
  if(filtered.size()<Registry[i].size())
   filtered.swap(Registry[i]);
 }
 }
 if(haveDialog)
 {
  DisplayProgress->setValue(Registry.size());
  delete DisplayProgress;
  haveDialog=false;
 }

 int tooShort=removeTooShort(remTooShort);
 cerr<<"Removing "<<tooShort<<" too short tracks(<="<<remTooShort<<")..\n";
 return false;
}

double MosquitoRegistry::trackLength(vector<TrackPoint> t)
{
 double tl=0;
 for(unsigned int i=0; i<t.size()-1; i++)
 {
  double segment=sqrt(pow(t[i].x-t[i+1].x,2)+pow(t[i].x-t[i+1].x,2));
  tl+=segment;
 } 
 return tl;
}

double MosquitoRegistry::calcDistFromLine(vector<TrackPoint> t)
{
 double dist=-1;
 if(t.size()==3)
 {
  double distAC=sqrt(pow(t[0].x-t[2].x,2)+pow(t[0].x-t[2].x,2));
  if(distAC>0)
   dist=((t[2].y-t[0].y)*t[1].x-(t[2].x-t[0].x)*t[1].y+t[2].x*t[0].y-t[2].y*t[0].x)/distAC;
 }
 else
 {
  cerr<<"Error: I need to have 3 Points!\n";
 }
 return abs(dist);
}

vector<TrackPoint> MosquitoRegistry::removeSpikes(vector<TrackPoint> t, double angle /*= 0.5*/)
{
 vector<double> a;
 a.push_back(0);
 for(unsigned int i=1; i<t.size()-1; i++)
 {
  double da=sqrt(pow(t[i].x-t[i+1].x,2)+pow(t[i].y-t[i+1].y,2));
  double db=sqrt(pow(t[i-1].x-t[i].x,2)+pow(t[i-1].y-t[i].y,2));
  double dc=sqrt(pow(t[i-1].x-t[i+1].x,2)+pow(t[i-1].y-t[i+1].y,2));
  if(da>0 && db>0)
  {
   double tmp = acos((da*da+db*db-dc*dc)/(2*da*db));
   a.push_back(tmp);
  }
 }
 a.push_back(0);

 for(unsigned int i=t.size()-2; i>0; i--)
 {
   if(a[i]<angle)
      t.erase(t.begin()+i);
 }
 return t;
}

int MosquitoRegistry::removeTooShort(int x)
{
 int n=0;
 int orig=(int)Registry.size();
 Registry.erase(
  std::remove_if(Registry.begin(), Registry.end(),
        [x](const vector<TrackPoint> & t) { return (t.size()<x); }),
    Registry.end());
 n=orig-(int)Registry.size();
 return n;
}

vector<double> MosquitoRegistry::lineLineDistance(unsigned int n,unsigned int m)
{
 vector<double> ret;
 if(n<Registry.size() && m<Registry.size() && m!=n && m>=0 && n>=0)
 {
   vector<unsigned long> fn;
   for(unsigned int i=0; i<Registry[n].size(); i++)
    fn.push_back(Registry[n][i].frame);
   for(unsigned int j=0; j<Registry[m].size(); j++)
    fn.push_back(Registry[m][j].frame);
   sort( fn.begin(), fn.end() );
   fn.erase( unique( fn.begin(), fn.end() ), fn.end() );
   vector<vector<TrackPoint> > tmpTA;
   vector<vector<TrackPoint> > tmpTB;
   vector<double> distances;
   for(unsigned int f=0; f<fn.size(); f++)
   {
    vector<TrackPoint> pt;
    tmpTA.push_back(pt);
    tmpTB.push_back(pt);
    distances.push_back(-1);
   }
   
   for(unsigned int i=0; i<Registry[n].size(); i++)
   {
    int cf=Registry[n][i].frame;
    int addr=-1;
    for(unsigned int j=0; j<fn.size(); j++)
     if(fn[j]==cf)
      {
        addr=j;
        break;
      }
      if(addr!=-1)
       tmpTA[addr].push_back(Registry[n][i]);  
   }
   
   for(unsigned int i=0; i<Registry[m].size(); i++)
   {
    int cf=Registry[m][i].frame;
    int addr=-1;
    for(unsigned int j=0; j<fn.size(); j++)
     if(fn[j]==cf)
      {
        addr=j;
        break;
      }
    if(addr!=-1)
     tmpTB[addr].push_back(Registry[m][i]);  
   }
   
   int startfn=Registry[n][0].frame;
   if(Registry[m][0].frame>startfn)
    startfn=Registry[m][0].frame;
   int stopfn=Registry[n][Registry[n].size()-1].frame;
   if(Registry[m][Registry[m].size()-1].frame<stopfn)
    stopfn=Registry[m][Registry[m].size()-1].frame;
   int imin=-1;
   int imax=-1;
   for(unsigned int j=0; j<fn.size(); j++)
    if(fn[j]==startfn)
     {
       imin=j;
       break;
     }
   for(unsigned int j=0; j<fn.size(); j++)
    if(fn[j]==stopfn)
     {
       imax=j;
       break;
     }     
   if(imax!=-1 && imin!=-1 && imin<=imax)
    {
     for(unsigned int f=imin; f<=imax; f++)
     {
       if(tmpTA[f].size()==1 && tmpTB[f].size()==1)
       {
        double tmpdist=sqrt(pow(tmpTA[f][0].x-tmpTB[f][0].x,2)+pow(tmpTA[f][0].x-tmpTB[f][0].x,2));
        distances[f]=tmpdist;
       }
       else if(tmpTA[f].size()==1 && tmpTB[f].size()==0)
       {
        vector<TrackPoint> tmp;
        int iprev=-1;
        for(int ii=f-1; ii>=0;ii--)
         if(tmpTB[ii].size()==1)
         {iprev=ii; break;} 
        int inext=-1;
        for(int ii=f+1; ii<fn.size();ii++)
         if(tmpTB[ii].size()==1)
         {inext=ii; break;}
        if(inext!=-1 && iprev!=-1)
        {
         tmp.push_back(tmpTB[iprev][0]);
         tmp.push_back(tmpTA[f][0]);
         tmp.push_back(tmpTB[inext][0]);
         double currDist=calcDistFromLine(tmp);
         distances[f]=currDist;
        }         
       }
       else if(tmpTA[f].size()==0 && tmpTB[f].size()==1)
       {
        vector<TrackPoint> tmp;
        int iprev=-1;
        for(int ii=f-1; ii>=0;ii--)
         if(tmpTA[ii].size()==1)
         {iprev=ii; break;} 
        int inext=-1;
        for(int ii=f+1; ii<fn.size();ii++)
         if(tmpTA[ii].size()==1)
         {inext=ii; break;}
        if(inext!=-1 && iprev!=-1)
        {
         tmp.push_back(tmpTA[iprev][0]);
         tmp.push_back(tmpTB[f][0]);
         tmp.push_back(tmpTA[inext][0]);
         double currDist=calcDistFromLine(tmp);
         distances[f]=currDist;
        }         
       }
      }
      double minDist=99999999;
      double maxDist=-1;
      double avgDist=0;
      int nDist=0;
      for(int i=0; i<distances.size(); i++)
       if(distances[i]!=-1)
       {
        if(distances[i]>maxDist) maxDist=distances[i];
        if(distances[i]<minDist) minDist=distances[i];
        avgDist+=distances[i];
        nDist++;
       }
      if(nDist>0)
      {
       ret.push_back(minDist);
       ret.push_back(maxDist);
       ret.push_back(avgDist/double(nDist));
       ret.push_back(double(nDist)); 
      }
    }
 }
 return ret;
}  

bool MosquitoRegistry::updateMosquitoPointer(MosquitoesInSeq* in)
{
 if(in)
 {
  for(unsigned int i=0; i<Registry.size(); i++)
   for(unsigned int j=0; j<Registry[i].size(); j++)
   {
    MosquitoesInFrame* pframe=in->mosqInFrameI(Registry[i][j].frame);
    if(pframe)
    {
     MosquitoPosition* ppos=pframe->mosqi(Registry[i][j].x, Registry[i][j].y);
     if(ppos)
     {
      Registry[i][j].setMosquitoPointer(ppos);
     }
    }
   }
  return true;
 }
 else
  return false;
}

bool MosquitoRegistry::connectRestingTracks(int maxDT, double maxDX)
{
 vector<vector<int> > possibleRests;
 for(int i=0; i<Registry.size(); i++)
 { 
   if(Registry[i].size()>1)
   {
    double ax=Registry[i][Registry[i].size()-1].x;
    double ay=Registry[i][Registry[i].size()-1].y;
    int af=Registry[i][Registry[i].size()-1].frame;
    for(int j=0; j<Registry.size(); j++)
     if(i!=j)
      if(Registry[j].size()>1)
     {
      double bx=Registry[j][Registry[j].size()-1].x;
      double by=Registry[j][Registry[j].size()-1].y;
      int bf=Registry[j][Registry[j].size()-1].frame;
      double dist=sqrt(pow(ax-bx,2)+pow(ay-by,2));
      int dtime=bf-af;
      if(dist<maxDX && dtime>0)
      {
       vector<int> tmp;
       tmp.push_back(i);
       tmp.push_back(j);
       possibleRests.push_back(tmp);
      }
     }
   }
 }
 vector<int> startIDs;
 vector<int> stopIDs;
 vector<int> multipleStartIDs;
 vector<int> multipleStopIDs;
 for(int i=0; i<possibleRests.size(); i++)
 {
  int found=0;
  for(int j=0; j<startIDs.size(); j++)
   if(startIDs[j]==possibleRests[i][0])
    found++;
  if(found==0)
   startIDs.push_back(possibleRests[i][0]);
  else if(found>1)
  {
   int found2=0;
   for(int j=0; j<multipleStartIDs.size(); j++)
    if(multipleStartIDs[j]==possibleRests[i][0])
     found2++;
   if(found2==0)
    multipleStartIDs.push_back(possibleRests[i][0]);
  }
 }

 for(int i=0; i<possibleRests.size(); i++)
 {
  int found=0;
  for(int j=0; j<stopIDs.size(); j++)
   if(stopIDs[j]==possibleRests[i][1])
    found++;
  if(found==0)
   stopIDs.push_back(possibleRests[i][1]);
  else if(found>1)
  {
   int found2=0;
   for(int j=0; j<multipleStopIDs.size(); j++)
    if(multipleStopIDs[j]==possibleRests[i][1])
     found2++;
   if(found2==0)
    multipleStopIDs.push_back(possibleRests[i][1]);
  }
 }
 
 if(multipleStopIDs.size()==0 && multipleStartIDs.size()==0)
 {
  for(int i=0; i<possibleRests.size(); i++)
  {
   cerr<<i<<":"<<possibleRests[i][0]<<"->"<<possibleRests[i][1]<<"\n";
  }
  for(int i=0; i<possibleRests.size(); i++)
  {
   bool ret=joinTracks(possibleRests[i][0],possibleRests[i][1]);
   for(int j=i+1; j<possibleRests.size(); j++)
   {
    if(possibleRests[j][0]==possibleRests[i][1])possibleRests[j][0]=possibleRests[i][1];
    if(possibleRests[j][0]>possibleRests[i][1])possibleRests[j][0]--;
    if(possibleRests[j][1]>possibleRests[i][1])possibleRests[j][1]--;
   }
  }  
 }
 else
 {
  for(int i=0; i<possibleRests.size(); i++)
  {
   int found=0;
   for(int j=0; j<multipleStopIDs.size(); j++)
    if(multipleStopIDs[j]==possibleRests[i][1])
     found++;
   for(int j=0; j<multipleStartIDs.size(); j++)
    if(multipleStartIDs[j]==possibleRests[i][0])
     found++;
   if(found==0)
    cerr<<i<<":"<<possibleRests[i][0]<<"->"<<possibleRests[i][1]<<"\n";
  }
  for(int i=0; i<possibleRests.size(); i++)
  {
   int found=0;
   for(int j=0; j<multipleStopIDs.size(); j++)
    if(multipleStopIDs[j]==possibleRests[i][1])
     found++;
   for(int j=0; j<multipleStartIDs.size(); j++)
    if(multipleStartIDs[j]==possibleRests[i][0])
     found++;
   if(found==0)
   {
    joinTracks(possibleRests[i][0],possibleRests[i][1]);
    for(int j=i+1; j<possibleRests.size(); j++)
    {
     if(possibleRests[j][0]==possibleRests[i][1])possibleRests[j][0]=possibleRests[i][1];
     if(possibleRests[j][0]>possibleRests[i][1])possibleRests[j][0]--;
     if(possibleRests[j][1]>possibleRests[i][1])possibleRests[j][1]--;
    }
    for(int j=0; j<multipleStartIDs.size(); j++)
    {
     if(multipleStartIDs[j]==possibleRests[i][1])multipleStartIDs[j]=possibleRests[i][1];
     if(multipleStartIDs[j]>possibleRests[i][1])multipleStartIDs[j]--;
    }
    for(int j=0; j<multipleStopIDs.size(); j++)
    {
     if(multipleStopIDs[j]==possibleRests[i][1])multipleStopIDs[j]=possibleRests[i][1];
     if(multipleStopIDs[j]>possibleRests[i][1])multipleStopIDs[j]--;
    }
   }
  }
 }
}

vector<vector<double> > MosquitoRegistry::getDataForTrack(unsigned long n, int dataType)
{
 vector<vector<double> > tmp;
 if(dataType==0)
 {
   for(int i=0; i<Registry[n].size()-1; i++)
   {
    double dist=sqrt(pow(Registry[n][i+1].x-Registry[n][i].x,2)+pow(Registry[n][i+1].y-Registry[n][i].y,2));
    double dt=Registry[n][i+1].time()-Registry[n][i].time();
    double tt=(Registry[n][i+1].time()+Registry[n][i].time())/2.;
    cerr<<"Points to draw:\n";
    if(dt>0)
    {
    vector<double> v;
    v.push_back(tt);
    v.push_back(dist/dt);
    cerr<<tt<<" , "<<dist/dt<<"\n";
    tmp.push_back(v);
    }
   }
 }
 return tmp; 
}

bool MosquitoRegistry::fixTime(MosquitoesInSeq* in)
{
 if(in)
 {
  for(unsigned int i=0; i<Registry.size(); i++)
   for(unsigned int j=0; j<Registry[i].size(); j++)
   {
    MosquitoesInFrame* pframe=in->mosqInFrameI(Registry[i][j].frame);
    if(pframe)
    {
     vector<int> ttt=pframe->getTime();
     if(ttt.size()==3)
     {
      Registry[i][j].tsec=ttt[0];
      Registry[i][j].tmsec=ttt[1];
      Registry[i][j].tusec=ttt[2];
     }
    }
   }
  return true;
 }
 else
  return false;
}


int MosquitoRegistry::markMosquitoesToKeep()
{
 for(unsigned int i=0; i<Registry.size(); i++)
  for(unsigned int j=0; j<Registry[i].size(); j++)
 {
  MosquitoPosition* mp=Registry[i][j].getMosquitoPointer();
  if(mp)
  {
   mp->setFlag(MosquitoPosition::flagType::KEEPME);
  } 
 }
 return 0;
}

bool MosquitoRegistry::estimateNextPosition(int i,double &ox, double &oy, unsigned long &on, double scalefactor /*=1.0*/)
{
 if(i>0 && i<Registry.size())
 {
  if(Registry[i].size()==2)
  {
   double dx=Registry[i][Registry[i].size()-1].x-Registry[i][Registry[i].size()-2].x;
   double dy=Registry[i][Registry[i].size()-1].y-Registry[i][Registry[i].size()-2].y; 
   double dt=double(Registry[i][Registry[i].size()-1].tsec)+double(Registry[i][Registry[i].size()-1].tmsec)/1000.
             +double(Registry[i][Registry[i].size()-1].tusec)/1000000.-
             (double(Registry[i][Registry[i].size()-2].tsec)+double(Registry[i][Registry[i].size()-2].tmsec)/1000.
             +double(Registry[i][Registry[i].size()-2].tusec)/1000000.);
   double df=double(Registry[i][Registry[i].size()-1].frame)-double(Registry[i][Registry[i].size()-2].frame);
   ox=Registry[i][Registry[i].size()-1].x+dx/df*scalefactor;
   oy=Registry[i][Registry[i].size()-1].y+dy/df*scalefactor;
   on=Registry[i][Registry[i].size()-1].frame+1;
   cerr<<"ox= "<<Registry[i][Registry[i].size()-1].x<<" + "<<dx <<"/"<<df<<"\n";
   cerr<<"oy= "<<Registry[i][Registry[i].size()-1].y<<" + "<<dy <<"/"<<df<<"\n";
   return true;  
  }
  else if(Registry[i].size()>2)
  {
   double dx=Registry[i][Registry[i].size()-1].x-Registry[i][Registry[i].size()-2].x;
   double dy=Registry[i][Registry[i].size()-1].y-Registry[i][Registry[i].size()-2].y; 
   double dt=double(Registry[i][Registry[i].size()-1].tsec)+double(Registry[i][Registry[i].size()-1].tmsec)/1000.
             +double(Registry[i][Registry[i].size()-1].tusec)/1000000.-
             (double(Registry[i][Registry[i].size()-2].tsec)+double(Registry[i][Registry[i].size()-2].tmsec)/1000.
             +double(Registry[i][Registry[i].size()-2].tusec)/1000000.);
   double df=double(Registry[i][Registry[i].size()-1].frame)-double(Registry[i][Registry[i].size()-2].frame);
   double dx2=Registry[i][Registry[i].size()-2].x-Registry[i][Registry[i].size()-3].x;
   double dy2=Registry[i][Registry[i].size()-2].y-Registry[i][Registry[i].size()-3].y;
   double dt2=double(Registry[i][Registry[i].size()-2].tsec)+double(Registry[i][Registry[i].size()-2].tmsec)/1000.
             +double(Registry[i][Registry[i].size()-2].tusec)/1000000.-
             (double(Registry[i][Registry[i].size()-3].tsec)+double(Registry[i][Registry[i].size()-3].tmsec)/1000.
             +double(Registry[i][Registry[i].size()-3].tusec)/1000000.);
   double dt3=double(Registry[i][Registry[i].size()-1].tsec)+double(Registry[i][Registry[i].size()-1].tmsec)/1000.
             +double(Registry[i][Registry[i].size()-1].tusec)/1000000.-
             (double(Registry[i][Registry[i].size()-3].tsec)+double(Registry[i][Registry[i].size()-3].tmsec)/1000.
             +double(Registry[i][Registry[i].size()-3].tusec)/1000000.);
   double ax=((dx/dt)-(dx2/dt2))/dt3;
   double ay=((dy/dt)-(dy2/dt2))/dt3;
   ox=Registry[i][Registry[i].size()-1].x+dx/df*scalefactor+ax*dt/df*dt/df*scalefactor;
   oy=Registry[i][Registry[i].size()-1].y+dy/df*scalefactor+ay*dt/df*dt/df*scalefactor;
   cerr<<"ox= "<<Registry[i][Registry[i].size()-1].x<<" + "<<dx <<"/"<<df<<" + "<<ax<<"*("<<dt<<"/"<<df<<")^2\n";
   cerr<<"oy= "<<Registry[i][Registry[i].size()-1].y<<" + "<<dy <<"/"<<df<<" + "<<ay<<"*("<<dt<<"/"<<df<<")^2\n";
   on=Registry[i][Registry[i].size()-1].frame+1;
   return true;
  }
  else
  {
   return false;
  }
  
 }
 else
  false;
}

bool MosquitoRegistry::estimatePrevPosition(int i,double &ox, double &oy, unsigned long &on, double scalefactor /*=1.0*/)
{
 if(i>0 && i<Registry.size())
 {
  if(Registry[i].size()==2)
  {
   double dx=Registry[i][0].x-Registry[i][1].x;
   double dy=Registry[i][0].y-Registry[i][1].y; 
   double dt=Registry[i][0].time()-Registry[i][1].time();
   double df=double(Registry[i][0].frame)-double(Registry[i][1].frame);
   ox=Registry[i][0].x-dx/df*scalefactor;
   oy=Registry[i][0].y-dy/df*scalefactor;
   on=Registry[i][0].frame-1;
   return true;  
  }
  else if(Registry[i].size()>2)
  {
   double dx=Registry[i][0].x-Registry[i][1].x;
   double dy=Registry[i][0].y-Registry[i][1].y; 
   double dt=Registry[i][0].time()-Registry[i][1].time();
   double df=double(Registry[i][0].frame)-double(Registry[i][1].frame);
   double dx2=Registry[i][1].x-Registry[i][2].x;
   double dy2=Registry[i][1].y-Registry[i][2].y;
   double dt2=Registry[i][1].time()-Registry[i][2].time();
   double dt3=Registry[i][0].time()-Registry[i][2].time();
   double ax=((dx/dt)-(dx2/dt2))/dt3;
   double ay=((dy/dt)-(dy2/dt2))/dt3;
   ox=Registry[i][0].x-dx/df*scalefactor-ax*dt/df*dt/df*scalefactor;
   oy=Registry[i][0].y-dy/df*scalefactor-ay*dt/df*dt/df*scalefactor;
   on=Registry[i][0].frame+1;
   return true;
  }
  else
  {
   return false;
  }
 }
 else
  false;
}

vector<vector<TrackPoint> > MosquitoRegistry::getMissingPieces(int n)
{
 vector<vector<TrackPoint> > tmp;
 if(n<Registry.size() && n>=0)
 {
  for(int m=0; m<Registry[n].size()-1; m++)
  {
   if(Registry[n][m+1].frame-Registry[n][m].frame>1)
   {
    vector<TrackPoint> t;
    t.push_back(Registry[n][m]);
    t.push_back(Registry[n][m+1]);
    tmp.push_back(t);
   }
  }
 }
 return tmp;
}

bool MosquitoRegistry::copyTo(MosquitoRegistry& a)
{
    a.Registry=Registry;
    return true;
}

vector<vector<cv::Scalar> > MosquitoRegistry::getTrackMetaForAni(unsigned long frame,int lastn, int mint, int colorByFlag, double startFrame, double endFrame)
{
    // 0 fixed color
    // 1 color track by first occurence
    // 2 color by track ID(rainbow)
    // 3 color by track ID(random)
    // 4 rainbow tail...
    // 5 color by time
    vector<vector<cv::Scalar> > tmp;
    srand (static_cast <unsigned> (time(0)));
    for(unsigned long i=0; i<Registry.size();i++)
    {
        int displayTrack=0;
        int firstPoint=-1;
        int lastPoint=-1;
        for(unsigned long j=0; j<Registry[i].size(); j++)
        if(Registry[i][j].frame>(int)frame-(int)lastn && Registry[i][j].frame<=frame)
        {
           if(firstPoint==-1) firstPoint=j;
           lastPoint=j;
           displayTrack++;
        }
        if(displayTrack>=mint)
        {
         vector<cv::Scalar> tmp2;
         if(colorByFlag==0 || colorByFlag<0 || colorByFlag>5)
         {
          for(unsigned long j=0; j<displayTrack; j++)
           tmp2.push_back(cv::Scalar(255,255,255));
         }
         else if(colorByFlag==1)
         {
          cv::Scalar color=getColor((double)Registry[i][0].frame,startFrame,endFrame);
          for(unsigned long j=0; j<displayTrack; j++)
           tmp2.push_back(color);
         }
         else if(colorByFlag==2)
         {
          cv::Scalar color=getColor((double)i,0.0,(double)Registry.size());
          for(unsigned long j=0; j<displayTrack; j++)
           tmp2.push_back(color);
         }
         else if(colorByFlag==3)
         {
          cv::Scalar color=getRandomColor(i);
          for(unsigned long j=0; j<displayTrack; j++)
           tmp2.push_back(color);
         }
         else if(colorByFlag==4)
         {
          for(unsigned long j=firstPoint; j<=lastPoint; j++)
          {
           cv::Scalar color=getColor((double)(frame-Registry[i][j].frame),0,(double)lastn);
           tmp2.push_back(color);
          }
         }
         else if(colorByFlag==5)
         {
          for(unsigned long j=firstPoint; j<lastPoint; j++)
          {
           cv::Scalar color=getColor(Registry[i][j].time(),startFrame,endFrame);         
           tmp2.push_back(color);
          }
         }
         tmp.push_back(tmp2); 
        }
    }  
    return tmp;
}

cv::Scalar MosquitoRegistry::getRandomColor(int i)
{
    unsigned int nSeed = 5323;
    for(int j=0; j<i; j++)
    {
     nSeed = (8253729 * nSeed + 2396403); 
    }
    int random=nSeed  % 32767;
    cv::Scalar color=getColor(double(random), 0., 32766.);
    return color;       
}

cv::Scalar MosquitoRegistry::getColor(double i, double from, double to)
{
  double v=(i-from)/(to-from);
  double r;
  double g;
  double b;
  if(v<0 || v>1)
  {
    r=0;
    b=0;
    g=0;
  }
  else
  {
    double cx=v*8.;
    if(cx<1)
    {
      r=0;
      g=0;
      b=(128+cx*128)/255.;
    }
    else if(cx<3)
    {
      r=0;
      g=(256*(cx-1.)/2.)/255.;
      b=1;
    }
    else if(cx<5)
    {
      r=(256*(cx-3.)/2.)/255.;
      g=1;
      b=(255-256*(cx-3.)/2.)/255.;
    }
    else if(cx<7)
    {
      r=1;
      g=(255-256*(cx-5.)/2.)/255.;
      b=0;
    }
    else
    {
      r=(255-256*(cx-7.)/2.)/255.;
      g=0;
      b=0;
    }  
  }
  r*=255.;
  g*=255.;
  b*=255.;
  return cv::Scalar((int)b,(int)g,(int)r);
}

vector<vector<double> > MosquitoRegistry::trackStatistics(vector<TrackPoint> cT)
{
 vector<vector<double> > a;
 if(cT.size()>1)
 {
 double t0=cT[0].time();
 double t1=cT[cT.size()-1].time();
 double pl=0;
 double means=0;
 double meanvx=0;
 double meanvy=0;
 vector<cv::Point3d> v;
 for(int i=0; i<cT.size()-1; i++)
 {
   double dx=cT[i+1].x-cT[i].x;
   double dy=cT[i+1].y-cT[i].y;
   double dt=cT[i+1].time()-cT[i].time();
   double d=sqrt(dx*dx+dy*dy);
   pl+=d;
   v.push_back(cv::Point3d(dx,dy,dt));
   means+=d/dt;
   meanvx+=dx/dt;
   meanvy+=dy/dt;
 }
 double avgs=pl/(t1-t0);
 means/=cT.size()-1;
 meanvx/=cT.size()-1;
 meanvy/=cT.size()-1;
 double stdmeans=0;
 double stdmeanvx=0;
 double stdmeanvy=0;
 double mins=means;
 double maxs=means;
 double minvx=meanvx;
 double maxvx=meanvx;
 double minvy=meanvy;
 double maxvy=meanvy;
 if(cT.size()>2)
 {
 for(int i=0; i<v.size(); i++)
 {
  double currS=sqrt(v[i].x*v[i].x+v[i].y*v[i].y)/v[i].z;
  stdmeans+=pow(currS-means,2);
  stdmeanvx+=pow(v[i].x/v[i].z-meanvx,2);
  stdmeanvy+=pow(v[i].y/v[i].z-meanvy,2);  
  if(currS>maxs)maxs=currS;
  if(currS<mins)mins=currS;
  if(v[i].x/v[i].z>maxvx) maxvx=v[i].x/v[i].z;
  if(v[i].x/v[i].z<minvx) minvx=v[i].x/v[i].z;
  if(v[i].y/v[i].z>maxvy) maxvy=v[i].y/v[i].z;
  if(v[i].y/v[i].z<maxvy) minvy=v[i].y/v[i].z;
 }
 stdmeans=sqrt(stdmeans/(cT.size()-1));
 stdmeanvx=sqrt(stdmeanvx/(cT.size()-1));
 stdmeanvy=sqrt(stdmeanvy/(cT.size()-1));
 }
 vector<double> tmp;
 tmp.push_back(avgs);
 a.push_back(tmp);
 vector<double>().swap(tmp);
 if(cT.size()>2)
 {
 tmp.push_back(means);
 a.push_back(tmp);
 vector<double>().swap(tmp);
 tmp.push_back(stdmeans);
 a.push_back(tmp);
 vector<double>().swap(tmp);
 tmp.push_back(mins);
 tmp.push_back(maxs);
 a.push_back(tmp);
 vector<double>().swap(tmp); 
 }
 tmp.push_back(meanvx);
 tmp.push_back(meanvy);
 a.push_back(tmp);
 vector<double>().swap(tmp); 
 if(cT.size()>2)
 {
 tmp.push_back(stdmeanvx);
 tmp.push_back(stdmeanvy);
 a.push_back(tmp);
 vector<double>().swap(tmp); 
 tmp.push_back(minvx);
 tmp.push_back(maxvx);
 tmp.push_back(minvy);
 tmp.push_back(maxvy);
 a.push_back(tmp);
 vector<double>().swap(tmp);
 }
 }
 return a; 
}

bool MosquitoRegistry::joinBasedOnStatistics(int n, int m)
{
 if(n<0 || n>Registry.size() || m<0 || m>Registry.size())
  return false;
 double conns=-1;
 double connvx=-1;
 double connvy=-1;
 int blahh=-1;
 if(Registry[n][Registry[n].size()-1].time()-Registry[m][0].time()>0)
 {
  double dt=Registry[n][Registry[n].size()-1].time()-Registry[m][0].time();
  conns=sqrt(pow(Registry[m][0].x-Registry[n][Registry[n].size()-1].x,2)+pow(Registry[m][0].y-Registry[n][Registry[n].size()-1].y,2))/dt;
  connvx=(Registry[n][Registry[n].size()-1].x-Registry[m][0].x)/dt;
  connvy=(Registry[n][Registry[n].size()-1].y-Registry[m][0].y)/dt;
  blahh=0;
 }
 else if(Registry[m][Registry[m].size()-1].time()-Registry[n][0].time()>0)
 {
  double dt=Registry[m][Registry[m].size()-1].time()-Registry[n][0].time();
  conns=sqrt(pow(Registry[n][0].x-Registry[m][Registry[m].size()-1].x,2)+pow(Registry[n][0].y-Registry[m][Registry[m].size()-1].y,2))/dt;
  connvx=(Registry[m][Registry[m].size()-1].x-Registry[n][0].x)/dt;
  connvy=(Registry[m][Registry[m].size()-1].y-Registry[n][0].y)/dt;
  blahh=1;
 }
 else
 {
  cerr<<"Tracks du intersect...\n";
  return false;
 }
 vector<vector<double> > aN=trackStatistics(getVectorOfTrackPoints(n));
 vector<vector<double> > aM=trackStatistics(getVectorOfTrackPoints(m));
 double allowDifferenceOf=0.1;
 double allowRotationOf=45./180.*CV_PI;
 if(aN.size()==0 || aM.size()==0)
  return false;
 double ans=0;
 double anvx=0;
 double anvy=0;
 double ams=0;
 double amvx=0;
 double amvy=0;
 if(aN.size()==7)
 {
  ans=aN[0][0];
  anvx=aN[4][0];
  anvy=aN[4][1];
 }
 else if(aN.size()==2)
 {
  ans=aN[0][0];
  anvx=aN[1][0];
  anvy=aN[1][1];
 }
 
 if(aM.size()==7)
 {
  ams=aM[0][0];
  amvx=aM[4][0];
  amvy=aM[4][1];
 }
 else if(aM.size()==2)
 {
  ams=aM[0][0];
  amvx=aM[1][0];
  amvy=aM[1][1];
 }
 double avgs=(ans+ams)/2;
 double normn=sqrt(anvx*anvx+anvy*anvy);
 double normm=sqrt(amvx*amvx+amvy*amvy);
 if(normm==0 or normn==0) return false;
 double dAngle=atan2(amvy/normm,amvx/normm) - atan2(anvy/normn,anvx/normn);
 if(dAngle<0) dAngle = -dAngle;
 if(dAngle > CV_PI) dAngle = 2. * CV_PI - dAngle;
 cerr<<"Connection of "<<n<<" and "<<m<<":\n";
 cerr<<"avgs="<<avgs<<"\n"<<"dAngle="<<dAngle/CV_PI*180.<<"\n";
 cerr<< "conn: speed="<<conns<< " v=("<<connvx<<","<<connvy<<")\n"; 
 bool possible=true;
 if(avgs*(1+allowDifferenceOf)<conns)
  possible=false;
 if(avgs*(1-allowDifferenceOf)>conns)
  possible=false;
 if(dAngle<allowRotationOf)
  possible=false;
 double acceleration=0;
 if(blahh==0)
 {
  double dv1x=(Registry[n][Registry[n].size()-1].x-Registry[n][Registry[n].size()-2].x)/(Registry[n][Registry[n].size()-1].time()-Registry[n][Registry[n].size()-2].time());
  double dv1y=(Registry[n][Registry[n].size()-1].y-Registry[n][Registry[n].size()-2].y)/(Registry[n][Registry[n].size()-1].time()-Registry[n][Registry[n].size()-2].time());
  double dv2x=(Registry[m][0].x-Registry[m][1].x)/(Registry[m][0].time()-Registry[m][1].time());
  double dv2y=(Registry[m][0].y-Registry[m][1].y)/(Registry[m][0].time()-Registry[m][1].time());
  double ax=(dv2x-dv1x)/(Registry[m][0].time()-Registry[n][Registry[n].size()-1].time());
  double ay=(dv2y-dv1y)/(Registry[m][0].time()-Registry[n][Registry[n].size()-1].time());
  acceleration=sqrt(ax*ax+ay*ay);
 }
 else if(blahh==1)
 {
  double dv1x=(Registry[m][Registry[m].size()-1].x-Registry[m][Registry[m].size()-2].x)/(Registry[m][Registry[m].size()-1].time()-Registry[m][Registry[m].size()-2].time());
  double dv1y=(Registry[m][Registry[m].size()-1].y-Registry[m][Registry[m].size()-2].y)/(Registry[m][Registry[m].size()-1].time()-Registry[m][Registry[m].size()-2].time());
  double dv2x=(Registry[n][0].x-Registry[n][1].x)/(Registry[n][0].time()-Registry[n][1].time());
  double dv2y=(Registry[n][0].y-Registry[n][1].y)/(Registry[n][0].time()-Registry[n][1].time());
  double ax=(dv2x-dv1x)/(Registry[n][0].time()-Registry[m][Registry[m].size()-1].time());
  double ay=(dv2y-dv1y)/(Registry[n][0].time()-Registry[m][Registry[m].size()-1].time());
  acceleration=sqrt(ax*ax+ay*ay);
 }
 cerr<<"acceleration="<<acceleration<<"\n";
 return possible;  
}

