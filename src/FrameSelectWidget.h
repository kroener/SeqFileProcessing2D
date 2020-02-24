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
#ifndef FRAMESELECTWIDGET_H
#define FRAMESELECTWIDGET_H
#include "Header.hpp"
#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
class QLabel;
class QSpinBox;
class QDoubleSpinBox;
/** 
  * \brief Widget to select Range of *.seq file
  */
class FrameSelectWidget : public QWidget
{

    Q_OBJECT

public:
    FrameSelectWidget();
    bool getMetaMode();

signals:
    void toValueChanged(int);
    void fromValueChanged(int);
    void stepValueChanged(int);

public slots:
    bool update(Header* header);
    void toChanged(int i);
    void fromChanged(int i);
    void stepChanged(int i);
    unsigned long getFrom();
    unsigned long getTo();
    unsigned long getStep();
    void setTo(int i);
    void setFrom(int i);
    void setToFrame(int i);
    void setFromFrame(int i);
    void setFrameStep(int i);
    void setMin(int i);
    void setMax(int i);
    void metaMode(bool b);
    void unsetMetaMode();

private:
    QSpinBox *fromSelectParam;
    QSpinBox *toSelectParam;
    QSpinBox *stepSelectParam;
    bool useMeta;
    unsigned long frames;
};
#endif // FRAMESELECTWIDGET_H  
