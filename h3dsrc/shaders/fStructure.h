#ifndef __Blot__Image_fsh__
#define __Blot__Image_fsh__

inline std::string Structure_fsh() 
{
	std::string str = 
	"#version 330 core\n"\
	"in vec4 vColor;\n"\
	"in vec2 vTex;\n"\
	"in vec4 vPos;\n"\
	"\n"\
	"uniform sampler2D pic_tex;\n"\
	"\n"\
	"out vec4 fragColor;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"	fragColor = vColor;\n"\
	"\n"\
	"\n"\
	"\n"\
	"}\n";
	return str;
}

inline std::string tStructure_fsh() 
{
	std::string str = 
	"out vec4 vColor;\n"\
	"out vec2 vTex;\n"\
	"out vec4 vPos;\n"\
	"\n"\
	"uniform vec3 light;\n"\
	"\n"\
	"uniform sampler2D pic_tex;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"   vec4 color = texture2D(pic_tex, vTex);\n"\
	"	gl_FragColor = color * vColor;\n"\
	"\n"\
	"\n"\
	"\n"\
	"}\n";
	return str;
}

#endif
