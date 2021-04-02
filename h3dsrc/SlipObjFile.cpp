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

#include "Frameworks.h"
using namespace Helen3D;

#include "SlipObjFile.h"
#include "FileReader.h"
#include <fstream>

SlipObjFile::SlipObjFile(std::string filename, bool unique) : SlipObject()
{
	_unique = unique;
	_filename = filename;
	readInput();
	
	setName(getBaseFilename(filename));
}

void SlipObjFile::copyUniqueVertex(Helen3D::Vertex *v)
{
	addIndex(vertexCount());
	_vertices.push_back(*v);
}

Vertex *SlipObjFile::addUniqueVertex(std::vector<vec3> &vs, int index)
{
	if (!_unique)
	{
		addIndex(index);
		return &_vertices[index];
	}

	addIndex(vertexCount());
	addVertex(vs[index].x, vs[index].y, vs[index].z);
	
	return &_vertices[_vertices.size() - 1];
}

void SlipObjFile::readInput()
{
	if (_filename.length() == 0)
	{
		return;
	}
	
	std::string ext;
	try
	{
		ext = _filename.substr(_filename.rfind('.'));
	}
	catch (const std::out_of_range &error)
	{

	}

	if (ext.substr(0, 4) == ".bob")
	{
		readBob(_filename);
		return;
	}

	std::vector<vec3> normals;
	std::vector<vec3> vertices;
	std::string contents = get_file_contents(_filename);
	std::vector<std::string> lines = split(contents, '\n');
	
	std::vector<vec3> textures;

	for (size_t i = 0; i < lines.size(); i++)
	{
		std::vector<std::string> bits = split(lines[i], ' ');

		if (bits.size() < 1)
		{
			continue;
		}

		if (bits[0] == "v")
		{
			if (bits.size() < 4)
			{
				std::cout << "Warning: cannot interpret "\
				"line: " << std::endl;
				std::cout << "\t" << lines[i] << std::endl;
				continue;
			}

			float v1 = atof(bits[1].c_str());
			float v2 = atof(bits[2].c_str());
			float v3 = atof(bits[3].c_str());

			vertices.push_back(make_vec3(v1, v2, v3));
			
			if (!_unique)
			{
				addVertex(v1, v2, v3);
			}
		}

		if (bits[0] == "vt")
		{
			if (bits.size() < 2)
			{
				std::cout << "Warning: cannot interpret "\
				"line: " << std::endl;
				std::cout << "\t" << lines[i] << std::endl;
				continue;
			}

			vec3 tex;
			tex.x = atof(bits[1].c_str());
			tex.y = atof(bits[2].c_str());
			tex.z = 0;

			textures.push_back(tex);
		}

		if (bits[0] == "vn")
		{
			if (bits.size() < 4)
			{
				std::cout << "Warning: cannot interpret "\
				"line: " << std::endl;
				std::cout << "\t" << lines[i] << std::endl;
				continue;
			}

			vec3 norm;
			norm.x = atof(bits[1].c_str());
			norm.y = atof(bits[2].c_str());
			norm.z = atof(bits[3].c_str());

			normals.push_back(norm);
		}

		if (bits[0] == "f")
		{
			if (bits.size() <= 3)
			{
				std::cout << "Warning: cannot interpret "\
				"line: " << std::endl;
				std::cout << "\t" << lines[i] << std::endl;
				std::cout << "\t(can only deal with >3 numbers)" << std::endl;
				continue;
			}

			GLuint first = 0;
			GLuint prev = 0;

			Vertex *vFirst = NULL;
			Vertex *vPrev = NULL;
			Vertex *last = NULL;

			for (size_t j = 1; j < bits.size(); j++)
			{
				std::vector<std::string> components = split(bits[j], '/');
				GLuint index = 0;
				
				if (components.size() > 0)
				{
					index = atoi(components[0].c_str()) - 1;
					if (j == 1)
					{
						first = index;
					}
					
					if (j > 3) /* fan, re-add previous indices */
					{
						if (!_unique)
						{
							addUniqueVertex(vertices, first);
							addUniqueVertex(vertices, prev);
						}
						else
						{
							copyUniqueVertex(vFirst);
							copyUniqueVertex(vPrev);
						}
					}
					
					last = addUniqueVertex(vertices,index);

					if (j == 1)
					{
						vFirst = last;
					}
				}
				
				if (last && components.size() > 1)
				{
					if (components[1].length() > 0)
					{
						int tindex = atoi(components[1].c_str()) - 1;
						last->tex[0] = textures[tindex].x;
						last->tex[1] = textures[tindex].y;
					}
				}
				
				if (last && components.size() > 2)
				{
					GLuint nindex = atoi(components[2].c_str()) - 1;
					if (normals.size() <= nindex)
					{
						std::cout << "Warning: out of bounds normal: " <<
						std::endl;
						std::cout << "\t" << lines[i] << std::endl;
					}

					vec3 norm = normals[nindex];
					last->normal[0] = norm.x;
					last->normal[1] = norm.y;
					last->normal[2] = norm.z;
				}

				vPrev = last;
			}
		}
	}
}


void SlipObjFile::writeBob(std::string filename)
{
	std::ofstream file;
	file.open(filename);

	size_t vs = vertexCount();
	file.write(reinterpret_cast<const char *>(&vs), sizeof(size_t));
	char *buffer = (char *)&_vertices[0];
	file.write(buffer, vSize());

	size_t is = indexCount();
	file.write(reinterpret_cast<const char *>(&is), sizeof(size_t));
	buffer = (char *)&_indices[0];
	file.write(buffer, iSize());
	
	file.close();
}

void SlipObjFile::readBob(std::string filename)
{
	std::ifstream file;
	file.open(filename);

	size_t vs;
	file.read(reinterpret_cast<char *>(&vs), sizeof(size_t));
	_vertices.resize(vs);
	char *buffer = (char *)&_vertices[0];
	file.read(buffer, sizeof(Vertex) * vs);

	size_t is;
	file.read(reinterpret_cast<char *>(&is), sizeof(size_t));
	_indices.resize(is);
	buffer = (char *)&_indices[0];
	file.read(buffer, sizeof(GLuint) * is);
	
	file.close();
}

