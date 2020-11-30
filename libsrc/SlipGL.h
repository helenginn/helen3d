// Slip n Slide
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

#ifndef __Slip__SlipGL__
#define __Slip__SlipGL__

#include <QtWidgets/qopenglwidget.h>
#include <QtGui/qopengl.h>
#include <QtGui/qopenglfunctions.h>

#include "mat4x4.h"

class SlipObject;
class QTimer;

class SlipGL : public QOpenGLWidget, QOpenGLFunctions
{
	Q_OBJECT
	
public:
	SlipGL(QWidget *parent);
	
	void preparePanels(int n);
	void addPanel();
	
	void panned(double x, double y);
	void draggedLeftMouse(double x, double y);
	void draggedRightMouse(double x, double y);
	void changeCentre(vec3 c);
	void rotate(double x, double y, double z);
	void setInvertZ(bool z)
	{
		_invertZ = z;
	}
	
	SlipObject *activeObj()
	{
		return _activeObj;
	}
	
	float getTime()
	{
		return _time;
	}
	
	mat4x4 getModel()
	{
		return _model;
	}
	
	vec3 getCentre()
	{
		return _centre;
	}
	
	mat4x4 getProjection()
	{
		return _proj;
	}
	
	void clearObjects()
	{
		_objects.clear();
	}
	
	void pause();
	void restartTimer();
	
	void addObject(SlipObject *obj, bool active);
	void updateProjection(double side = 0.5);
	void removeObject(SlipObject *obj);
	
	void setBackground(double r, double g, double b, double a);
public slots:
	
protected:
	virtual void initializeGL();
	virtual void paintGL();

	void initialisePrograms();
	void zoom(float x, float y, float z);
	void updateCamera();
	void setupCamera();
	void time();

	SlipObject *activeObject()
	{
		return _activeObj;
	}
	
	float _camAlpha, _camBeta, _camGamma;
	float zNear, zFar;
	float _time;
	bool _invertZ;

	QTimer *_timer;
	vec3 _centre;
	vec3 _translation;
	vec3 _transOnly;
	vec3 _totalCentroid;

	mat4x4 _model;
	mat4x4 _proj;
	std::vector<SlipObject *> _objects;
	
	SlipObject *_activeObj;

	double _a; double _r; double _g; double _b;

	struct detector *_d;
};


#endif
