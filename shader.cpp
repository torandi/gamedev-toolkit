#include "shader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include <glutil/Shader.h>
#include <glload/gll.hpp>
#include <glload/gl_3_3.h>

Shader::globals_t Shader::globals;

GLuint Shader::load_shader(GLenum eShaderType, const std::string &strFilename) {
	std::ifstream shaderFile(strFilename.c_str());
	std::stringstream shaderData;
	shaderData << shaderFile.rdbuf();
	shaderFile.close();
	printf("Compiling shader %s\n", strFilename.c_str());
	try {
		return glutil::CompileShader(eShaderType, shaderData.str());
	} catch(glutil::ShaderException &e) {
		fprintf(stderr, "%s\n", e.what());
		throw;
	}
}

GLuint Shader::create_program(const std::vector<GLuint> &shaderList) {
	try {
		return glutil::LinkProgram(shaderList);
	} catch(glutil::ShaderException &e) {
		fprintf(stderr, "%s\n", e.what());
		throw;
	}

	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
}

Shader Shader::create_shader(std::string base_name) {
	Shader shader;
	shader.name = base_name;

	std::vector<GLuint> shader_list;
	//Load shaders:
	shader_list.push_back(load_shader(GL_VERTEX_SHADER, SHADER_PATH+base_name+VERT_SHADER_EXTENTION));
	shader_list.push_back(load_shader(GL_FRAGMENT_SHADER, SHADER_PATH+base_name+FRAG_SHADER_EXTENTION));
	
	shader.program = create_program(shader_list);

	std::for_each(shader_list.begin(), shader_list.end(), glDeleteShader);

	return shader;
}
