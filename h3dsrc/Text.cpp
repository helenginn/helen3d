// helen3d
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

#include "Text.h"
#include "shaders/shText.h"
#include <iostream>
#include <QFontMetrics>
#include <QFont>
#include <QImage>
#include <QPainter>
#include <QColor>

Text::Text() : SlipObject()
{
	_renderType = GL_POINTS;
	_vString = vText();
	_gString = gText();
	_fString = fText();
	setNeedsExtra(true);
	setName("Text");
	_size = 24;
	_colour = Qt::black;
	_pos = empty_vec3();
	_offset = empty_vec3();
	_image = NULL;
}

Text::~Text()
{
	if (_image != NULL)
	{
		delete _image;
	}
	
	_image = NULL;
}

void Text::prepare()
{
	_vertices.clear();
	_indices.clear();

	if (_text.length() == 0)
	{
		return;
	}
	
	const float scale = 0.0005;

	QString str = QString::fromStdString(" " + _text);
	QFont font("Sans Serif", _size * 8);
	QFontMetrics fm(font);
	float drawHeight = fm.height();
	float width = fm.width(str);
	_width = width * 1.2;
	_height = drawHeight * 1.5;

	_image = new QImage(_width, _height, QImage::Format_RGB888);
	QPainter painter(_image);
	painter.fillRect(0, 0, _width, _height, Qt::white);
	painter.setPen(_colour);
	painter.setFont(font);
	painter.drawText(width * 0.2, drawHeight, str);

	Helen3D::Vertex vertex;
	memset(&vertex, '\0', sizeof(Helen3D::Vertex));
	vertex.color[3] = 1;
	pos_from_vec(vertex.pos, _pos);
	pos_from_vec(vertex.normal, _offset);
	vertex.normal[0] -= _width * scale / 4;
	vertex.extra[0] = _width * scale;
	vertex.extra[1] = _height * scale;
	
	_vertices.push_back(vertex);
	_indices.push_back(0);
}

void Text::bindTextures()
{
	bindOneTexture(_image);
}

void Text::setColour(float r, float g, float b, float a)
{
	{
		_colour = QColor(r * 255, g * 255, b * 255, a * 255);
	}
}
