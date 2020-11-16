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
#include "vec3.h"

class SlipGL;

class Plot3D : public QObject, public SlipObject
{
Q_OBJECT
public:
	Plot3D();

	
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

	void addPoint(vec3 point);

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
	SlipGL *_keeper;
	virtual void recolour();

	int _a;
	int _b;
	int _c;
};

#endif
