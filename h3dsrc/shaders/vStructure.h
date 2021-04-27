#ifndef __Blot__Image_vsh__
#define __Blot__Image_vsh__

inline std::string Structure_vsh()
{
	std::string str = 
	"#version 330 core\n"\
	"in vec3 normal;\n"\
	"in vec3 position;\n"\
	"in vec4 color;\n"\
	"in vec4 extra;\n"\
	"in vec2 tex;\n"\
	"\n"\
	"uniform mat4 model;\n"\
	"uniform mat4 projection;\n"\
	"uniform float time;\n"\
	"\n"\
	"out vec4 vColor;\n"\
	"out vec4 vPos;\n"\
	"out vec2 vTex;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"    vec4 pos = vec4(position[0], position[1], position[2], 1.0);\n"\
	"	 vec4 norm4 = vec4(normal[0], normal[1], normal[2], 1.0);\n"\
	"	 pos = model * pos;\n"\
	"	 vec4 lightpos = vec4(pos[0], pos[1], pos[2], 1);\n"\
	"    float mag = abs(dot(normalize(norm4), normalize(lightpos)));"\
	"    float red   = color[0] + 0.4 * mag;\n"
	"    float green = color[1] + 0.4 * mag;\n"
	"    float blue  = color[2] + 0.4 * mag;\n"
	"    red   = min(red, 1.);\n"\
	"    green = min(green, 1.);\n"\
	"    blue  = min(blue, 1.);\n"\
	"    vPos = projection * pos;\n"\
	"    gl_Position = vPos;\n"\
	"	 vColor = vec4(red, green, blue, color[3]);\n"\
	"    vTex = tex;\n"\
	"}";
	return str;
}

#endif
