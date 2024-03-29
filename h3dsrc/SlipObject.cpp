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

#include "Frameworks.h"
using namespace Helen3D;

#include "SlipObject.h"
#include "SlipGL.h"
#include "Mesh.h"
#include "charmanip.h"
#include "mat3x3.h"
#include <float.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <string>
#include <QImage>
#include "shaders/vImage.h"
#include "shaders/fImage.h"
#include "shaders/fWipe.h"

double SlipObject::_selectionResize = 1.3;

SlipObject::SlipObject(const SlipObject &other)
{
	_vertices = other._vertices;
	_indices = other._indices;
	_unselectedVertices = other._unselectedVertices;
	_meshDot = other._meshDot;
	_red = other._red;
	_green = other._green;
	_blue = other._blue;
	_central = other._central;
	_usesFocalDepth = other._usesFocalDepth;
	_usesLighting = other._usesLighting;
	_textured = other._textured;
	_is2D = other._is2D;
	_backToFront = other._backToFront;
	_renderType = other._renderType;
	_vString = other._vString;
	_fString = other._fString;
	_program = 0;
	_usingProgram = 0;
	_model = other._model;
	_proj = other._proj;
	_unproj = other._unproj;
	_textures = other._textures;
	_handleOwnTextures = other._handleOwnTextures;
	_random = other._random;
	_uModel = 0;
	_uProj = 0;
	_uTime = 0;
	_name = other._name;
	_mesh = NULL;
	_normals = NULL;

	_extra = other._extra;
	_remove = other._remove;
	_disabled = other._disabled;
	_selected = other._selected;
	_highlighted = other._highlighted;
	_selectable = other._selectable;
	_texternal = other._texternal;
	_focus = other._focus;
	_gl = other._gl;
}

void SlipObject::addToVertexArray(vec3 add, std::vector<Vertex> *vs)
{
	for (size_t i = 0; i < vs->size(); i++)
	{
		(*vs)[i].pos[0] += add.x;
		(*vs)[i].pos[1] += add.y;
		(*vs)[i].pos[2] += add.z;
	}
}

void SlipObject::addToVertices(vec3 add)
{
	if (add.x != add.x || add.y != add.y || add.z != add.z)
	{
		return;
	}

	addToVertexArray(add, &_vertices);
	addToVertexArray(add, &_unselectedVertices);
	positionChanged();
}

SlipObject::SlipObject()
{
	_handleOwnTextures = false;
	_normals = NULL;
	_gl = NULL;
	_is2D = true;
	_textured = false;
	_texternal = false;
	_name = "generic object";
	_meshDot = 0.9;
	_vString = Pencil_vsh();
	_fString = Pencil_fsh();
	_remove = false;
	_mesh = NULL;
	_renderType = GL_TRIANGLES;
	_program = 0;
	_usingProgram = 0;
	_backToFront = false;
	_uModel = 0;
	_extra = false;
	_focus = empty_vec3();
	_usesLighting = false;
	_usesFocalDepth = false;
	_central = 0;
	_disabled = 0;
	_highlighted = 0;
	_selected = 0;
	_selectable = 0;
}

SlipObject::~SlipObject()
{
	if (_mesh != NULL)
	{
		delete _mesh;
		_mesh = NULL;
	}
	
	deletePrograms();
	deleteVBOBuffers();
	deleteTextures();
	
	_vString = "";
	_fString = "";
	_gString = "";
	_unselectedVertices.clear();
	_vertices.clear();
	_indices.clear();
	std::vector<Vertex>().swap(_unselectedVertices);
	std::vector<Vertex>().swap(_vertices);
	std::vector<GLuint>().swap(_indices);
}

GLuint SlipObject::addShaderFromString(GLuint program, GLenum type, 
                                       std::string str)
{
	GLint length = str.length();

	const char *cstr = str.c_str();
	GLuint shader = glCreateShader(type);
	bool error = checkErrors("create shader");
	
	if (error)
	{
		switch (type)
		{
			case GL_GEOMETRY_SHADER:
			std::cout <<  "geometry" << std::endl;
			break;
			
			case GL_VERTEX_SHADER:
			std::cout <<  "vertex" << std::endl;
			break;
			
			case GL_FRAGMENT_SHADER:
			std::cout <<  "fragment" << std::endl;

			default:
			std::cout << "Other" << std::endl;
			break;
		}
	}

	glShaderSource(shader, 1, &cstr, &length);
	checkErrors("sourcing shader");
	
	glCompileShader(shader);
	checkErrors("compiling shader");

	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		char *log;

		/* get the shader info log */
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		log = (char *)malloc(length);
		glGetShaderInfoLog(shader, length, &result, log);

		/* print an error message and the info log */
		std::cout << "Shader: unable to compile: " << std::endl;
		std::cout << str << std::endl;
		std::cout << log << std::endl;
		free(log);

		glDeleteShader(shader);
		return 0;
	}

	glAttachShader(_program, shader);
	return shader;
}

void SlipObject::deletePrograms()
{
	if (_program != 0)
	{
		glDeleteProgram(_program);
	}
	_program = 0;
}

void SlipObject::rebindToProgram()
{

}

void SlipObject::initialisePrograms(std::string *v, std::string *f,
                                    std::string *g)
{
	initializeOpenGLFunctions();

	if (_program != 0)
	{
		return;
	}

	if (v == NULL)
	{
		v = &_vString;
	}

	if (f == NULL)
	{
		f = &_fString;
	}

	if (g == NULL)
	{
		g = &_gString;
	}

	GLint result;

	/* create program object and attach shaders */
	_program = glCreateProgram();
	checkErrors("create new program");

	addShaderFromString(_program, GL_VERTEX_SHADER, *v);
	checkErrors("adding vshader");
	
	if (g->length() > 0)
	{
		addShaderFromString(_program, GL_GEOMETRY_SHADER, *g);
		checkErrors("adding gshader");
	}

	addShaderFromString(_program, GL_FRAGMENT_SHADER, *f);
	checkErrors("adding fshader");

	glBindAttribLocation(_program, 0, "position");
	glBindAttribLocation(_program, 1, "normal");
	glBindAttribLocation(_program, 2, "color");

	if (!_extra)
	{
		glBindAttribLocation(_program, 3, "projection");
	}
	else
	{
		glBindAttribLocation(_program, 3, "extra");
	}

	if (_textured || _textures.size())
	{
		glBindAttribLocation(_program, 4, "tex");
	}

	checkErrors("binding attributions");

	/* link the program and make sure that there were no errors */
	glLinkProgram(_program);
	glGetProgramiv(_program, GL_LINK_STATUS, &result);
	checkErrors("linking program");

	if (result == GL_FALSE)
	{
		std::cout << "sceneInit(): Program linking failed." << std::endl;

		GLint length = 1000;
		char *log = (char *)malloc(length);
		/* get the shader info log */
		glGetProgramInfoLog(_program, GL_INFO_LOG_LENGTH, &length, log);

		/* print an error message and the info log */
		std::cout << log << std::endl;
		/* delete the program */
		glDeleteProgram(_program);
		_program = 0;
	}
}

void SlipObject::genTextures()
{
	if (_textures.size() == 0)
	{
		_textures.resize(1);
		glGenTextures(1, &_textures[0]);
	}
}

void SlipObject::deleteTextures()
{
	if (_texternal || _textures.size() == 0)
	{
		return;
	}

	glDeleteTextures(_textures.size(), &_textures[0]);
	_textures.clear();
}

void SlipObject::bindTextures()
{

}

void SlipObject::bindOneTexture(QImage *image, bool alpha)
{
	genTextures();

	glBindTexture(GL_TEXTURE_2D, _textures[0]);
	
	GLint intform = GL_RGB8;
	GLenum myform = GL_RGB;
	
	if (alpha)
	{
		intform = GL_RGBA8;
		myform = GL_RGBA;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, intform, image->width(), image->height(),
	             0, myform, GL_UNSIGNED_BYTE, image->constBits());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenerateMipmap(GL_TEXTURE_2D);
	checkErrors("binding text-ure");
}

void SlipObject::bindOneTexture(Picture &pic)
{
	genTextures();
	glBindTexture(GL_TEXTURE_2D, _textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pic.width, pic.height, 
	             0, GL_RGBA, GL_UNSIGNED_BYTE, pic.data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenerateMipmap(GL_TEXTURE_2D);
	checkErrors("binding textures");
}

void SlipObject::unbindVBOBuffers()
{
	if (_textures.size())
	{
//		glDisableVertexAttribArray(4); 
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	checkErrors("vbo buffer unbinding");
}

void SlipObject::deleteVBOBuffers()
{
	std::map<QOpenGLContext *, GLuint>::iterator it;
	/*
	for (it = _vaoMap.begin(); it != _vaoMap.end(); it++)
	{
		GLuint vao = it->second;
//		glDeleteVertexArrays(1, &vao);
	}
	*/
}

void SlipObject::rebindVBOBuffers()
{
	int vao = vaoForContext();
	glBindVertexArray(vao);

	bool error = checkErrors("vertex array rebinding");
	
	if (error)
	{
		std::cout << "Vao: " << vao << std::endl;
	}

	checkErrors("vertex array attribute reenabling");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bElements[_usingProgram]);
	checkErrors("index array binding");
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize(), iPointer(), GL_STATIC_DRAW);
	checkErrors("index array buffering");
	glBindBuffer(GL_ARRAY_BUFFER, _bVertices[_usingProgram]);
	checkErrors("vbo binding");
	glBufferData(GL_ARRAY_BUFFER, vSize(), vPointer(), GL_STATIC_DRAW);
	checkErrors("vbo buffering");
}

int SlipObject::vaoForContext()
{
	QOpenGLContext *c = QOpenGLContext::currentContext();
	
	if (_vaoMap.count(c) && _vaoMap[c].count(_usingProgram))
	{
		GLuint vao = _vaoMap[c][_usingProgram];
		return vao;
	}
	
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	_vaoMap[c][_usingProgram] = vao;
	setupVBOBuffers();
	
	return vao;
}

void SlipObject::setupVBOBuffers()
{
	int vao = vaoForContext();
	glBindVertexArray(vao);
	checkErrors("binding vertex array for setup");

	GLuint bv = 0;
	glGenBuffers(1, &bv);
	_bVertices[_usingProgram] = bv;

	glBindBuffer(GL_ARRAY_BUFFER, bv);
	checkErrors("binding array buffer");
	glBufferData(GL_ARRAY_BUFFER, vSize(), vPointer(), GL_STATIC_DRAW);

	checkErrors("rebuffering data buffer");

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	if (_textured || _textures.size())
	{
		glEnableVertexAttribArray(4); 
	}

	/* Vertices */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
	                      (void *)(0 * sizeof(float)));
	checkErrors("binding vertices");

	/* Normals */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      (void *)(3 * sizeof(float)));
	checkErrors("binding indices");

	/* Colours */
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
	                      (void *)(6 * sizeof(float)));

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      (void *)(10 * sizeof(float)));

	checkErrors("binding attributes");
	
	bindTextures();

	if (_textured || _textures.size())
	{
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(14 * sizeof(float)));

		checkErrors("rebinding texture attributes");
	}

	GLuint be = 0;
	glGenBuffers(1, &be);
	_bElements[_usingProgram] = be;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, be);
	checkErrors("binding element array buffer");
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize(), iPointer(), GL_STATIC_DRAW);
	checkErrors("rebuffering data element array buffer");
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	checkErrors("unbinding vertex attribute arrays");
}

bool SlipObject::checkErrors(std::string what)
{
	GLenum err = glGetError();

	if (err != 0)
	{
		std::cout << "Error as " << _name << " was doing " << what << ":" 
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

void SlipObject::render(SlipGL *sender)
{
	if (_mesh != NULL)
	{
		_mesh->render(sender);
	}

	if (!tryLockMutex())
	{
		return;
	}

	if (_disabled)
	{
		unlockMutex();
		return;
	}
	
	if (vSize() == 0 || iSize() == 0)
	{
		unlockMutex();
		return;
	}
	
	_gl = sender;
	
	if (_program == 0)
	{
		initialisePrograms();
	}
	
	if (_gl->getOverrideProgram() > 0)
	{
		_usingProgram = _gl->getOverrideProgram();
	}
	else
	{
		_usingProgram = _program;
	}

	glUseProgram(_usingProgram);

	checkErrors("use program");
	rebindVBOBuffers();
	
	checkErrors("rebinding program");

	_model = sender->getModel();
	const char *uniform_name = "model";
	_uModel = glGetUniformLocation(_usingProgram, uniform_name);
	_glModel = mat4x4_transpose(_model);
	glUniformMatrix4fv(_uModel, 1, GL_FALSE, &_glModel.vals[0]);
	checkErrors("rebinding model");

	_proj = sender->getProjection();
	uniform_name = "projection";
	_uProj = glGetUniformLocation(_usingProgram, uniform_name);
	_glProj = mat4x4_transpose(_proj);
	glUniformMatrix4fv(_uProj, 1, GL_FALSE, &_glProj.vals[0]);
	checkErrors("rebinding projection");

	float time = sender->getTime();
	uniform_name = "time";
	_uTime = glGetUniformLocation(_usingProgram, uniform_name);
	glUniform1f(_uTime, time);
	checkErrors("rebinding time");
	
	if (_gl->getOverrideProgram() == 0)
	{
		extraUniforms();
	}

	checkErrors("rebinding extras");

	if ((_textured || _textures.size() > 0) 
	    && _gl->getOverrideProgram() == 0
	    && !_handleOwnTextures)
	{
		GLuint which = (_is2D ? GL_TEXTURE_2D : GL_TEXTURE_2D_ARRAY);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(which, _textures[0]);
		GLuint uTex = glGetUniformLocation(_usingProgram, "pic_tex");
		glUniform1i(uTex, 0);
	}
	
	if (_gl != NULL && _gl->depthMap() > 0 && !_handleOwnTextures
	    && _gl->getOverrideProgram() == 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _gl->depthMap());
		GLuint uTex = glGetUniformLocation(_usingProgram, "shadow_map");
		glUniform1i(uTex, 1);

		mat4x4 lightMat = sender->lightMat();
		uniform_name = "light_mat";
		GLuint uLight = glGetUniformLocation(_usingProgram, uniform_name);
		_glLightMat = mat4x4_transpose(lightMat);
		glUniformMatrix4fv(uLight, 1, GL_FALSE, &_glLightMat.vals[0]);
		checkErrors("rebinding lights");

	}
	
	glDrawElements(_renderType, indexCount(), GL_UNSIGNED_INT, 0);
	checkErrors("drawing elements");
	
	glUseProgram(0);
	unbindVBOBuffers();
	unlockMutex();

}

void SlipObject::setAlpha(double alpha)
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].color[3] = alpha;
	}

	for (size_t i = 0; i < _unselectedVertices.size(); i++)
	{
		_unselectedVertices[i].color[3] = alpha;
	}
}

void SlipObject::recolourBoth(double red, double green, double blue)
{
	recolour(red, green, blue);
	recolour(red, green, blue, &_unselectedVertices);
}

void SlipObject::recolour(double red, double green, double blue,
                          std::vector<Vertex> *vs)
{
	if (vs == NULL)
	{
		vs = &_vertices;
	}
	for (size_t i = 0; i < vs->size(); i++)
	{
		(*vs)[i].color[0] = red;
		(*vs)[i].color[1] = green;
		(*vs)[i].color[2] = blue;
		(*vs)[i].color[3] = 1.0;
	}
}

void SlipObject::resize(double scale, bool unselected)
{
	vec3 centre = centroid();
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 pos = vec_from_pos(_vertices[i].pos);
		vec3_subtract_from_vec3(&pos, centre);
		vec3_mult(&pos, scale);
		vec3_add_to_vec3(&pos, centre);
		pos_from_vec(_vertices[i].pos, pos);
	}
	
	if (!unselected)
	{
		resized(scale);
		return;
	}
	
	for (size_t i = 0; i < _unselectedVertices.size(); i++)
	{
		vec3 pos = vec_from_pos(_unselectedVertices[i].pos);
		vec3_subtract_from_vec3(&pos, centre);
		vec3_mult(&pos, scale);
		vec3_add_to_vec3(&pos, centre);
		pos_from_vec(_unselectedVertices[i].pos, pos);
	}
}

double SlipObject::averageRadius()
{
	vec3 centre = centroid();
	double all = 0;
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 pos = vec_from_pos(_vertices[i].pos);
		vec3_subtract_from_vec3(&pos, centre);
		double length = vec3_length(pos);
		all += length;
	}
	
	all /= (double)_vertices.size();
	return all;
}


double SlipObject::envelopeRadius()
{
	double longest = 0;

	vec3 centre = centroid();
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 pos = vec_from_pos(_vertices[i].pos);
		vec3_subtract_from_vec3(&pos, centre);
		double sqlength = vec3_sqlength(pos);
		
		if (sqlength > longest)
		{
			longest = sqlength;
		}
	}
	
	return sqrt(longest);
}

void SlipObject::changeProgram(std::string &v, std::string &f)
{
	deletePrograms();
	_vString = v;
	_fString = f;
}

void SlipObject::fixCentroid(vec3 centre)
{
	vec3 current = centroid();
	vec3 diff = vec3_subtract_vec3(centre, current);
	addToVertices(diff);
}

void SlipObject::setDisabled(bool dis)
{
	_disabled = dis;
}

vec3 SlipObject::centroid()
{
	vec3 sum = empty_vec3();
	double count = 0;

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		if (_vertices[i].pos[0] != _vertices[i].pos[0])
		{
			continue;
		}

		sum.x += _vertices[i].pos[0];
		sum.y += _vertices[i].pos[1];
		sum.z += _vertices[i].pos[2];
		count++;
	}
	
	double scale = 1 / count;
	vec3_mult(&sum, scale);
	
	return sum;
}

void SlipObject::addVertex(vec3 v, std::vector<Vertex> *vec)
{
	addVertex(v.x, v.y, v.z, vec);
}

void SlipObject::addVertex(float v1, float v2, float v3,
                           std::vector<Vertex> *vec)
{
	Vertex v;
	memset(v.pos, 0, sizeof(Vertex));

	v.color[2] = 0.;
	v.color[3] = 1.;
	v.pos[0] = v1;
	v.pos[1] = v2;
	v.pos[2] = v3;

	if (vec == NULL)
	{
		_vertices.push_back(v);
	}
	else
	{
		vec->push_back(v);
	}
}

void SlipObject::addIndices(GLuint i1, GLuint i2, GLuint i3)
{
	_indices.push_back(i1);
	_indices.push_back(i2);
	_indices.push_back(i3);
}

void SlipObject::addIndex(GLint i)
{
	if (i < 0)
	{
		_indices.push_back(_vertices.size() + i);
		return;
	}

	_indices.push_back(i);
}

bool SlipObject::index_behind_index(IndexTrio one, IndexTrio two)
{
	return (one.z > two.z);
}

bool SlipObject::index_in_front_of_index(IndexTrio one, IndexTrio two)
{
	return (one.z < two.z);
}

void SlipObject::reorderIndices()
{
	if (_renderType == GL_LINES || _renderType == GL_POINTS)
	{
		return;
	}

	_temp.resize(_indices.size() / 3);
	
	int count = 0;
	for (size_t i = 0; i < _indices.size(); i+=3)
	{
		int n = _indices[i];
		vec3 tmpVec = vec_from_pos(_vertices[n].pos);
		n = _indices[i + 1];
		vec3 tmpVec1 = vec_from_pos(_vertices[n].pos);
		n = _indices[i + 2];
		vec3 tmpVec2 = vec_from_pos(_vertices[n].pos);
		vec3_add_to_vec3(&tmpVec, tmpVec1);
		vec3_add_to_vec3(&tmpVec, tmpVec2);
		tmpVec = mat4x4_mult_vec(_model, tmpVec);
		_temp[count].z = tmpVec.z;
		_temp[count].index[0] = _indices[i];
		_temp[count].index[1] = _indices[i + 1];
		_temp[count].index[2] = _indices[i + 2];
		count++;
	}
	
	if (_backToFront)
	{
		std::sort(_temp.begin(), _temp.end(), index_behind_index);
	}
	else
	{
		std::sort(_temp.begin(), _temp.end(), index_in_front_of_index);
	}

	count = 0;

	for (size_t i = 0; i < _temp.size(); i++)
	{
		_indices[count + 0] = _temp[i].index[0];
		_indices[count + 1] = _temp[i].index[1];
		_indices[count + 2] = _temp[i].index[2];
		count += 3;
	}
}

bool xPolygon(vec3 point, vec3 *vs)
{
	bool c = false;
	
	for (int i = 0, j = 2; i < 3; j = i++) 
	{
		if (((vs[i].z > point.z) != (vs[j].z > point.z))
		    && (point.y < (vs[j].y - vs[i].y) * (point.z - vs[i].z)
		    / (vs[j].z - vs[i].z) + vs[i].y)) 
		{
			c = !c;
		}
	}

	return c;
}

bool yPolygon(vec3 point, vec3 *vs)
{
	bool c = false;
	
	for (int i = 0, j = 2; i < 3; j = i++) 
	{
		if (((vs[i].z > point.z) != (vs[j].z > point.z))
		    && (point.x < (vs[j].x - vs[i].x) * (point.z - vs[i].z)
		    / (vs[j].z - vs[i].z) + vs[i].x)) 
		{
			c = !c;
		}
	}

	return c;
}

bool zPolygon(vec3 point, vec3 *vs)
{
	bool c = false;
	
	for (int i = 0, j = 2; i < 3; j = i++) 
	{
		if (((vs[i].y > point.y) != (vs[j].y > point.y))
		    && (point.x < (vs[j].x - vs[i].x) * (point.y - vs[i].y)
		    / (vs[j].y - vs[i].y) + vs[i].x)) 
		{
			c = !c;
		}
	}

	return c;
}

bool SlipObject::pointInside(vec3 point)
{
	if (hasMesh())
	{
		return _mesh->pointInside(point);
	}

	int skip = (_renderType == GL_LINES ? 6 : 3);
	vec3 dir = make_vec3(0, 0, 1);
	bool c = false;

	for (size_t i = 0; i < _indices.size(); i += skip)
	{
		GLuint *ptr = &_indices[i];
		bool backwards = false;
		
		if ((point.x < _vertices[_indices[i]].pos[0] &&
		    point.x < _vertices[_indices[i+1]].pos[0] &&
		     point.x < _vertices[_indices[i+2]].pos[0]) ||
		(point.x > _vertices[_indices[i]].pos[0] &&
		 point.x > _vertices[_indices[i+1]].pos[0] &&
		 point.x > _vertices[_indices[i+2]].pos[0]))
		{
			continue;
		}

		if ((point.y < _vertices[_indices[i]].pos[1] &&
		    point.y < _vertices[_indices[i+1]].pos[1] &&
		     point.y < _vertices[_indices[i+2]].pos[1]) ||
		(point.y > _vertices[_indices[i]].pos[1] &&
		 point.y > _vertices[_indices[i+1]].pos[1] &&
		 point.y > _vertices[_indices[i+2]].pos[1]))
		{
			continue;
		}

		vec3 ray = rayTraceToPlane(point, ptr, dir, &backwards);
		if (backwards)
		{
			continue;
		}

		vec3 vs[3];
		vs[0] = vec_from_pos(_vertices[ptr[0]].pos);
		vs[1] = vec_from_pos(_vertices[ptr[1]].pos);
		vs[2] = vec_from_pos(_vertices[ptr[2]].pos);
		bool inside = zPolygon(ray, (vec3 *)vs);

		if (inside)
		{
			c = !c;
		}
	}

	return c;
}

vec3 SlipObject::closestRayTraceToPlane(vec3 point, GLuint *trio)
{
	vec3 vs[3];
	vs[0] = vec_from_pos(_vertices[trio[0]].pos); 
	vs[1] = vec_from_pos(_vertices[trio[1]].pos);
	vs[2] = vec_from_pos(_vertices[trio[2]].pos);

	vec3 diff1 = vec3_subtract_vec3(vs[1], vs[0]);
	vec3 diff2 = vec3_subtract_vec3(vs[2], vs[0]);
	vec3 cross = vec3_cross_vec3(diff1, diff2);
	vec3_set_length(&cross, 1); 
	
	vec3 subtract = vec3_subtract_vec3(vs[0], point);
	double d = vec3_dot_vec3(subtract, cross);
	
	vec3_mult(&cross, d);
	vec3_add_to_vec3(&point, cross);
	
	return point;
}

vec3 SlipObject::rayTraceToPlane(vec3 point, GLuint *trio, vec3 dir,
                                 bool *backwards)
{
	vec3 vs[3];
	vs[0] = vec_from_pos(_vertices[trio[0]].pos); 
	vs[1] = vec_from_pos(_vertices[trio[1]].pos);
	vs[2] = vec_from_pos(_vertices[trio[2]].pos);

	vec3 diff1 = vec3_subtract_vec3(vs[1], vs[0]);
	vec3 diff2 = vec3_subtract_vec3(vs[2], vs[0]);
	vec3 cross = vec3_cross_vec3(diff1, diff2);
	vec3_set_length(&cross, 1); 
	
	double denom = vec3_dot_vec3(dir, cross);
	vec3 subtract = vec3_subtract_vec3(vs[0], point);
	double nom = vec3_dot_vec3(subtract, cross);
	double d = nom / denom;
	
	vec3_mult(&dir, d);
	vec3_add_to_vec3(&point, dir);
	
	*backwards = (d < 0);
	return point;
}

bool SlipObject::polygonIncludesY(vec3 point, GLuint *trio)
{
	vec3 vs[3];
	vs[0] = vec_from_pos(_vertices[trio[0]].pos);
	vs[1] = vec_from_pos(_vertices[trio[1]].pos);
	vs[2] = vec_from_pos(_vertices[trio[2]].pos);

	return yPolygon(point, (vec3 *)vs);
}

bool SlipObject::polygonIncludes(vec3 point, GLuint *trio)
{
	vec3 vs[3];
	vs[0] = vec_from_pos(_vertices[trio[0]].pos);
	vs[1] = vec_from_pos(_vertices[trio[1]].pos);
	vs[2] = vec_from_pos(_vertices[trio[2]].pos);

	double xmin = std::min(std::min(vs[0].x, vs[1].x), vs[2].x);
	double ymin = std::min(std::min(vs[0].y, vs[1].y), vs[2].y);
	double zmin = std::min(std::min(vs[0].z, vs[1].z), vs[2].z);
	
	double xmax = std::max(std::max(vs[0].x, vs[1].x), vs[2].x);
	double ymax = std::max(std::max(vs[0].y, vs[1].y), vs[2].y);
	double zmax = std::max(std::max(vs[0].z, vs[1].z), vs[2].z);
	
	double zdiff = zmax - zmin;
	double ydiff = ymax - ymin;
	double xdiff = xmax - xmin;
	
	if (zdiff < ydiff && zdiff < xdiff)
	{
		return zPolygon(point, (vec3 *)vs);
	}
	else if (ydiff < zdiff && ydiff < xdiff)
	{
		return yPolygon(point, (vec3 *)vs);
	}
	else
	{
		return xPolygon(point, (vec3 *)vs);
	}
}

vec3 SlipObject::nearestVertexNearNormal(vec3 pos, vec3 normal,
                                         bool *isBehind)
{
	double closest = FLT_MAX;
	Vertex *vClose = NULL;
	*isBehind = false;
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		Vertex v = _vertices[i];

		vec3 diff = make_vec3(pos.x - v.pos[0],
		                      pos.y - v.pos[1],
		                      pos.z - v.pos[2]);

		double length = vec3_length(diff);
		vec3_mult(&diff, 1 / length);
		
		double dot = vec3_dot_vec3(diff, normal);
		
		if (fabs(dot) < _meshDot) { continue; }
		
		if (dot < 0) { *isBehind = true; }
		
		if (*isBehind && dot > 0) { continue; }
		
		if (length < closest)
		{
			closest = length;
			vClose = &_vertices[i];
		}
	}
	
	if (vClose == NULL)
	{
		return nearestVertex(pos);
	}

	vec3 finvec = vec_from_pos(vClose->pos);
	return finvec;

}

Vertex *SlipObject::nearestVertexPtr(vec3 pos, bool useMesh)
{
	if (useMesh && _mesh != NULL)
	{
		return _mesh->nearestVertexPtr(pos, false);
	}

	double closest = FLT_MAX;
	Vertex *vClose = NULL;
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		Vertex v = _vertices[i];
		vec3 diff = make_vec3(pos.x - v.pos[0],
		                      pos.y - v.pos[1],
		                      pos.z - v.pos[2]);

		if (abs(diff.x) > closest || abs(diff.y) > closest 
		    || abs(diff.z) > closest)
		{
			continue;
		}

		double length = vec3_sqlength(diff);
		
		if (length < closest)
		{
			closest = length;
			vClose = &_vertices[i];
		}
	}
	
	return vClose;
}

vec3 SlipObject::randomVertex()
{
	size_t which = rand() % _vertices.size();

	vec3 v = vec_from_pos(_vertices[which].pos);
	
	return v;
}

vec3 SlipObject::nearestVertex(vec3 pos, bool useMesh)
{
	if (useMesh && _mesh != NULL)
	{
		return _mesh->nearestVertex(pos, false);
	}

	Vertex *vClose = nearestVertexPtr(pos, useMesh);

	vec3 finvec = vec_from_pos(vClose->pos);
	return finvec;
}

void SlipObject::boundaries(vec3 *min, vec3 *max)
{
	*min = make_vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	*max = make_vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		Vertex v = _vertices[i];
		if (v.pos[0] < min->x) min->x = v.pos[0];
		if (v.pos[1] < min->y) min->y = v.pos[1];
		if (v.pos[2] < min->z) min->z = v.pos[2];

		if (v.pos[0] > max->x) max->x = v.pos[0];
		if (v.pos[1] > max->y) max->y = v.pos[1];
		if (v.pos[2] > max->z) max->z = v.pos[2];
	}
}

void SlipObject::changeMidPoint(double x, double y)
{
	vec3 pos = centroid();
	double last = 1;

	vec3 model = mat4x4_mult_vec3(_model, pos, &last);
	mat4x4_mult_vec3(_proj, model, &last);
	
	double newx = last * x / _proj.vals[0];
	double newy = last * y / _proj.vals[5];
	vec3 move = make_vec3(newx - model.x, newy - model.y, 0);

	mat3x3 rot = mat4x4_get_rot(_model);

	vec3 newpos = mat3x3_mult_vec(rot, move);

	addToVertices(newpos);
	
}

double SlipObject::intersects(vec3 pos, vec3 dir)
{
	double closest = FLT_MAX;

	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		bool back = false;
		vec3 intersect = rayTraceToPlane(pos, &_indices[i], dir, &back);
		
		if (back)
		{
			continue;
		}

		bool passes = polygonIncludes(intersect, &_indices[i]);

		if (passes)
		{
			vec3 a = vec_from_pos(_vertices[_indices[i+2]].pos);
			vec3 b = vec_from_pos(_vertices[_indices[i+1]].pos);
			vec3 c = vec_from_pos(_vertices[_indices[i]].pos);
			vec3_add_to_vec3(&c, a);
			vec3_add_to_vec3(&c, b);
			vec3_mult(&c, 1./3.);
			vec3_subtract_from_vec3(&c, pos);

			double l = vec3_length(c);
			
			if (l < closest)
			{
				closest = l;
			}
		}
	}
	
	if (closest == FLT_MAX)
	{
		return -1;
	}
	
	return closest;
}

bool SlipObject::intersectsPolygon(double x, double y, double *z)
{
	vec3 target = make_vec3(x, y, 0);
	
	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		vec3 projs[3];
		memset(&projs[0], '\0', sizeof(vec3) * 3);

		for (int j = 0; j < 3; j++)
		{
			projs[j] = vec_from_pos(_vertices[_indices[i+j]].pos);

			double last = 1;
			vec3 model = mat4x4_mult_vec3(_model, projs[j], &last);
			projs[j] = mat4x4_mult_vec3(_proj, model, &last);

			vec3_mult(&projs[j], 1 / last);
			
			if (model.z > 0 || model.z < *z)
			{
				continue;
			}
			
			if (projs[j].x < -1 || projs[j].x > 1)
			{
				break;
			}
			
			if (projs[j].y < -1 || projs[j].y > 1)
			{
				break;
			}
		}
		
		bool passes = zPolygon(target, projs);

		if (passes)
		{
			*z = (projs[0].z + projs[1].z + projs[2].z) / 3;
			return true;
		}
	}
	
	return false;
}

bool SlipObject::intersects(double x, double y, double *z, int *vidx)
{
	vec3 target = make_vec3(x, y, 0);
	bool found = false;
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 pos = vec_from_pos(_vertices[i].pos);
		
		if (_central)
		{
			pos = centroid();
		}
		
		double last = 1;
		vec3 model = mat4x4_mult_vec3(_model, pos, &last);
		vec3 proj = mat4x4_mult_vec3(_proj, model, &last);
		
		vec3_mult(&proj, 1 / last);

		if (proj.x < -1 || proj.x > 1)
		{
			continue;
		}

		if (proj.y < -1 || proj.y > 1)
		{
			continue;
		}
		
		if (model.z > 0)
		{
			continue;
		}

		vec3 diff = vec3_subtract_vec3(proj, target);
		
		if (fabs(diff.x) < 0.04 && fabs(diff.y) < 0.04)
		{
			if (model.z > *z)
			{
				if (vidx != NULL)
				{
					*vidx = i;
				}
				*z = model.z;
				found = true;
			}
		}
		
		if (_central)
		{
			break;
		}
	}
	
	return found;
}

void SlipObject::calculateNormalsAndCheck()
{
	std::cout << "Checking normals ... " << std::endl;
	int flipped = 0;

	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		vec3 pos1 = vec_from_pos(_vertices[_indices[i+0]].pos);
		vec3 pos2 = vec_from_pos(_vertices[_indices[i+1]].pos);
		vec3 pos3 = vec_from_pos(_vertices[_indices[i+2]].pos);
		
		vec3 diff31 = vec3_subtract_vec3(pos3, pos1);
		vec3 diff21 = vec3_subtract_vec3(pos2, pos1);

		vec3 cross = vec3_cross_vec3(diff31, diff21);
		vec3_add_to_vec3(&pos1, cross);

		if (!pointInside(pos1))
		{
			GLuint tmp = _indices[i+0];
			_indices[i+0] = _indices[i+1];
			_indices[i+1] = tmp;
			flipped++;
		}
	}

	std::cout << "Flipped " << flipped << std::endl;
	calculateNormals();
}

void SlipObject::flip()
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].normal[0] *= -1;
		_vertices[i].normal[1] *= -1;
		_vertices[i].normal[2] *= -1;
	}
}

void SlipObject::calculateNormals(bool flip)
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].normal[0] = 0;
		_vertices[i].normal[1] = 0;
		_vertices[i].normal[2] = 0;
	}
	
	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		vec3 pos1 = vec_from_pos(_vertices[_indices[i+0]].pos);
		vec3 pos2 = vec_from_pos(_vertices[_indices[i+1]].pos);
		vec3 pos3 = vec_from_pos(_vertices[_indices[i+2]].pos);

		vec3 diff31 = vec3_subtract_vec3(pos3, pos1);
		vec3 diff21 = vec3_subtract_vec3(pos2, pos1);

		vec3 cross = vec3_cross_vec3(diff31, diff21);
		vec3_set_length(&cross, 1);
		
		if (cross.x != cross.x)
		{
			continue;
		}
		
		/* Normals */					
		for (int j = 0; j < 3; j++)
		{
			_vertices[_indices[i + j]].normal[0] += cross.x;
			_vertices[_indices[i + j]].normal[1] += cross.y;
			_vertices[_indices[i + j]].normal[2] += cross.z;
		}
	}

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 norm = vec_from_pos(_vertices[i].normal);
		vec3_set_length(&norm, 1);

		pos_from_vec(_vertices[i].normal, norm);
	}
}

void SlipObject::setSelectable(bool selectable)
{
	if (selectable)
	{
		_unselectedVertices = _vertices;
	}
	else
	{
		_unselectedVertices.clear();
	}

	_selectable = selectable;
}

void SlipObject::setHighlighted(bool highlighted)
{
	if (!_selectable)
	{
		return;
	}
	
	if (highlighted && !_highlighted && !_selected)
	{
		_vertices = _unselectedVertices;
		resize(1.3);
	}
	
	if (!highlighted && _highlighted && !_selected)
	{
		_vertices = _unselectedVertices;
	}

	_highlighted = highlighted;
}

void SlipObject::setSelected(bool selected)
{
	if (!_selected && selected)
	{
		recolour(0.7, 0.7, 1);
		
		if (!_highlighted)
		{
			resize(_selectionResize);
		}
	}
	
	if ((_selected || _highlighted) && !selected)
	{
		_vertices = _unselectedVertices;
	}

	_highlighted = selected;
	_selected = selected;
}

bool SlipObject::collapseCommonVertices(int quick, double thresh)
{
	std::cout << "Collapsing common vertices..." << std::endl;
	size_t prior = _vertices.size();

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		size_t target = (quick > 0 ? quick : _vertices.size());
		for (size_t j = i + 1; j < i + target + 1 && j < _vertices.size(); j++)
		{
			if (fabs(_vertices[i].pos[0] - _vertices[j].pos[0]) > thresh)
			{
				continue;
			}
			if (fabs(_vertices[i].pos[1] - _vertices[j].pos[1]) > thresh)
			{
				continue;
			}
			if (fabs(_vertices[i].pos[2] - _vertices[j].pos[2]) > thresh)
			{
				continue;
			}

			/* duplicate */
			for (size_t k = 0; k < _indices.size(); k++)
			{
				if (_indices[k] == j)
				{
					_indices[k] = i;
				}
			}
		}
	}
	
	removeUnusedVertices();

	size_t post = _vertices.size();
	
	std::cout << "From " << prior << " to " << post << " vertices." << std::endl;
	std::cout << _indices.size() / 3 << " faces." << std::endl;
	
//	calculateNormals();
	
	return (post < prior);
}

void SlipObject::writeObjFile(std::string filename)
{
	std::ofstream file;
	file.open(filename);
	std::cout << "Writing to " << filename << std::endl;

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		file << "v " << std::setprecision(10) << 
		_vertices[i].pos[0] << " " <<
		_vertices[i].pos[1] << " " <<
		_vertices[i].pos[2] << std::endl;
		file << "vn " << _vertices[i].normal[0] << " " <<
		_vertices[i].normal[1] << " " <<
		_vertices[i].normal[2] << std::endl;
		file << "vt " << _vertices[i].tex[0] << " " <<
		_vertices[i].tex[1] << std::endl;
	}
	
	file << std::endl;

	for (size_t k = 0; k < _indices.size(); k += 3)
	{
		file << "f " << _indices[k] + 1 << "/" << 
		_indices[k + 0] + 1 << "/" << _indices[k + 0] + 1
		<< " " << _indices[k + 1] + 1 << "/" << 
		_indices[k + 1] + 1 << "/" << _indices[k + 1] + 1
		<< " " << _indices[k + 2] + 1 << "/" <<
		_indices[k + 2] + 1 << "/" << _indices[k + 2] + 1 << std::endl;
	}
	
	file << std::endl;
	
	file.close();
}

void SlipObject::cacheTriangulate()
{
	std::map<std::pair<GLuint, GLuint>, GLuint> lines;
	std::map<std::pair<GLuint, GLuint>, GLuint>::iterator linesit;

	std::map<std::pair<GLuint, GLuint>, GLuint> done_pairs;
	std::map<GLuint, GLuint> done;

	std::map<GLuint, std::map<GLuint, GLuint> > newVs;
	std::vector<Vertex> vs;
	
	std::cout << "Triangulating..." << std::endl;

	/* lookup for all your trios */
	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		std::pair<GLuint, GLuint> pair = std::make_pair(_indices[i], 
		                                                _indices[i+1]);
		lines[pair] = _indices[i+2];
	}
	
	_indices.clear();

	/* iterate through all your trios */
	for (linesit = lines.begin(); linesit != lines.end(); linesit++)
	{
		GLuint add = 6;
		std::pair<GLuint, GLuint> pair = linesit->first;
		GLuint i1 = pair.first;
		GLuint i2 = pair.second;
		GLuint i3 = linesit->second;
		
		vec3 v1 = vec_from_pos(_vertices[i1].pos);
		vec3 v2 = vec_from_pos(_vertices[i2].pos);
		vec3 v3 = vec_from_pos(_vertices[i3].pos);
		
		int c0, c1, c2;
		int c3, c4, c5;
		
		vec3 v12 = vec3_add_vec3(v1, v2);
		vec3_mult(&v12, 0.5);
		vec3 v23 = vec3_add_vec3(v2, v3);
		vec3_mult(&v23, 0.5);
		vec3 v31 = vec3_add_vec3(v3, v1);
		vec3_mult(&v31, 0.5);

		if (done.count(i1))
		{
			c0 = done[i1];
			add--;
		}
		else
		{
			addVertex(v1, &vs);
			c0 = vs.size() - 1;
		}

		if (done.count(i2))
		{
			c1 = done[i2];
			add--;
		}
		else
		{
			addVertex(v2, &vs);
			c1 = vs.size() - 1;
		}

		if (done.count(i3))
		{
			c2 = done[i3];
			add--;
		}
		else
		{
			addVertex(v3, &vs);
			c2 = vs.size() - 1;
		}
		
		std::pair<GLuint, GLuint> fb, bo, of;
		fb = std::make_pair(i1, i2);
		bo = std::make_pair(i2, i3);
		of = std::make_pair(i3, i1);

		if (done_pairs.count(fb))
		{
			c3 = done_pairs[fb];
			add--;
		}
		else
		{
			addVertex(v12, &vs);
			c3 = vs.size() - 1;
		}

		if (done_pairs.count(bo))
		{
			c4 = done_pairs[bo];
			add--;
		}
		else
		{
			addVertex(v23, &vs);
			c4 = vs.size() - 1;
		}

		if (done_pairs.count(of))
		{
			c5 = done_pairs[of];
			add--;
		}
		else
		{
			addVertex(v31, &vs);
			c5 = vs.size() - 1;
		}
		
		done[i1] = c0;
		done[i2] = c1;
		done[i3] = c2;
		done_pairs[std::make_pair(i1, i2)] = c3;
		done_pairs[std::make_pair(i2, i1)] = c3;
		done_pairs[std::make_pair(i2, i3)] = c4;
		done_pairs[std::make_pair(i3, i2)] = c4;
		done_pairs[std::make_pair(i3, i1)] = c5;
		done_pairs[std::make_pair(i1, i3)] = c5;
		
		for (size_t i = vs.size() - add; i < vs.size(); i++)
		{
			vs[i].color[0] = _vertices[i1].color[0];
			vs[i].color[1] = _vertices[i1].color[1];
			vs[i].color[2] = _vertices[i1].color[2];
		}

		addIndices(c0, c3, c5);
		addIndices(c5, c3, c4);
		addIndices(c5, c4, c2);
		addIndices(c4, c3, c1);
	}
	
	_vertices = vs;
	std::cout << "Vertices: " << _vertices.size() << std::endl;
	std::cout << _indices.size() / 3 << " faces." << std::endl;
	
	calculateNormals();
}

void SlipObject::triangulate()
{
	if (_renderType == GL_LINES)
	{
		return;
	}
	
	lockMutex();

	std::map<std::pair<GLuint, GLuint>, GLuint> lines;
	std::map<std::pair<GLuint, GLuint>, GLuint>::iterator linesit;

	std::map<GLuint, std::map<GLuint, GLuint> > newVs;

	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		for (int j = 0; j < 3; j++)
		{
			int ij = i + (j % 3);
			int ij1 = i + ((j + 1) % 3);
			int min = std::min(_indices[ij], _indices[ij1]);
			int max = std::max(_indices[ij], _indices[ij1]);
			
			std::pair<GLuint, GLuint> pair = std::make_pair(min, max);

			lines[pair] = 1;
		}
	}

	for (linesit = lines.begin(); linesit != lines.end(); linesit++)
	{
		std::pair<GLuint, GLuint> pair = linesit->first;
		GLuint front = pair.first;
		GLuint back = pair.second;
		
		vec3 v1 = vec_from_pos(_vertices[front].pos);
		vec3 v2 = vec_from_pos(_vertices[back].pos);
		double x = (_vertices[front].tex[0] + _vertices[back].tex[0]) / 2;
		double y = (_vertices[front].tex[1] + _vertices[back].tex[1]) / 2;
		
		vec3_add_to_vec3(&v1, v2);
		vec3_mult(&v1, 0.5);
		
		Vertex v = _vertices[front]; 
		pos_from_vec(v.pos, v1);
		v.tex[0] = x;
		v.tex[1] = y;
		
		_vertices.push_back(v);
		newVs[front][back] = _vertices.size() - 1;
		newVs[back][front] = _vertices.size() - 1;
	}
	
	size_t idxSize = _indices.size();
	for (size_t i = 0; i < idxSize; i += 3)
	{
		GLuint i1 = _indices[i];
		GLuint i2 = _indices[i + 1];
		GLuint i3 = _indices[i + 2];
		
		GLuint i12 = newVs[i1][i2];
		GLuint i13 = newVs[i1][i3];
		GLuint i23 = newVs[i2][i3];
		
		_indices[i+1] = i12;
		_indices[i+2] = i13;
		
		addIndices(i13, i12, i23);
		addIndices(i13, i23, i3);
		addIndices(i23, i12, i2);
	}
	
	calculateNormals();
	
	unlockMutex();
}

void SlipObject::changeToLines()
{
	if (_renderType == GL_LINES)
	{
		return;
	}

	_renderType = GL_LINES;
	std::vector<GLuint> is = _indices;
	_indices.clear();

	for (size_t i = 0; i < is.size(); i += 3)
	{
		addIndices(is[i], is[i + 1], is[i + 2]);
		addIndices(is[i], is[i + 1], is[i + 2]);
	}
}

void SlipObject::changeToTriangles()
{
	if (_renderType == GL_TRIANGLES)
	{
		return;
	}

	_renderType = GL_TRIANGLES;
	std::vector<GLuint> is = _indices;
	_indices.clear();

	for (size_t i = 0; i < is.size(); i += 6)
	{
		addIndices(is[i], is[i + 1], is[i + 2]);
	}

}

void SlipObject::setCustomMesh(Mesh *m)
{
	_mesh = m;
}

Mesh *SlipObject::makeMesh(int tri)
{
	_mesh = new Mesh(this, tri);
	return _mesh;
}

void SlipObject::colourOutlayBlack()
{
	for (size_t i = 0; i < vertexCount(); i++)
	{
		Vertex v = vertex(i);
		vec3 p = vec_from_pos(v.pos);
		
		if (!pointInside(p))
		{
			v.color[0] = 0;
			v.color[1] = 0;
			v.color[2] = 0.2;
			
			lockMutex();
			setVertex(i, v);
			unlockMutex();
		}
	}
}

void SlipObject::clearMesh()
{
	if (!hasMesh())
	{
		return;
	}

	_mesh->remove();
	_mesh = NULL;
}

void SlipObject::copyFrom(SlipObject *s)
{
	_vertices = s->_vertices;
	_indices = s->_indices;
	_unselectedVertices = s->_unselectedVertices;
}

void SlipObject::removeUnusedVertices()
{
	std::vector<bool> flags = std::vector<bool>(_vertices.size(), false);
	std::vector<int> offsets = std::vector<int>(_vertices.size(), 0);

	for (size_t i = 0; i < _indices.size(); i++)
	{
		if (_indices[i] >= flags.size())
		{
			std::cout << "Flag " << i << " out of bounds!" << std::endl;
		}
		if (flags[_indices[i]] == false)
		{
			flags[_indices[i]] = true;
		}
	}

	for (size_t i = 1; i < flags.size(); i++)
	{
		offsets[i] = offsets[i-1] + 1;

		if (flags[i] == false)
		{
			offsets[i] = offsets[i-1];
		}
	}
	
	std::cout << "Final offset: " << offsets.back() << std::endl;

	for (size_t i = 0; i < _indices.size(); i++)
	{
		int offset = offsets[_indices[i]];
		_indices[i] = offset;
	}

	std::vector<Vertex> vs;
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		if (flags[i])
		{
			vs.push_back(_vertices[i]);
		}
	}
	
	_vertices = vs;
	std::cout << "Vertex count now " << vertexCount() << std::endl;
}

void SlipObject::changeVertexShader(std::string v)
{
	_vString = v;
	deletePrograms();
}

void SlipObject::changeFragmentShader(std::string f)
{
	_fString = f;
	deletePrograms();
}

void SlipObject::setFocalPoint(vec3 vec)
{
	if (!_usesFocalDepth)
	{
		return;
	}
	
	_focus = vec;
}

void SlipObject::setPosition(vec3 pos)
{
	vec3 p = centroid();

	vec3 diff = vec3_subtract_vec3(pos, p);
	lockMutex();
	addToVertices(diff);
	unlockMutex();
	
	if (hasMesh())
	{
		mesh()->setPosition(pos);
	}
	
	positionChanged();
}

void SlipObject::rotateByMatrix(mat3x3 m)
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 v = vec_from_pos(_vertices[i].pos);
		vec3 n = vec_from_pos(_vertices[i].normal);
		mat3x3_mult_vec(m, &v);
		mat3x3_mult_vec(m, &n);
		pos_from_vec(_vertices[i].pos, v);
		pos_from_vec(_vertices[i].normal, n);
	}
}

void SlipObject::rotate(mat3x3 &rot)
{
	vec3 c = centroid();

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 v = vec_from_pos(_vertices[i].pos);
		vec3 n = vec_from_pos(_vertices[i].normal);
		vec3_subtract_from_vec3(&v, c);
		mat3x3_mult_vec(rot, &v);
		mat3x3_mult_vec(rot, &n);
		vec3_add_to_vec3(&v, c);
		pos_from_vec(_vertices[i].pos, v);
		pos_from_vec(_vertices[i].normal, n);

		if (isSelectable())
		{
			memcpy(&_unselectedVertices[i].pos, &_vertices[i].pos,
			       sizeof(GLfloat) * 3);
		}
	}
}

vec3 SlipObject::findClosestVecToSurface(vec3 &interior)
{
	if (hasMesh())
	{
		return mesh()->findClosestVecToSurface(interior);
	}

	double distance = FLT_MAX;
	vec3 closest = make_vec3(FLT_MAX, FLT_MAX, FLT_MAX);

	for (size_t i = 0; i < indexCount(); i += 3)
	{
		vec3 close = closestRayTraceToPlane(interior, iPointer() + i);
		
		bool included = polygonIncludes(close, iPointer() + i);
		
		if (!included)
		{
			continue;
		}

		vec3_subtract_from_vec3(&close, interior);

		double l = vec3_length(close);
		
		if (l >= distance || l != l)
		{
			continue;
		}
		
		closest = close;
		distance = l;
	}

	vec3_mult(&closest, -1);
	return closest;
}

void SlipObject::prepareNormalVision()
{
	if (_normals != NULL)
	{
		delete _normals;
		_normals = NULL;
	}

	_normals = new SlipObject();
	_normals->_renderType = GL_LINES;
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 v = vec_from_pos(_vertices[i].pos);
		vec3 n = vec_from_pos(_vertices[i].normal);
		_normals->addVertex(v.x, v.y, v.z);
		_normals->addIndex(-1);
		vec3_add_to_vec3(&v, n);
		_normals->addVertex(v.x, v.y, v.z);
		_normals->addIndex(-1);
	}
}

void SlipObject::appendObject(SlipObject *object)
{
	int add = _vertices.size();
	_vertices.reserve(_vertices.size() + object->vertexCount());
	_indices.reserve(_indices.size() + object->indexCount());
	
	for (size_t i = 0; i < object->vertexCount(); i++)
	{
		Helen3D::Vertex &v = object->_vertices[i];
		_vertices.push_back(v);
	}

	for (size_t i = 0; i < object->_indices.size(); i++)
	{
		long idx = object->_indices[i] + add;
		_indices.push_back(idx);
	}
}

void SlipObject::heatToVertex(Helen3D::Vertex &v, double heat)
{
	if (heat < 0) heat = 0;
	vec3 colour_start, colour_aim;
	if (heat >= -1 && heat < 0.5)
	{
		colour_start = make_vec3(0.4, 0.4, 0.4); // grey
		colour_aim = make_vec3(0.55, 0.45, 0.29); // straw
	}
	else if (heat >= 0.5 && heat < 1)
	{
		colour_start = make_vec3(0.55, 0.45, 0.29); // straw
		colour_aim = make_vec3(0.39, 0.46, 0.68); // blue
	}
	else if (heat >= 1 && heat < 2)
	{
		colour_start = make_vec3(0.39, 0.46, 0.68); // blue
		colour_aim = make_vec3(0.68, 0.16, 0.08); // cherry red
	}
	else if (heat >= 2 && heat < 3)
	{
		colour_start = make_vec3(0.68, 0.16, 0.08); // cherry red
		colour_aim = make_vec3(0.92, 0.55, 0.17); // orange
	}
	else if (heat >= 3)
	{
		colour_start = make_vec3(0.92, 0.55, 0.17); // orange
		colour_aim = make_vec3(0.89, 0.89, 0.16); // yellow
	}

	double mult = heat - 1;
	if (mult < 0) mult = 0;
	mult *= 3;
	heat = fmod(heat, 1);
	colour_aim -= colour_start;
	vec3_mult(&colour_aim, heat);
	colour_start += colour_aim;
	pos_from_vec(v.color, colour_start);
	vec3_mult(&colour_start, mult);
	pos_from_vec(v.extra, colour_start);
}
