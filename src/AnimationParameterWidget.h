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
#ifndef ANIMATIONPARAMETERWIDGET_H
#define ANIMATIONPARAMETERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
class QLabel;
class QPushButton;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;

class AnimationParameterWidget : public QWidget
{

    Q_OBJECT

public:
    AnimationParameterWidget();

signals:
    void onCreateAnimationClicked();

public slots:
    bool getDrawPoints();
    bool getDrawTracks();
    bool getDrawTime();
    bool getScaleOutput();
    int getOutputWidth();
    int getColorPointR();
    int getColorPointG();
    int getColorPointB();
    int getColorTrackR();
    int getColorTrackG();
    int getColorTrackB();
    int getPointRadius();
    int getTrackThickness();
    int getTrackLength();    
    int getTrackMinLength();    
    void createAnimationClicked();    
    int getTrackColorMode();
    
private:
    //Checkboxes:
    QCheckBox *drawPoints;
    QCheckBox *drawTracks;
    QCheckBox *drawTime;
    QCheckBox *scaleOutput;
    //Spin Boxes:
    QSpinBox *outputWidth;
    QSpinBox *colorPointR;
    QSpinBox *colorPointG;
    QSpinBox *colorPointB;
    QSpinBox *colorTrackR;
    QSpinBox *colorTrackG;
    QSpinBox *colorTrackB;
    QSpinBox *pointRadius;
    QSpinBox *trackThickness;
    QSpinBox *trackLength;    
    QSpinBox *trackMinLength;    
    //Buttons:
    QPushButton *createAnimation;
    QComboBox *trackColor;    
};
#endif // ANIMATIONPARAMETERWIDGET_H  
