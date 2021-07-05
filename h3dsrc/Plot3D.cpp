// cluster4x
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

#include "Frameworks.h"
#include <iostream>
using namespace Helen3D;

#include "SlipGL.h"
#include "Plot3D.h"
#include "shaders/Blob_fsh.h"
#include "shaders/Blob_vsh.h"

float Plot3D::_textSize = 8;
float Plot3D::_size = 20;
bool Plot3D::_drawText = false;
bool Plot3D::_depth = false;

Plot3D::Plot3D() : SlipObject()
{
	_keeper = NULL;
	_renderType = GL_POINTS;
	_a = 0;
	_b = 1;
	_c = 2;
	_fString = Blob_fsh();
	_vString = Blob_vsh();
}

void Plot3D::render(SlipGL *gl)
{
	for (size_t i = 0; i < _texts.size() && _drawText; i++)
	{
		_texts[i]->render(gl);
	}

	SlipObject::render(gl);
}

void Plot3D::addPoint(vec3 point, std::string text)
{
	_indices.push_back(_vertices.size());

	Vertex v;
	memset(v.pos, 0, sizeof(Vertex));

	v.color[3] = 1;
	
	v.pos[0] = point.x;
	v.pos[1] = point.y;
	v.pos[2] = point.z;

	_vertices.push_back(v);
	
	addText(text, point);
}

void Plot3D::addText(std::string str, vec3 point)
{
	if (!_drawText)
	{
		return;
	}

	Text *text = new Text();

	if (str.length())
	{
		text->setProperties(point, str, _textSize, Qt::black, 0, 0.08, 0.0);
		text->prepare();
	}
	_texts.push_back(text);
}

void Plot3D::selectInWindow(float x1, float y1, float x2, float y2,
                             int add)
{
	emit updateSelection();
	recolour();
}

void Plot3D::repopulate()
{
	_vertices.clear();
	_indices.clear();
	for (size_t i = 0; i < _texts.size(); i++)
	{
		delete _texts[i];
	}
	_texts.clear();

	populate();

	recolour();
	if (_keeper)
	{
		_keeper->update();
	}
}


void Plot3D::recolour()
{

}

void Plot3D::extraUniforms()
{
	{
	const char *uniform_name = "size";
	GLuint u = glGetUniformLocation(_usingProgram, uniform_name);
	glUniform1f(u, _size);
	checkErrors("rebinding size");
	}

	{
	const char *uniform_name = "depth";
	GLuint uDepth = glGetUniformLocation(_usingProgram, uniform_name);
	glUniform1i(uDepth, _depth);
	checkErrors("rebinding depth");
	}
}
