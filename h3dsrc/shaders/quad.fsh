#ifndef __vagabond_Quad_fsh__
#define __vagabond_Quad_fsh__

inline std::string Quad_fsh()
{
	std::string str = 
		"#version 330 core\n"\
		"\n"\
		"in vec4 vPos;\n"\
		"in vec2 vTex;\n"\
		"\n"\
		"uniform sampler2D pic_tex;\n"\
		"uniform sampler2D bright_tex;\n"\
		"\n"\
		"uniform int mode;\n"\
		"uniform float threshold;\n"\
		"\n"\
		"uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216,\n"\
		"0.054054, 0.016216);\n"\
		"\n"\
		"out vec4 fragColor;\n"\
		"\n"\
		"void main()\n"\
		"{\n"\
		"	if (mode == 2)\n"\
		"	{\n"\
		"		vec3 result = texture(pic_tex, vTex).rgb;\n"\
		"		result += texture(bright_tex, vTex).rgb;\n"\
		"\n"\
		"		const float exposure = 1.2;\n"\
		"		const float gamma = 1.5;\n"\
		"		// exposure tone mapping\n"\
		"		vec3 mapped = vec3(1.0) - exp(-result * result * exposure);\n"\
		"\n"\
		"		// gamma correction \n"\
		"		mapped = pow(mapped, vec3(1.0 / gamma));\n"\
		"		fragColor = vec4(mapped, 1.0);\n"\
		"	}\n"\
		"	else if (mode <= 1)\n"\
		"	{\n"\
		"		vec3 result = vec3(0, 0, 0);\n"\
		"		ivec2 tex = ivec2(textureSize(bright_tex, 0) * vTex);\n"\
		"\n"\
		"		for (int i = -4; i < 5; i++)\n"\
		"		{\n"\
		"			ivec2 off = ivec2(mode * i, (1 - mode) * i);\n"\
		"			vec3 pix = texelFetch(bright_tex, tex + off, 0).rgb;\n"\
		"			pix *= weight[abs(i)];\n"\
		"\n"\
		"			result += pix;\n"\
		"		}\n"\
		"\n"\
		"		fragColor = vec4(result, 1.0);\n"\
		"	}\n"\
		"}\n";
	return str;
}

#endif

