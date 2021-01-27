// SlipGL
// Copyright (C) 2017-2018 Helen Ginn
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

#include "SlipGL.h"
#include "SlipObject.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWindow>
#include <QTimer>
#include <iostream>
#include <iomanip>
#include <QImageWriter>

#define MOUSE_SENSITIVITY 500

bool SlipGL::_setup = false;

void SlipGL::initializeGL()
{
	initializeOpenGLFunctions();
	
	checkErrors("before initializeGL");

	glClearColor(_r, _g, _b, _a);
	checkErrors("clear color");
	glEnable(GL_DEPTH_TEST);
	checkErrors("depth test");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	checkErrors("clear buffer bits");

	glEnable(GL_BLEND);
	checkErrors("blend");
	glEnable(GL_POINT_SPRITE);
	checkErrors("point sprite");
	glEnable(GL_PROGRAM_POINT_SIZE);
	checkErrors("point size");

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	checkErrors("blend func");

	initialisePrograms();
	_setup = true;
}

void SlipGL::setBackground(double r, double g, double b, double a)
{
	_r = r; _g = g; _b = b; _a = a;
}

SlipGL::SlipGL(QWidget *p) : QOpenGLWidget(p)
{
	_dbCount = 0;
	_acceptsFocus = true;
	_shiftPressed = false;
	_controlPressed = false;
	_moving = false;
	_lastX = -1;
	_lastY = -1;
	_mouseButton = Qt::NoButton;
	zNear = 4;
	zFar = 400;

	setBackground(0, 0, 0, 1);
	_invertZ = false;
	_paused = false;
	_time = 0;
	_timer = new QTimer();
	_timer->setInterval(50);
	_timer->setSingleShot(false);
	connect(_timer, &QTimer::timeout, this, &SlipGL::time);
	_timer->start();

	if (p)
	{
		setGeometry(p->geometry());
	}

	setupCamera();

}

void SlipGL::pause()
{
	_timer->setSingleShot(true);
	_paused = true;
}

void SlipGL::restartTimer()
{
	_timer->setInterval(50);
	_timer->setSingleShot(false);
	_timer->start();
	_paused = false;
}

void SlipGL::addObject(SlipObject *obj, bool active)
{
	_objects.push_back(obj);
	obj->setModel(_model);
	obj->setProj(_proj);
	obj->setUnproj(_unproj);
	obj->reorderIndices();
	
	if (active)
	{
		vec3 c = obj->centroid();
		vec3_add_to_vec3(&_centre, c);
		vec3_mult(&c, -1);
		_translation = c;
	}
}

void SlipGL::changeCentre(vec3 c)
{
	_centre = c;
	updateCamera();
}

void SlipGL::removeObject(SlipObject *obj)
{
	std::vector<SlipObject *>::iterator it;
	
	it = std::find(_objects.begin(), _objects.end(), obj);
	
	if (it != _objects.end())
	{
		_objects.erase(it);
	}
}

void SlipGL::addPanel()
{
	/*
	*/
}

void SlipGL::preparePanels(int n)
{
	_objects.reserve(n);
}

void SlipGL::panned(double x, double y)
{
	zoom(-x, y, 0);
}

void SlipGL::zoom(float x, float y, float z)
{
	_translation.x += x;
	_translation.y += y;
	_translation.z += z;
	
	_transOnly.x += x;
	_transOnly.y += y;
	_transOnly.z += z;

}

void SlipGL::time()
{
	_time += 0.002;
	
	if (_time > 1) _time -= 1;
	
	update();
}

void SlipGL::rotate(double x, double y, double z)
{
	_camAlpha += x;
	_camBeta += y;
	_camGamma += z;
}

void SlipGL::draggedLeftMouse(double x, double y)
{
	x /= MOUSE_SENSITIVITY;
	y /= MOUSE_SENSITIVITY;

	double mult = _invertZ ? -1 : 1;

	_camAlpha -= y * mult;
	_camBeta -= x;
	_camGamma -= 0;

	updateCamera();
	
	for (unsigned int i = 0; i < _objects.size(); i++)
	{
		_objects[i]->setModel(_model);
		_objects[i]->reorderIndices();
	}

	update();
}

void SlipGL::draggedRightMouse(double x, double y)
{
	zoom(0, 0, -y / MOUSE_SENSITIVITY * 10);
	update();
}

void SlipGL::showEvent(QShowEvent *e)
{
	QOpenGLWidget::showEvent(e);
}

void SlipGL::initialisePrograms()
{
	for (unsigned int i = 0; i < _objects.size(); i++)
	{
		_objects[i]->initialisePrograms();
	}
}

void SlipGL::resizeDepthBuffers(int w, int h)
{
	for (size_t i = 0; i < _dbCount; i++)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, _depthRbo[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
}

void SlipGL::prepareDepthBuffer(size_t count)
{
	_dbCount = count;

	glGenFramebuffers(count, _depthFbo);
	glGenRenderbuffers(count, _depthRbo);

	for (size_t i = 0; i < count; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _depthFbo[i]);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		
		int w = width(); int h = height();
		int ratio = QApplication::desktop()->devicePixelRatio();
		w *= ratio; h *= ratio;

		glBindRenderbuffer(GL_RENDERBUFFER, _depthRbo[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		                          GL_RENDERBUFFER, _depthRbo[i]);

		glClear(GL_DEPTH_BUFFER_BIT);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Problem with frame buffer!" << std::endl;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void SlipGL::paintGL()
{
	checkErrors("before paintGL");
	updateCamera();
	if (_paused)
	{
		return;
	}

	glClearColor(_r, _g, _b, _a);
	checkErrors("clear color");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	checkErrors("clear buffer bits");

	for (unsigned int i = 0; i < _objects.size(); i++)
	{
		if (_objects[i]->shouldRemove())
		{
			delete _objects[i];
			_objects.erase(_objects.begin() + i);
			continue;
		}

		_objects[i]->setModel(_model);
		_objects[i]->setProj(_proj);
		_objects[i]->setUnproj(_unproj);
		_objects[i]->render(this);
	}
}

void SlipGL::updateCamera()
{
	vec3 centre = _centre;
	vec3 negCentre = centre;
	vec3_mult(&negCentre, -1);

	mat4x4 change = make_mat4x4();
	mat4x4_translate(&change, negCentre);
	mat4x4_rotate(&change, _camAlpha, _camBeta, _camGamma);
	mat4x4_translate(&change, centre);

	mat4x4 transMat = make_mat4x4();
	_centre = vec3_add_vec3(_centre, _translation);
	mat4x4_translate(&transMat, _translation);

	mat4x4 tmp = mat4x4_mult_mat4x4(change, transMat);
	_model = mat4x4_mult_mat4x4(tmp, _model);

	_camAlpha = 0; _camBeta = 0; _camGamma = 0;
	_translation = make_vec3(0, 0, 0);
	
	_centre.x = 0;
	_centre.y = 0;
	setFocalPoint(negCentre);
}


void SlipGL::setupCamera()
{
	_translation = make_vec3(0, 0, START_Z);
	_transOnly = make_vec3(0, 0, 0);
	_totalCentroid = make_vec3(0, 0, 0);
	_centre = make_vec3(0, 0, 0);
	_camAlpha = 0;
	_camBeta = 0;
	_camGamma = 0;
	_model = make_mat4x4();

	updateProjection();
	updateCamera();
}


void SlipGL::updateProjection(double side)
{
	float aspect = (float)height() / (float)width();
	
	_proj = mat4x4_frustum(-side, side, side * aspect, -side * aspect,
	                       zNear, zFar);
	_unproj = mat4x4_unfrustum(-side, side, side * aspect, -side * aspect,
	                       zNear, zFar);
}

void SlipGL::resizeGL(int w, int h)
{
	int ratio = QApplication::desktop()->devicePixelRatio();
	updateProjection();
	resizeDepthBuffers(w * ratio, h * ratio);
}

void SlipGL::setFocalPoint(vec3 pos)
{
	for (size_t i = 0; i < _objects.size(); i++)
	{
		_objects[i]->setFocalPoint(pos);
	}
}

void SlipGL::focusOnPosition(vec3 pos, double dist)
{
	if (pos.x != pos.x)
	{
		return;
	}

	vec3 newPos = transformPosByModel(pos);
	_centre = newPos;
	vec3_mult(&newPos, -1);
	newPos.z -= dist;

	_translation = vec3_add_vec3(_translation, newPos);
}

void SlipGL::mousePressEvent(QMouseEvent *e)
{
	if (!_acceptsFocus)
	{
		e->ignore();
		return;
	}

	_lastX = e->x();
	_lastY = e->y();
	_mouseButton = e->button();
	_moving = false;
}

void SlipGL::mouseMoveEvent(QMouseEvent *e)
{
	if (!_acceptsFocus)
	{
		e->ignore();
		return;
	}

	double x = e->x(); double y = e->y();
	convertCoords(&x, &y);
	_moving = true;
	
	double newX = e->x();
	double xDiff = _lastX - newX;
	double newY = e->y();
	double yDiff = _lastY - newY;
	_lastX = newX;
	_lastY = newY;

	if (_mouseButton == Qt::LeftButton)
	{
		if (_controlPressed)
		{
			panned(xDiff / 30, yDiff / 30);
		}
		else
		{
			draggedLeftMouse(xDiff * 4, yDiff * 4);
		}
	}
	else if (_mouseButton == Qt::RightButton)
	{
		draggedRightMouse(xDiff * 30, yDiff * 30);
	}
}

void SlipGL::mouseReleaseEvent(QMouseEvent *e)
{
	if (!_acceptsFocus)
	{
		e->ignore();
		return;
	}

	_mouseButton = Qt::NoButton;
}


void SlipGL::keyPressEvent(QKeyEvent *event)
{
	if (!_acceptsFocus)
	{
		event->ignore();
		return;
	}

	if (event->key() == Qt::Key_Alt || event->key() == Qt::Key_Control)
	{
		_controlPressed = true;
	}
	else if (event->key() == Qt::Key_Shift)
	{
		_shiftPressed = true;
	}
}

void SlipGL::keyReleaseEvent(QKeyEvent *event)
{
	if (!_acceptsFocus)
	{
		event->ignore();
		return;
	}
	
	if (event->key() == Qt::Key_Alt || event->key() == Qt::Key_Control)
	{
		_controlPressed = false;
	}
	else if (event->key() == Qt::Key_Shift)
	{
		_shiftPressed = false;
	}
}

void SlipGL::convertCoords(double *x, double *y)
{
	double w = width();
	double h = height();

	*x = 2 * *x / w - 1.0;
	*y =  - (2 * *y / h - 1.0);
}

void SlipGL::copyDefaultToOffscreenDepthBuffer(int which)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFramebufferObject());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _depthFbo[which]);

	int ratio = QApplication::desktop()->devicePixelRatio();

	glBlitFramebuffer(0, 0, ratio * width(), ratio * height(),
	                  0, 0, ratio * width(), ratio * height(),
	                  GL_DEPTH_BUFFER_BIT,
	                  GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SlipGL::copyOffscreenDepthBufferToDefault(int which)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _depthFbo[which]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());

	int ratio = QApplication::desktop()->devicePixelRatio();

	glBlitFramebuffer(0, 0, ratio * width(), ratio * height(),
	                  0, 0, ratio * width(), ratio * height(),
	                  GL_DEPTH_BUFFER_BIT,
	                  GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool SlipGL::checkErrors(std::string what)
{
	GLenum err = glGetError();

	if (err != 0)
	{
		std::cout << "Error as SlipGL was doing " << what << ":" 
		<< err << std::endl;
		
		switch (err)
		{
			case GL_INVALID_ENUM:
			std::cout << "Invalid enumeration" << std::endl;
			break;

			case GL_STACK_OVERFLOW:
			std::cout << "Stack overflow" << std::endl;
			break;

			case GL_STACK_UNDERFLOW:
			std::cout << "Stack underflow" << std::endl;
			break;

			case GL_OUT_OF_MEMORY:
			std::cout << "Out of memory" << std::endl;
			break;

			case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cout << "Invalid framebuffer op" << std::endl;
			break;

			case GL_INVALID_VALUE:
			std::cout << "Invalid value" << std::endl;
			break;

			case GL_INVALID_OPERATION:
			std::cout << "Invalid operation" << std::endl;
			break;

		}
	}
	
	return (err != 0);
}

void SlipGL::saveImage(std::string filename)
{
	QImage image = grabFramebuffer();
	QImageWriter writer(QString::fromStdString(filename));
	writer.write(image);
}
