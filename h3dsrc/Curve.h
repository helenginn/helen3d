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

#ifndef __abmap__Curve__
#define __abmap__Curve__

#include <vector>
#include <mutex>
#include <condition_variable>
#include <string>
#include <float.h>
#include <QPen>
#include <iostream>
#include <QTreeWidgetItem>
#include <QObject>

class CurveView;
class QThread;

class Curve : public QObject, public QTreeWidgetItem
{
Q_OBJECT
public:
	Curve(QTreeWidget *w);

	void setFilename(std::string fn)
	{
		_filename = fn;
	}
	
	void setNickname(std::string n)
	{
		_nickname = n;
	}
	
	std::string nickname()
	{
		return _nickname;
	}

	void drawCurve(CurveView *view);
	
	bool isRegionSet(int r)
	{
		return !(_minRegion[r] == FLT_MAX && _maxRegion[r] == FLT_MAX);
	}
	
	void setRegion(int r, double min, double max);
	void getRegion(int r, double *min, double *max);
	
	void toOriginal();
	
	std::vector<double> xs()
	{
		return _xs;
	}
	
	std::vector<double> ys()
	{
		return _ys;
	}
	
	void copyYs(Curve *c);
	
	double *regionPtr(int r, bool min)
	{
		if (min)
		{
			return &_minRegion[r];
		}
		
		return &_maxRegion[r];
	}
	
	void subtract(double shift);
	
	void setCurveView(CurveView *v)
	{
		_lastView = v;
	}
	
	CurveView *getCurveView()
	{
		return _lastView;
	}
	
	void setPointData(bool point)
	{
		_pointData = point;
	}
	
	void addDataPoint(double x, double y)
	{
		_xs.push_back(x);
		_ys.push_back(y);
	}
	
	void clear()
	{
		_xs.clear();
		_ys.clear();
	}
public slots:
	
signals:
	
protected:
	virtual QPen getPen();

	std::vector<double> _xs;
	std::vector<double> _ys;
	std::vector<double> _orgys;

	std::mutex _mut;
	std::condition_variable _cv;
private:
	QThread *_worker;
	CurveView *_lastView;
	std::string _filename;
	std::string _nickname;

	bool _pointData;
	double _minRegion[3];
	double _maxRegion[3];
};

#endif
