// Slip n Slide
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

#ifndef __Slip_SlipObject__
#define __Slip_SlipObject__

#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>

#include "Pic2Header.h"
#include "Frameworks.h"
#include <mutex>
#include <hcsrc/vec3.h>
#include <hcsrc/mat4x4.h>

inline vec3 vec_from_pos(GLfloat *pos)
{
	vec3 tmpVec = make_vec3(pos[0], pos[1],
	                        pos[2]);

	return tmpVec;
}

inline void pos_from_vec(GLfloat *pos, vec3 v)
{
	pos[0] = v.x;
	pos[1] = v.y;
	pos[2] = v.z;
}

class SlipGL;
class Mesh;

class SlipObject : public QOpenGLExtraFunctions
{
public:
	SlipObject();
	virtual ~SlipObject();
	virtual void initialisePrograms(std::string *v = NULL, 
	                                std::string *f = NULL,
	                                std::string *g = NULL);
	virtual void render(SlipGL *sender);
	
	void managesTextures(bool val)
	{
		_texternal = val;
	}
	
	GLuint renderType()
	{
		return _renderType;
	}
	
	Helen3D::Vertex *vPointer()
	{
		return &_vertices[0];
	}
	
	Helen3D::Vertex vertex(size_t idx)
	{
		return _vertices[idx];
	}
	
	size_t vertexCount()
	{
		return _vertices.size();
	}
	
	void setVertex(size_t idx, Helen3D::Vertex v)
	{
		_vertices[idx] = v;
	}

	size_t vSize()
	{
		return sizeof(Helen3D::Vertex) * _vertices.size();
	}

	void clearVertices()
	{
		_vertices.clear();
		_indices.clear();
	}
	
	SlipObject(const SlipObject &other);
	
	void copyFrom(SlipObject *s);

	GLuint *iPointer()
	{
		return &_indices[0];
	}

	size_t iSize()
	{
		return sizeof(GLuint) * _indices.size();
	}
	
	GLuint texture(size_t i)
	{
		return _textures[i];
	}
	
	size_t indexCount()
	{
		return _indices.size();
	}
	
	GLuint index(int i)
	{
		return _indices[i];
	}
	
	std::string name()
	{
		return _name;
	}
	
	virtual void setName(std::string name)
	{
		_name = name;
	}
	
	bool isDisabled()
	{
		return _disabled;
	}
	
	void setModel(mat4x4 model)
	{
		_model = model;
	}
	
	void setProj(mat4x4 proj)
	{
		_proj = proj;
	}
	
	void setUnproj(mat4x4 unproj)
	{
		_unproj = unproj;
	}
	
	void lockMutex()
	{
		_mut.lock();
	}
	
	bool tryLockMutex()
	{
		return _mut.try_lock();
	}
	
	void unlockMutex()
	{
		_mut.unlock();
	}
	
	void setDisabled(bool dis);
	
	void addToVertices(vec3 add);
	
	void rotateByMatrix(mat3x3 m);
	virtual void rotate(mat3x3 &rot);
	bool collapseCommonVertices(int quick = -1, double thresh = 1e-6);
	void removeUnusedVertices();
	void recolourBoth(double red, double green, double blue);
	void recolour(double red, double green, double blue,
	              std::vector<Helen3D::Vertex> *vs = NULL);
	void setAlpha(double alpha);
	void changeProgram(std::string &v, std::string &f);
	vec3 centroid();
	vec3 randomVertex();
	void setPosition(vec3 pos);
	vec3 nearestVertex(vec3 pos, bool useMesh = false);
	vec3 nearestVertexNearNormal(vec3 pos, vec3 normal, bool *isBehind);
	Helen3D::Vertex *nearestVertexPtr(vec3 pos, bool useMesh);
	
	void changeMidPoint(double x, double y);
	void setHighlighted(bool highlighted);
	void setSelected(bool selected);
	void reorderIndices();
	void boundaries(vec3 *min, vec3 *max);
	bool intersects(double x, double y, double *z, int *vidx = NULL);
	bool intersectsPolygon(double x, double y, double *z);
	double intersects(vec3 pos, vec3 dir);
	void writeObjFile(std::string filename);
	double envelopeRadius();
	double averageRadius();
	Mesh *makeMesh(int tri = 3);
	void setCustomMesh(Mesh *m);
	
	bool hasMesh()
	{
		return (_mesh != NULL);
	}
	
	Mesh *mesh()
	{
		return _mesh;
	}
	
	void clearMesh();
	
	bool pointInside(vec3 point);
	vec3 findClosestVecToSurface(vec3 &interior);
	void colourOutlayBlack();
	void changeToLines();
	void changeToTriangles();
	virtual void triangulate();
	void cacheTriangulate();
	
	void remove()
	{
		_remove = true;
	}
	
	GLuint getProgram()
	{
		return _program;
	}
	
	bool shouldRemove()
	{
		return _remove;
	}
	
	void setFragmentShader(std::string shader)
	{
		_fString = shader;
	}

	void setShadersLike(SlipObject *o)
	{
		_vString = o->_vString;
		_fString = o->_fString;
	}

	virtual void setFocalPoint(vec3 vec);
	bool checkErrors(std::string what = "");
	void deleteVBOBuffers();

	void setColour(double red, double green, double blue)
	{
		_red = red;
		_green = green;
		_blue = blue;
		recolour(red, green, blue);
		recolour(red, green, blue, &_unselectedVertices);
	}

	void setExtra(double x, double y, double z, double w)
	{
		for (size_t i = 0; i < _vertices.size(); i++)
		{
			_vertices[i].extra[0] = x;
			_vertices[i].extra[1] = y;
			_vertices[i].extra[2] = z;
			_vertices[i].extra[3] = w;
		}
	}

	void resize(double scale, bool unselected = false);
	
	void changeVertexShader(std::string v);
	void changeFragmentShader(std::string f);
	virtual void calculateNormals(bool flip = false);
	virtual void flip();
	void setSelectable(bool selectable);
	void addToVertexArray(vec3 add, std::vector<Helen3D::Vertex> *vs);
	
	static void changeSelectionResize(double resize)
	{
		_selectionResize = resize;
	}
	
	void setNeedsExtra(bool extra)
	{
		_extra = extra;
	}

	bool polygonIncludes(vec3 point, GLuint *trio);
	vec3 closestRayTraceToPlane(vec3 point, GLuint *trio);
	void addVertex(vec3 v, std::vector<Helen3D::Vertex> *vec = NULL);
	void addIndex(GLint i);
	void addIndices(GLuint i1, GLuint i2, GLuint i3);
	void calculateNormalsAndCheck();
protected:
	void rebindToProgram();
	bool polygonIncludesY(vec3 point, GLuint *trio);
	vec3 rayTraceToPlane(vec3 point, GLuint *trio, vec3 dir,
	                     bool *backwards);
	void addVertex(float v1, float v2, float v3,
	               std::vector<Helen3D::Vertex> *vec = NULL);
	void fixCentroid(vec3 centre);
	void bindOneTexture(Picture &pic);
	void bindOneTexture(QImage *image, bool alpha = false);
	void genTextures();
	virtual void bindTextures();
	virtual void positionChanged() {};
	virtual void resized(double scale) {};
	
	virtual void extraUniforms() {};

	vec3 getFocus()
	{
		return _focus;
	}
	
	void setFocus(vec3 focus)
	{
		_focus = focus;
	}
	
	bool isSelected()
	{
		return _selected;
	}
	
	bool isSelectable()
	{
		return _selectable;
	}
	
	SlipObject *getNormals()
	{
		return _normals;
	}

	std::vector<Helen3D::Vertex> _vertices;
	std::vector<GLuint> _indices;
	std::vector<Helen3D::Vertex> _unselectedVertices;

	double _meshDot;

	double _red;
	double _green;
	double _blue;

	bool _central;
	bool _usesFocalDepth;
	bool _usesLighting;
	bool _textured;
	bool _is2D;
	bool _backToFront;
	GLuint _renderType;
	std::string _vString;
	std::string _fString;
	std::string _gString;
	GLuint _program;
	GLuint _usingProgram;
	GLint _uLight;
	GLint _uFocus;

	GLfloat _lightPos[3];
	GLfloat _xAxis[3];
	GLfloat _focalPos[3];

	mat4x4 _model;
	mat4x4 _proj;
	mat4x4 _unproj;
	std::vector<GLuint> _textures;
	bool _handleOwnTextures;
	void prepareNormalVision();
private:
	void deleteTextures();
	void rebindVBOBuffers();
	void unbindVBOBuffers();
	void setupVBOBuffers();
	int vaoForContext();
	void deletePrograms();
	GLuint addShaderFromString(GLuint program, GLenum type, std::string str);

	static bool index_behind_index(IndexTrio one, IndexTrio two);
	static bool index_in_front_of_index(IndexTrio one, IndexTrio two);

	std::string _random;
	std::map<GLuint, GLuint> _bVertices;
	std::map<GLuint, GLuint> _bElements;
	std::map<QOpenGLContext *, std::map<GLuint, GLuint>> _vaoMap;
	GLuint _uModel;
	GLuint _uProj;
	GLuint _uTime;
	std::vector<IndexTrio> _temp; // stores with model mat
	std::string _name;
	Mesh *_mesh;
	SlipObject *_normals;
	std::mutex _mut;
	
	mat4x4 _glLightMat;
	mat4x4 _glProj;
	mat4x4 _glModel;

	bool _extra;
	bool _remove;
	bool _disabled;
	bool _selected;
	bool _highlighted;
	bool _selectable;
	bool _texternal;
	vec3 _focus;
	
	SlipGL *_gl;
	static double _selectionResize;
};

#endif
