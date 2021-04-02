#ifndef __snow__shadow__
#define __snow__shadow__

inline std::string vShadow()
{
	std::string str = 
	"attribute vec3 normal;\n"\
	"attribute vec3 position;\n"\
	"attribute vec4 color;\n"\
	"attribute vec4 extra;\n"\
	"attribute vec2 tex;\n"\
	"\n"\
	"uniform mat4 model;\n"\
	"uniform mat4 projection;\n"\
	"uniform float time;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"    vec4 pos = vec4(position[0], position[1], position[2], 1.0);\n"\
	"    gl_Position = projection * model * pos;\n"\
	"}";
	return str;
}

inline std::string fShadow() 
{
	std::string str = 
	"\n"\
	"void main()\n"\
	"{\n"\
	"	\n"\
	"\n"\
	"\n"\
	"\n"\
	"}\n";
	return str;
}

#endif
