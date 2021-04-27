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

#ifndef __abmap__SlipObjFile__
#define __abmap__SlipObjFile__

#include "SlipObject.h"
#include <hcsrc/vec3.h>
#include <h3dsrc/Frameworks.h>


class SlipObjFile : virtual public SlipObject
{
public:
	SlipObjFile(std::string filename, bool unique = false);

	void writeBob(std::string filename);
	void readBob(std::string filename);
private:
	Helen3D::Vertex *addUniqueVertex(std::vector<vec3> &vs, int index);
	void copyUniqueVertex(Helen3D::Vertex *v);
	void readInput();
	std::string _filename;

	bool _unique;
};

#endif
