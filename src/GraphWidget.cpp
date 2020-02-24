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
#include "GraphWidget.h"

#include <QPainter>
#include <vector>
#include <iostream>

#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtWidgets/QGridLayout>
QT_CHARTS_USE_NAMESPACE

GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent)
{
  setMinimumSize(100, 100);
  chartView = new QChartView;
  chartView->setRenderHint(QPainter::Antialiasing);
  chart = chartView->chart();
  chart->legend()->setVisible(false);
  chart->setTitle("Plot");
  chart->setAnimationOptions(QChart::AllAnimations);
  QGridLayout *mainLayout = new QGridLayout;
  mainLayout->addWidget(chartView, 1, 1);
  setLayout(mainLayout);
  
}

QSize GraphWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize GraphWidget::sizeHint() const
{
    return QSize(400, 200);
}

void GraphWidget::setData(std::vector<QPointF> data,bool append /* = false*/)
{
   if(!append)
    chartView->chart()->removeAllSeries();  
   QLineSeries *series = new QLineSeries();
   for(unsigned int i=0; i<data.size(); i++)
      series->append(data[i]);
  chartView->chart()->addSeries(series);
  chartView->chart()->createDefaultAxes();
}

void GraphWidget::addData(std::vector<QPointF> data, int type /*= 0*/, bool clear /*=false*/)
{
  if(clear)
      chartView->chart()->removeAllSeries();

  if(type==0)
  {
   QLineSeries *series = new QLineSeries();
   for(unsigned int i=0; i<data.size(); i++)
      series->append(data[i]);
  chartView->chart()->addSeries(series);
  chartView->chart()->createDefaultAxes();
  }
  else if(type==1)
  {
   QScatterSeries *series = new QScatterSeries();
   for(unsigned int i=0; i<data.size(); i++)
      series->append(data[i]);
  chartView->chart()->addSeries(series);
  chartView->chart()->createDefaultAxes();
  }
}

void GraphWidget::addDataPoint(QPointF data, int type /*= 0*/, bool clear /*=false*/)
{
  chartView->chart()->removeAllSeries();
  if(clear)
  {
   std::vector<QPointF>().swap(lastData);
  }
  lastData.push_back(data);
  if(type==0)
  {
   QLineSeries *series = new QLineSeries();
   for(unsigned int i=0; i<lastData.size(); i++)
      series->append(lastData[i]);
  chartView->chart()->addSeries(series);
  chartView->chart()->createDefaultAxes();
  }
  else if(type==1)
  {
   QScatterSeries *series = new QScatterSeries();
   for(unsigned int i=0; i<lastData.size(); i++)
      series->append(lastData[i]);
  chartView->chart()->addSeries(series);
  chartView->chart()->createDefaultAxes();
  }
}

void GraphWidget::resetDataPoints()
{
   std::vector<QPointF>().swap(lastData);
}

std::vector<QPointF> GraphWidget::getDataPoints()
{
   return lastData;
}

void GraphWidget::removeAllSeries()
{
    chartView->chart()->removeAllSeries();  
}
