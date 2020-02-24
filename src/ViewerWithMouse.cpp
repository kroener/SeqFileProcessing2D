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
#include "ViewerWithMouse.h"
#include <iostream>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <vector>

void ViewerWithMouse::mousePressEvent(QMouseEvent *event)
{
    m = event->globalPos();
    mm = event->globalPos();
    mouseButton = event->button();
}

void ViewerWithMouse::mouseMoveEvent(QMouseEvent *event)
{
    QPointF x=posToImgCoords(mm);
    QPointF xx=posToImgCoords(m);
    QPointF y=posToImgCoords(event->globalPos());
    if(mouseButton==Qt::LeftButton)
    {
        moveCrop((int)(x.x()-y.x()),(int)(x.y()-y.y()));
    }
    else if(mouseButton==Qt::RightButton)
    {
        setSelection(xx.x(),xx.y(),y.x(),y.y());
    }
    mm=event->globalPos();
}

void ViewerWithMouse::mouseReleaseEvent(QMouseEvent *event)
{
   QPointF x=posToImgCoords(m);
   QPointF y=posToImgCoords(event->globalPos());
   if(event->button()==Qt::RightButton)
   {
   if(x.x()<y.x() && x.y()<y.y())
    setCrop((int)x.x(),(int)x.y(),(int)(y.x()-x.x()),(int)(y.y()-x.y()));
   else if(x.x()>=y.x() && x.y()<y.y())
    setCrop((int)y.x(),(int)x.y(),(int)(x.x()-y.x()),(int)(y.y()-x.y()));
   else if(x.x()<y.x() && x.y()>=y.y())
    setCrop((int)x.x(),(int)y.y(),(int)(y.x()-x.x()),(int)(x.y()-y.y()));
   else if(x.x()>=y.x() && x.y()>=y.y())
    setCrop((int)y.x(),(int)y.y(),(int)(x.x()-y.x()),(int)(x.y()-y.y()));
   clearSelection();
   }
}

