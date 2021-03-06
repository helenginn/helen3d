#ifndef __vagabond_Blob_fsh__
#define __vagabond_Blob_fsh__

inline std::string Blob_fsh()
{
	std::string str =
	"#version 330 core\n"\
	"in vec4 vColor;\n"\
	"in vec2 vTex;\n"\
	"in vec4 vPos;\n"\
	"\n"\
	"uniform vec3 light_pos;\n"\
	"uniform vec3 focus;\n"\
	"\n"\
	"uniform sampler2D blobTexture;\n"\
	"\n"\
	"out vec4 fragColor;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"	vec2 frag = gl_PointCoord;\n"\
	"	vec2 xy = vec2(frag[0], frag[1]);\n"\
	"	vec4 temp = texture2D(blobTexture, xy);\n"\
	"	if (temp[3] < 0.05) {\n"\
	"		discard;\n"\
	"	}\n"\
	"	float min_distance = -20.;\n"\
	"	float max_distance = -100.;\n"\
	"	if (focus[2] > -25.)\n"\
	"	{\n"\
	"		min_distance = focus[2] + 0.;\n"\
	"		max_distance = focus[2] - 8.;\n"\
	"		if (vPos[2] > -2.) {\n"\
	"			discard;\n"\
	"		}\n"\
	"	}\n"\
	"   float transparency = (vPos[2] - min_distance) / (max_distance - min_distance);\n"\
	"	transparency = max(transparency, 0.0);\n"\
	"	transparency = min(transparency, 1.0);\n"\
	"	fragColor = temp * vColor;\n"\
	"   fragColor[3] = 1. - transparency;\n"\
	"\n"\
	"\n"\
	"\n"\
	"}\n";

	return str;
}


#endif

