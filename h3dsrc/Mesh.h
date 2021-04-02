// abmap
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

#ifndef __abmap__Mesh__
#define __abmap__Mesh__

#include <QObject>
#include "Icosahedron.h"

class SlipGL;

class Mesh : public QObject, public Icosahedron
{
Q_OBJECT
public:
	Mesh(SlipObject *other, int triangulations = 3);

signals:
	void resultReady();
public slots:
	void shrinkWrap();
	void smoothCycles();
	void inflateCycles();

protected:
	int _wrapCycles;
	int _smoothCycles;
private:
	void smoothen(std::vector<Helen3D::Vertex> &vcopy);
	void hug(std::vector<Helen3D::Vertex> &vcopy);
	
	virtual vec3 getTargetPos(vec3 meshPos, vec3 meshDir, bool *behind);

	SlipObject *_parent;

	static double _speed;
};

#endif
