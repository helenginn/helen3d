// abmap
// Copyright (C) 2019 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#include "Curve.h"
#include "CurveView.h"
#include "FileReader.h"
#include <iostream>
#include <QThread>
#include <QGraphicsView>
#include <QGraphicsLineItem>

Curve::Curve(QTreeWidget *w) : QTreeWidgetItem(w)
{
	_worker = NULL;
	_pointData = false;
	
	for (int i = 0; i < 3; i++)
	{
		_minRegion[i] = FLT_MAX;
		_maxRegion[i] = FLT_MAX;
	}
}

void Curve::drawCurve(CurveView *view)
{
	_lastView = view;
	QGraphicsScene *_scene = view->scene();
	QPen pen = getPen();
	
	std::vector<double> tmpx, tmpy;
	
	_mut.lock();

	tmpx = _xs;
	tmpy = _ys;

	_mut.unlock();
	
	if (tmpx.size() < 2 || tmpy.size() < 2)
	{
		return;
	}

	double x = tmpx[0];
	double y = tmpy[0];
	view->convertCoords(&x, &y);

	for (size_t i = 0; i < tmpx.size(); i++)
	{
		double newx = tmpx[i];
		double newy = tmpy[i];
		view->convertCoords(&newx, &newy);
		
		if (!_pointData)
		{
			QGraphicsLineItem *line = _scene->addLine(x, y, newx, newy, pen);
			line->setPen(pen);
		}
		else
		{
			QGraphicsEllipseItem *ellipse;
			ellipse = _scene->addEllipse(newx - 1, newy - 1, 
			                             2, 2, pen);
			ellipse->setPen(pen);

		}
		x = newx;
		y = newy;
	}
}

QPen Curve::getPen()
{
	return QPen(QColor(0, 0, 0));

}

void Curve::getRegion(int r, double *min, double *max)
{
	*min = _minRegion[r];
	*max = _maxRegion[r];

}

void Curve::setRegion(int r, double min, double max)
{
	_minRegion[r] = min;
	_maxRegion[r] = max;
}


void Curve::toOriginal()
{
	if (_worker && _worker->isRunning())
	{
		std::cout << "Proper error message" << std::endl;
		return;
	}

	_ys = _orgys;
}

void Curve::subtract(double shift)
{
	for (size_t i = 0; i < _ys.size(); i++)
	{
		_ys[i] += shift;
	}
}
