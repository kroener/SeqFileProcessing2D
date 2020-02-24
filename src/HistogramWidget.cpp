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
#include "HistogramWidget.h"

#include <QPainter>
#include <vector>
#include <iostream>
HistogramWidget::HistogramWidget(QWidget *parent)
    : QWidget(parent)
{
    fixX=0;
    fixY=0;
    fixXmin=0;
    fixXmax=0;
    fixYmin=0;
    fixYmax=0;
    fracN=0;
    minT=0;        
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

QSize HistogramWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize HistogramWidget::sizeHint() const
{
    return QSize(400, 200);
}

void HistogramWidget::setData(std::vector<std::vector<int> > hist, int maxI)
{
    Hist=hist;
    maxi=maxI;
    update();
}

void HistogramWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    if(Hist.size()>0)
    {
        int xmax=Hist[Hist.size()-1][0]+1;
        int ymin=0;
        int ymax=Hist[maxi][1];
        double xscale=1;
        double yscale=1;
        int H=painter.device()->height();
        int W=painter.device()->width();
        if(W> 0 && H>0)
        {
            xscale=double(0.4*W)/double(xmax);
            yscale=double(0.8*H)/double(ymax-ymin);
            double x0= -xmax-1./8.*double(2*xmax);
            double y0= ymin-1./8.*double(ymax-ymin);
            pen.setBrush(Qt::black);
            painter.setPen(pen);
            for(unsigned long i=0; i<Hist.size(); i++)
            {
                painter.drawRect(QRect(
                                     double(Hist[i][0]-0.5-x0)*xscale,
                                     (double(ymax-Hist[i][1])-y0)*yscale,
                                     (1.0)*xscale,
                                     double(Hist[i][1])*yscale));
            }
                pen.setBrush(Qt::green);
                painter.setPen(pen);
                painter.drawLine(QLineF((0-x0)*xscale,0,(0-x0)*xscale,H));
                pen.setBrush(Qt::cyan);
                painter.setPen(pen);
                painter.drawLine(QLineF((Hist[maxi][0]-x0)*xscale,0,(Hist[maxi][0]-x0)*xscale,H));
                if(fracN!=0)
                {
                pen.setBrush(Qt::red);
                painter.setPen(pen);
                painter.drawLine(QLineF((fracN*(xmax-Hist[maxi][0])+Hist[maxi][0]-x0)*xscale,0,(fracN*(xmax-Hist[maxi][0])+Hist[maxi][0]-x0)*xscale,H));
                }
                pen.setBrush(Qt::magenta);
                painter.setPen(pen);
                painter.drawLine(QLineF((xmax-x0)*xscale,0,(xmax-x0)*xscale,H));
                if(minT!=0)
                {
                pen.setBrush(Qt::blue);
                painter.setPen(pen);
                painter.drawLine(QLineF((minT-x0)*xscale,0,(minT-x0)*xscale,H));
                }
        }
    }
}

void HistogramWidget::SetP(double th,double thmin)
{
   fracN=th;
   minT=thmin;
}
