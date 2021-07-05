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

#ifndef __helen3d__text__
#define __helen3d__text__

#include "SlipObject.h"
#include <QColor>

class QImage;

class Text : public SlipObject
{
public:
	Text();
	
	virtual ~Text();
	
	void setColour(float r, float g, float b, float a);
	
	void setModelPos(vec3 v)
	{
		_pos = v;
	}

	void setText(std::string text)
	{
		_text = text;
	}
	
	void setProperties(vec3 v, std::string text, size_t size = 24,
	                   QColor colour = Qt::black, float offx = 0, 
	                   float offy = 0, float offz = 0)
	{
		_pos = v;
		_text = text;
		_size = size;
		_colour = colour;
		_offset.x = offx;
		_offset.y = offy;
		_offset.z = offz;
	}
	
	void prepare();
	virtual void bindTextures();
private:
	vec3 _pos;
	vec3 _offset;
	int _width;
	int _height;
	int _size;

	QImage *_image;
	QColor _colour;
	std::string _text;
};

#endif
