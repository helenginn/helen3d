#ifndef __helen3d__shtext__
#define __helen3d__shtext__

inline std::string vText()
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
	"\n"\
	"out vec4 vColor;\n"\
	"out vec3 vOffset;\n"\
	"out vec4 vPos;\n"\
	"out vec2 vTex;\n"\
	"out vec2 dims;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"    vec4 pos = vec4(position[0], position[1], position[2], 1.0);\n"\
	"    vPos = model * pos;\n"\
	"    gl_Position = vPos;\n"\
	"	 vColor = color;\n"\
	"	 vOffset = normal;\n"\
	"    dims = vec2(extra[0], extra[1]);\n"\
	"}";
	return str;
}

inline std::string gText()
{
	std::string str = 
	"#version 330 core\n"\
	"layout (points) in;\n"\
	"layout (triangle_strip, max_vertices = 6) out;\n"\
	"\n"\
	"in vec4 vColor[];\n"\
	"in vec4 vPos[];\n"\
	"in vec3 vOffset[];\n"\
	"in vec2 dims[];\n"\
	"\n"\
	"uniform mat4 projection;\n"\
	"\n"\
	"out vec2 fTex;\n"\
	"out vec4 fColor;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"	 vec4 gl0 = gl_in[0].gl_Position;\n"\
	"	 float width = dims[0][0];\n"\
	"	 float height = dims[0][1];\n"\
	"	 float x = vOffset[0][0];\n"\
	"	 float y = vOffset[0][1];\n"\
	"    float depth = vOffset[0][2];\n"\
	"	 fColor = vColor[0];\n"\
	"\n"\
	"	 fTex = vec2(0., 1.);\n"\
	"    vec4 pos = gl0 + vec4(-width+x, -height+y, depth, 0.0); \n"\
	"    gl_Position = projection * pos;\n"\
    "    EmitVertex();\n"\
	"	 fTex = vec2(1., 1.);\n"\
	"    pos = gl0 + vec4(+width+x, -height+y, depth, 0.0); \n"\
	"    gl_Position = projection * pos;\n"\
    "    EmitVertex();\n"\
	"	 fTex = vec2(0., 0.);\n"\
	"    pos = gl0 + vec4(-width+x, +height+y, depth, 0.0); \n"\
	"    gl_Position = projection * pos;\n"\
    "    EmitVertex();\n"\
	"	 fTex = vec2(1., 1.);\n"\
	"    pos = gl0 + vec4(+width+x, -height+y, depth, 0.0); \n"\
	"    gl_Position = projection * pos;\n"\
    "    EmitVertex();\n"\
	"	 fTex = vec2(0., 0.);\n"\
	"    pos = gl0 + vec4(-width+x, +height+y, depth, 0.0); \n"\
	"    gl_Position = projection * pos;\n"\
    "    EmitVertex();\n"\
	"	 fTex = vec2(1., 0.);\n"\
	"    pos = gl0 + vec4(+width+x, +height+y, depth, 0.0); \n"\
	"    gl_Position = projection * pos;\n"\
    "    EmitVertex();\n"\
	"    \n"\
	"}";
	return str;
}

inline std::string fText()
{
	std::string str = 
	"#version 330 core\n"\
	"in vec4 fColor;\n"\
	"in vec2 fTex;\n"\
	"in vec4 vPos;\n"\
	"\n"\
	"uniform sampler2D text;\n"\
	"\n"\
	"out vec4 FragColor;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"    vec4 color = texture(text, fTex);\n"\
	"    color[3] = 1. - (color[0] + color[1] + color[2]) / 3.;\n"\
	"	 if (color[3] < 0.2) {\n"\
	"	 discard;\n"\
	"	 }\n"\
	"    FragColor = color;\n"\
	"}";
	return str;
}

#endif

