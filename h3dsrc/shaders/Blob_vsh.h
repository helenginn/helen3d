#ifndef __vagabond_Blob_vsh__
#define __vagabond_Blob_vsh__

inline std::string Blob_vsh()
{
	std::string str =
	"#version 330\n"\
	"in vec3 normal;\n"\
	"in vec3 position;\n"\
	"in vec4 color;\n"\
	"in vec2 tex;\n"\
	"\n"\
	"out vec4 vColor;\n"\
	"out vec2 vTex;\n"\
	"out vec4 vPos;\n"\
	"\n"\
	"uniform mat4 projection;\n"\
	"uniform mat4 model;\n"\
	"uniform vec3 light_pos;\n"\
	"uniform vec3 focus;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"   vec4 pos = vec4(position[0], position[1], position[2], 1.0);\n"\
	"   gl_Position = projection * model * pos;\n"\
	"   vec4 model4 = model * pos;\n"\
	"	vPos = model4;\n"\
	"	gl_PointSize = -800. / model4[2];\n"\
	"	gl_PointSize *= normal[0];\n"\
	"	vColor = color;\n"\
	"}";

	return str;
}


#endif

