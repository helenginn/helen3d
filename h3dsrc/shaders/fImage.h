#ifndef __Blot__Image_fsh__
#define __Blot__Image_fsh__

inline std::string Pencil_fsh() 
{
	std::string str = 
	"#version 330 core\n"\
	"in vec4 vColor;\n"\
	"in vec2 vTex;\n"\
	"in vec4 vPos;\n"\
	"in float vTime;\n"\
	"\n"\
	"out vec4 gl_FragColor;\n"\
	"\n"\
	"uniform sampler2D pic_tex;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"	gl_FragColor = vColor;\n"\
	"\n"\
	"\n"\
	"\n"\
	"}\n";
	return str;
}


#endif
