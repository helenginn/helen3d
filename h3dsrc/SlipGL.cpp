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

#define SHADOW_DIM 2048

#include "SlipGL.h"
#include "Quad.h"
#include "SlipObject.h"
#include "shaders/shShadow.h"
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
	_quad = NULL;
	_shadowing = false;
	_shadowProgram = 0;
	_depthMap = 0;
	_depthFbo = 0;
	_sceneMapCount = 0;
	memset(_sceneMap, '\0', sizeof(GLuint) * 8);
	memset(_pingPongMap, '\0', sizeof(GLuint) * 2);
	memset(_pingPongFbo, '\0', sizeof(GLuint) * 2);
	_sceneFbo = 0;
	_sceneDepth = 0;
	_wO = 0;
	_hO = 0;

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

void SlipGL::preparePingPongBuffers(int w_over, int h_over)
{
	glGenFramebuffers(2, _pingPongFbo);
	glGenTextures(2, _pingPongMap);

	int ratio = QApplication::desktop()->devicePixelRatio();
	int w = width() * ratio;
	int h = height() * ratio;
	
	if (w_over > 0 && h_over > 0)
	{
		w = w_over;
		h = h_over;
	}

	_wO = w;
	_hO = h;

	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _pingPongFbo[i]);
		glBindTexture(GL_TEXTURE_2D, _pingPongMap[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h,
		             0, GL_RGBA, GL_FLOAT, NULL);
		std::cout << "Dimensions: " << w << " " << h << std::endl;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		                       GL_TEXTURE_2D, _pingPongMap[i], 0);
		
		GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (result != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Incomplete frame buffer" << std::endl;
			
			switch (result)
			{
				case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				std::cout << "incomplete attachment" << std::endl;
				break;
				case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				std::cout << "incomplete missing attachment" << std::endl;
				break;
				case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				std::cout << "incomplete draw buffer" << std::endl;
				break;
				case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				std::cout << "incomplete read buffer" << std::endl;
				break;
				case GL_FRAMEBUFFER_UNSUPPORTED:
				std::cout << "unsupported format combo" << std::endl;
				break;
				case GL_FRAMEBUFFER_UNDEFINED:
				std::cout << "buffer undefined" << std::endl;
				break;
				case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				std::cout << "incomplete multisample" << std::endl;
				break;
				case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
				std::cout << "incomplete layer targets" << std::endl;
				break;
				
				default:
				std::cout << "Something else " << result << std::endl;
				break;
			}
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SlipGL::resizeTextures(int w_over, int h_over)
{
	int ratio = QApplication::desktop()->devicePixelRatio();
	int w = width() * ratio;
	int h = height() * ratio;
	
	if (w_over > 0 && h_over > 0)
	{
		w = w_over;
		h = h_over;
	}

	glBindTexture(GL_TEXTURE_2D, _sceneDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
	             w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	for (size_t i = 0; i < _sceneMapCount; i++)
	{
		glBindTexture(GL_TEXTURE_2D, _sceneMap[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 
		             w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	}
	
	if (_pingPongFbo[0] > 0)
	{
		for (size_t i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, _pingPongMap[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 
			             w, h, 0, GL_RGBA, GL_FLOAT, NULL);
		}
	}

}

void SlipGL::prepareRenderToTexture(size_t count)
{
	glGenFramebuffers(1, &_sceneFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _sceneFbo);

	glGenTextures(count, _sceneMap);
	glGenTextures(1, &_sceneDepth);

	_sceneMapCount = count;

	int ratio = QApplication::desktop()->devicePixelRatio();
	int w = width() * ratio;
	int h = height() * ratio;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _sceneDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
	             w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
	                       GL_TEXTURE_2D, _sceneDepth, 0);

	unsigned int attachments[8];
	
	for (size_t i = 0; i < count; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i + 1);
		glBindTexture(GL_TEXTURE_2D, _sceneMap[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 
		             w, h, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		checkErrors("Making framebuffer");
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 
		                       GL_TEXTURE_2D, _sceneMap[i], 0);
		checkErrors("Binding texture");
		
		attachments[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	
	glDrawBuffers(count, attachments);  

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	_quad = new Quad();
}

void SlipGL::prepareShadowBuffer()
{
	glGenFramebuffers(1, &_depthFbo);
	glGenTextures(1, &_depthMap);
	glBindTexture(GL_TEXTURE_2D, _depthMap);

	int w = SHADOW_DIM; int h = SHADOW_DIM;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
	             w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  

	glBindFramebuffer(GL_FRAMEBUFFER, _depthFbo);
	checkErrors("Making framebuffer");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
	                       GL_TEXTURE_2D, _depthMap, 0);
	checkErrors("Binding texture");
	GLenum none = GL_NONE;
	glDrawBuffers(1, &none);
	glReadBuffer(GL_NONE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	shadowProgram();
}

void SlipGL::renderShadows()
{
	_shadowing = true;
	int w = SHADOW_DIM; int h = SHADOW_DIM;
	glViewport(0, 0, w, h);
	glBindFramebuffer(GL_FRAMEBUFFER, _depthFbo);
    glClear(GL_DEPTH_BUFFER_BIT);
	mat4x4 model = _model;
	mat4x4 proj = _proj;
    changeProjectionForLight(0);
    renderScene();
	_model = model;
	_proj = proj;

	int ratio = QApplication::desktop()->devicePixelRatio();
	glViewport(0, 0, width() * ratio, height() * ratio);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
	_shadowing = false;
}

void SlipGL::renderScene()
{
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

void SlipGL::paintGL()
{
	checkErrors("before paintGL");
	updateCamera();
	if (_paused)
	{
		return;
	}
	
	if (_depthMap > 0)
	{
		renderShadows();
	}
	
	if (_sceneFbo > 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _sceneFbo);
	}
	
	glClearColor(_r, _g, _b, _a);
	checkErrors("clear color");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	checkErrors("clear buffer bits");

	renderScene();
	
	if (_sceneFbo > 0)
	{
//		glViewport(0, 0, _wO, _hO);

		if (_pingPongFbo[0] > 0)
		{
			specialQuadRender();
		}
		else
		{
			quad()->setTexture(1, _sceneMap[0]);
		}

		int ratio = QApplication::desktop()->devicePixelRatio();
//		glViewport(0, 0, width() * ratio, height() * ratio);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0); 
		renderQuad();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	updateProjection();
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

void SlipGL::shadowProgram()
{
	if (_shadowProgram != 0)
	{
		return;
	}
	
	std::string v = vShadow();
	std::string f = fShadow();
	
	_shadowObj = new SlipObject();
	_shadowObj->initialisePrograms(&v, &f);

	_shadowProgram = _shadowObj->getProgram();
}

GLuint SlipGL::getOverrideProgram()
{
	if (_shadowing)
	{
		return _shadowProgram;
	}
	
	return 0;
}

void SlipGL::renderQuad()
{
	glClearColor(_r, _g, _b, _a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_quad->render(this);
}

void SlipGL::specialQuadRender()
{
	int amount = 2;
	int mode = 0;
	float threshold = 0.3;
	_quad->setTexture(0, _sceneMap[0]);

	for (unsigned int i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _pingPongFbo[mode]); 
		checkErrors("ping pong frame bind");
		_quad->setThreshold(threshold);
		_quad->setMode(mode);
		_quad->setTexture(1, (i == 0) ?  _sceneMap[1] : _pingPongMap[!mode]); 
		renderQuad();
		checkErrors("rendered quad");
		mode = !mode;
	}

	_quad->setMode(2);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
	_quad->setTexture(0, _sceneMap[0]); 
	_quad->setTexture(1, _pingPongMap[mode]); 
}
