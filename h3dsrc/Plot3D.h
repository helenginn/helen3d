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

#ifndef __cluster4x__Plot3D__
#define __cluster4x__Plot3D__

#include "SlipObject.h"
#include <QObject>
#include <hcsrc/vec3.h>
#include <h3dsrc/Text.h>

class SlipGL;

class Plot3D : public QObject, public SlipObject
{
Q_OBJECT
public:
	Plot3D();
	
	static bool usesDepth()
	{
		return _depth;
	}
	
	static bool showsText()
	{
		return _drawText;
	}
	
	static float fontSize()
	{
		return _textSize;
	}
	
	static float pointSize()
	{
		return _size;
	}

	static void setDepthCue(bool cue)
	{
		_depth = cue;
	}

	static void setShowText(bool draw)
	{
		_drawText = draw;
	}

	static void setFontSize(float size)
	{
		_textSize = size;
	}

	static void setPointSize(float size)
	{
		_size = size;
	}
	
	void setKeeper(SlipGL *gl)
	{
		_keeper = gl;
	}
	
	void setAxes(int a, int b, int c)
	{
		_a = a;
		_b = b;
		_c = c;
	}

	void addPoint(vec3 point, std::string text = "");

	/* add 1 = add to selection
	 * add 0 = replace selection
	 * add -1 = remove from selection */
	virtual void selectInWindow(float x1, float y1, float x2, float y2,
	                            int add);
	
	virtual size_t axisCount() = 0;
	virtual std::string axisLabel(int i) = 0;
	
	virtual void populate() = 0;
	virtual void repopulate();
signals:
	void updateSelection();
protected:
	virtual void extraUniforms();
	void addText(std::string text, vec3 point);
	SlipGL *_keeper;
	
	virtual void render(SlipGL *gl);
	virtual void recolour();
	std::vector<Text *> _texts;
	static float _size;
	static float _textSize;
	static bool _drawText;
	static bool _depth;

	int _a;
	int _b;
	int _c;
};

#endif
