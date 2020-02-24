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
#ifndef CODECPARAMETERWIDGET_H
#define CODECPARAMETERWIDGET_H

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

class CodecParameterWidget : public QWidget
{

    Q_OBJECT

public:
    CodecParameterWidget();
    void setHardware(bool b);
    void setBitrate(int i);
    void setGopSize(int i);
    void setFrameNum(int i);
    void setFrameDen(int i);
    void setCodec(int i);
    void setPreset(int i);
    void setUseCrf(bool b);
    void setCrf(int i);

public slots:
    bool getHardware();
    int getBitrate();
    int getGopSize();
    int getFrameNum();
    int getFrameDen();
    int getCodec();
    int getPreset();
    bool getUseCrf();
    int getCrf();
        
private:
    QComboBox *codec;
    QComboBox *preset;
    QCheckBox *hardware;
    QSpinBox *bitrate;
    QSpinBox *gopSize;
    QSpinBox *fNum;
    QSpinBox *fDen;
    QCheckBox *useCrf;    
    QSpinBox *crf;
};
#endif // CODECPARAMETERWIDGET_H  
