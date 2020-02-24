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
#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QPen>
#include <QWidget>
#include <vector>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>

QT_CHARTS_USE_NAMESPACE

class GraphWidget : public QWidget
{
    Q_OBJECT

public:

    GraphWidget(QWidget *parent = 0);
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

public slots:
    void setData(std::vector<QPointF> data, bool append = false);
    void addData(std::vector<QPointF> data, int type = 0, bool clear = false);
    void addDataPoint(QPointF data, int type = 0, bool clear = false);
    void resetDataPoints();
    std::vector<QPointF> getDataPoints();
    void removeAllSeries();

private:
    QChartView *chartView;
    QChart *chart;
    std::vector<QPointF> lastData;
};

#endif // GRAPHWIDGET_H
