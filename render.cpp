#include "render.h"
#include <glload/gll.hpp>
#include <glload/gl_3_3.h>
#include <glutil/Shader.h>
#include <glm/glm.hpp>
#include <glutil/MatrixStack.h>
#include <glimg/glimg.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <algorithm>
#include <SDL/SDL.h>

glutil::MatrixStack modelViewMatrix;
std::vector<RenderObject> objects;
glm::vec3 camera_pos, look_at, up_dir;

GLuint load_shader(GLenum eShaderType, const std::string &strFilename)
{
	std::ifstream shaderFile(strFilename.c_str());
	std::stringstream shaderData;
	shaderData << shaderFile.rdbuf();
	shaderFile.close();

	try
	{
		return glutil::CompileShader(eShaderType, shaderData.str());
	}
	catch(glutil::ShaderException &e)
	{
		fprintf(stderr, "%s\n", e.what());
		throw;
	}
}

GLuint create_program(const std::vector<GLuint> &shaderList)
{
	try
	{
		return glutil::LinkProgram(shaderList);
	}
	catch(glutil::ShaderException &e)
	{
		fprintf(stderr, "%s\n", e.what());
		throw;
	}

	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
}

float deg_to_rad(float fAngDeg)
{
	const float fDegToRad = 3.14159f * 2.0f / 360.0f;
	return fAngDeg * fDegToRad;
}

float zNear = 1.0f;
float zFar = 1000.0f;

shader_t shader;

GLuint vao, pbo;

const float vertexData[] = {
	1.0f,  1.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 1.0f, 1.0f,
	0.0f,  1.0f, 1.0f, 1.0f,

	1.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f,  1.0f, 1.0f, 1.0f,

	1.0f,  1.0f, 0.0f, 1.0f,
	0.0f,  1.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,

	1.0f, 0.0f, 0.0f, 1.0f,
	0.0f,  1.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f, 1.0f,

	0.0f,  1.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 0.0f, 1.0f,

	0.0f,  1.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
	0.0f,  1.0f, 0.0f, 1.0f,

	1.0f,  1.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 1.0f, 1.0f,

	1.0f,  1.0f, 1.0f, 1.0f,
	1.0f,  1.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,

	1.0f,  1.0f, 0.0f, 1.0f,
	1.0f,  1.0f, 1.0f, 1.0f,
	0.0f,  1.0f, 1.0f, 1.0f,

	1.0f,  1.0f, 0.0f, 1.0f,
	0.0f,  1.0f, 1.0f, 1.0f,
	0.0f,  1.0f, 0.0f, 1.0f,

	1.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 1.0f, 1.0f,

	1.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,


	//Face 0
	469, 163,
	469, 315,
	317, 163,

	469, 315,
	317, 315,
	317, 163,


	//Face 1

	165, 315,
	13, 315,
	13, 163,

	165, 163,
	165, 315,
	13, 163,
	//Face 2
	317, 11,
	317, 163,
	165, 11,

	317, 163,
	165, 163,
	165, 11,

	//Face 3
	317, 315,
 	317, 467,
	165, 315,

 	317, 467,
	165, 467,
	165, 315,

	//Face 4
	317, 163, 
	317, 315,
	165, 163,

	317, 315,
	165, 315,
	165, 163,

	//Face 5
	621, 163,
	621, 315,
	469, 163,

	621, 315,
	469, 315,
	469, 163

};

GLuint texture_unit = 0;
GLuint texture;

void render_init(int w, int h, bool fullscreen) {


	camera_pos = glm::vec3(0,0,-2.0);
	look_at = glm::vec3(0.0, 0.0, 0);
	up_dir = glm::vec3(0.0, 1.0, 0.0);

  	/* create window */
  	SDL_Init(SDL_INIT_VIDEO);
	int flags = SDL_OPENGL | SDL_DOUBLEBUF;
	if ( fullscreen ) flags |= SDL_FULLSCREEN;
	SDL_SetVideoMode(w, h, 0, flags);
	SDL_WM_SetCaption("Game menu","Game menu");

	glload::LoadFunctions();

	std::vector<GLuint> shader_list;
	//Load shader:
	shader_list.push_back(load_shader(GL_VERTEX_SHADER, "simple.vert"));
	shader_list.push_back(load_shader(GL_FRAGMENT_SHADER, "simple.frag"));
	
	shader.program = create_program(shader_list);

	std::for_each(shader_list.begin(), shader_list.end(), glDeleteShader);

	//load texture

	glimg::ImageSet *pImgSet = glimg::loaders::stb::LoadFromFile("texture/cube.jpg");
	texture = glimg::CreateTexture(pImgSet, 0);


	//Init shaders

	shader.mvp = glGetUniformLocation(shader.program, "mvp");
	shader.texSize = glGetUniformLocation(shader.program, "texSize");
	shader.texture = glGetUniformLocation(shader.program, "tex");

	glUseProgram(shader.program);

	glUniform2f(shader.texSize, 640.0f, 482.0f);
	glUniform1i(shader.texture, texture_unit);

	glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, texture);

	glUseProgram(0);

	glGenBuffers(1, &pbo);
	glBindBuffer(GL_ARRAY_BUFFER, pbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* setup opengl */
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glViewport(0, 0, w, h);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_CLAMP);


	modelViewMatrix.Perspective(45.0f, w/(float)h, zNear, zFar);
}

float rotation = 0;

void render(double dt){
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader.program);

	modelViewMatrix.Push();

	modelViewMatrix.LookAt(camera_pos, look_at, up_dir);

	modelViewMatrix.Translate(glm::vec3(0.7, 0, 10.0));

	rotation += 20.0f*dt;

	rotation = fmod(rotation, 360.0f);
	
	modelViewMatrix.RotateY(rotation);
	modelViewMatrix.RotateZ(rotation);
	modelViewMatrix.RotateX(rotation);

	glUniformMatrix4fv(shader.mvp, 1, GL_FALSE, glm::value_ptr(modelViewMatrix.Top()));

	modelViewMatrix.Pop();

	size_t texCoord= sizeof(float) * 4 * 3 * 2 * 6;
	glBindBuffer(GL_ARRAY_BUFFER,pbo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)texCoord);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);


	SDL_GL_SwapBuffers();
}
