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
#ifndef VIEWERWITHMOUSE_H
#define VIEWERWITHMOUSE_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <vector>
#include <QVector>
#include <QPointF>

#include "Viewer.h"

class ViewerWithMouse : public Viewer
{
    Q_OBJECT
public:

protected:
    void 	mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void 	mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void 	mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    QPoint m;
    QPoint mm;
    Qt::MouseButton mouseButton;
};

#endif // VIEWERWITHMOUSE_H
 