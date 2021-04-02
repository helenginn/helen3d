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

#include "Quad.h"
#include "SlipGL.h"
#include "shaders/vStructure.h"
#include "shaders/fStructure.h"
#include <iostream>

Quad::Quad() : SlipObject()
{
	makeQuad();

	_handleOwnTextures = true;
	_mode = 0;
	_threshold = 0;
	_other = 0;
	_vString = Structure_vsh();
	_fString = Structure_fsh();
	setName("Quad");
}

void Quad::makeQuad()
{
	addVertex(-1, -1, 0);
	addVertex(-1, +1, 0);
	addVertex(+1, -1, 0);
	addVertex(+1, +1, 0);

	addIndices(0, 1, 2);
	addIndices(1, 2, 3);
}

void Quad::prepareNormalDist()
{
	_count = 5;
	float sigma = _count / 2.;

	for (size_t i = 0; i < _count; i++)
	{
		float isq = i * i;
		_dist[i] = 1/sqrt(sigma*2*M_PI)*exp(-isq/(2 * sigma));
	}
}

void Quad::prepareTextures(SlipGL *sender)
{
	if (_textures.size() == 0)
	{
		_textures.resize(sender->sceneTextureCount() + 1);
		
		_textures[0] = sender->sceneDepth();
		
		for (size_t i = 0; i < sender->sceneTextureCount(); i++)
		{
			_textures[i + 1] = sender->sceneTexture(i);
		}
	}
	
	prepareNormalDist();
}

void Quad::render(SlipGL *sender)
{
	SlipObject::render(sender);
}

void Quad::extraUniforms()
{
	{
		const char *uniform_name = "horizontal";
		GLuint pos = glGetUniformLocation(_program, uniform_name);
		glUniform1i(pos, _mode);
	}

	{
		const char *uniform_name = "mode";
		GLuint pos = glGetUniformLocation(_program, uniform_name);
		glUniform1i(pos, _mode);
	}

	{
		const char *uniform_name = "threshold";
		GLuint pos = glGetUniformLocation(_program, uniform_name);
		glUniform1f(pos, _threshold);
	}

	{
		const char *uniform_name = "other";
		GLuint pos = glGetUniformLocation(_program, uniform_name);
		glUniform1f(pos, _other);
	}

	{
		/*
		const char *uniform_name = "weight";
		GLuint pos = glGetUniformLocation(_program, uniform_name);
		glUniform1fv(pos, _count, _dist);
		*/
	}

	checkErrors("single num uniforms");

	GLint uTex = glGetUniformLocation(_program, "pic_tex");
	if (uTex >= 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _textures[0]);
		glUniform1i(uTex, 0);
	}

	uTex = glGetUniformLocation(_program, "bright_tex");
	if (uTex >= 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _textures[1]);
		glUniform1i(uTex, 1);
	}

	checkErrors("uniform add texture");
}

void Quad::bindTextures()
{
}
