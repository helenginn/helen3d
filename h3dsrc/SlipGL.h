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
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QMouseEvent>

#include <hcsrc/mat4x4.h>

class SlipObject;
class Quad;
class QTimer;

class SlipGL : public QOpenGLWidget, public QOpenGLExtraFunctions
{
	Q_OBJECT
	
public:
	SlipGL(QWidget *parent, bool extraRendering = false);
	
	void preparePanels(int n);
	void addPanel();
	
	void focusOnPosition(vec3 pos, double dist = 13);   
	void panned(double x, double y);
	void draggedLeftMouse(double x, double y);
	void draggedRightMouse(double x, double y);
	void changeCentre(vec3 c);
	void rotate(double x, double y, double z);
	void setInvertZ(bool z)
	{
		_invertZ = z;
	}
	
	void setAcceptsFocus(bool accepts)
	{
		_acceptsFocus = accepts;
	}
	
	void setZNear(double near)
	{
		zNear = near;
		updateProjection();
	}
	
	void setZFar(double far)
	{
		zFar = far;
		updateProjection();
	}
	
	void setFrustum(bool f)
	{
		_frustum = f;
		updateProjection();
	}
	
	SlipObject *activeObj()
	{
		return _activeObj;
	}
	
	float getTime()
	{
		return _time;
	}
	
	void setModel(mat4x4 m)
	{
		_model = m;
	}
	
	mat4x4 getModel()
	{
		return _model;
	}
	
	vec3 getCentre()
	{
		return _centre;
	}
	
	void setProjection(mat4x4 proj)
	{
		_proj = proj;
	}
	
	mat4x4 getProjection()
	{
		return _proj;
	}

	void saveImage(std::string filename);

	vec3 transformPosByModel(vec3 pos)
	{
		vec3 newPos = mat4x4_mult_vec(_model, pos);
		return newPos;
	}
	
	mat4x4 lightMat()
	{
		return _lightMat;
	}
	
	GLuint depthMap()
	{
		return _depthMap;
	}
	
	std::vector<SlipObject *> takeObjects()
	{
		std::vector<SlipObject *> tmp = _objects;
		_objects.clear();
		return tmp;
	}
	
	void setObjects(std::vector<SlipObject *> tmp)
	{
		_objects = tmp;
	}
	
	void clearObjects()
	{
		_objects.clear();
	}
	
	size_t objectCount()
	{
		return _objects.size();
	}
	
	SlipObject *object(int i)
	{
		return _objects[i];
	}
	
	GLuint getOverrideProgram();
	
	Quad *quad()
	{
		return _quad;
	}
	
	size_t sceneTextureCount()
	{
		return _sceneMapCount;
	}
	
	GLuint sceneTexture(int i)
	{
		return _sceneMap[i];
	}
	
	GLuint sceneDepth()
	{
		return _sceneDepth;
	}
	
	void pause();
	void restartTimer();
	
	void addObject(SlipObject *obj, bool active = false);
	void addObjectAtIndex(SlipObject *obj, int index);
	virtual void updateProjection(double side = 0.5);
	void removeObject(SlipObject *obj);
	
	void setBackground(double r, double g, double b, double a);

	bool checkErrors(std::string what = "");
public slots:
	
protected:
	void shadowProgram();
	void prepareShadowBuffer();
	void preparePingPongBuffers(int w_over = -1, int h_over = -1);
	void prepareRenderToTexture(size_t count);
	void resizeTextures(int w_over = -1, int h_over = -1);
	void renderShadows();
	void renderScene();
	void renderQuad();
	virtual void specialQuadRender();

	virtual void showEvent(QShowEvent *e);

	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent(QKeyEvent *event);

	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int w, int h);
	virtual void setFocalPoint(vec3 pos);
	void convertCoords(double *x, double *y);

	virtual void changeProjectionForLight(int i) {};

	void initialisePrograms();
	void zoom(float x, float y, float z);
	void updateCamera();
	void setupCamera();
	void time();
	bool _paused;

	SlipObject *activeObject()
	{
		return _activeObj;
	}
	
	GLuint _depthMap;
	GLuint _depthFbo;

	GLuint _sceneMap[8];
	GLuint _sceneDepth;
	GLuint _sceneFbo;
	size_t _sceneMapCount;
	
	GLuint _pingPongMap[2];
	GLuint _pingPongFbo[2];
	
	float _camAlpha, _camBeta, _camGamma;
	float zNear, zFar;
	float _time;
	bool _invertZ;

	QTimer *_timer;
	vec3 _centre;
	vec3 _translation;
	vec3 _transOnly;
	vec3 _totalCentroid;

	mat4x4 _lightMat;
	mat4x4 _model;
	mat4x4 _proj;
	mat4x4 _unproj;
	
	GLuint _shadowProgram;

	std::vector<SlipObject *> _objects;
	
	SlipObject *_activeObj;
	SlipObject *_shadowObj;
	Quad *_quad;

	double _a; double _r; double _g; double _b;

	double _lastX; double _lastY;
	double _shadowing;
	bool _moving;
	Qt::MouseButton _mouseButton;
	bool _controlPressed;
	bool _shiftPressed;
	bool _acceptsFocus;
	bool _extraRendering;
	bool _frustum;
	
	int _wO, _hO;
	
	QWidget *_p;

	static bool _setup;
};


#endif
