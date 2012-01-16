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

float zNear = 1.0f;
float zFar = 1000.0f;

shader_t shader;

GLuint vao;

void render_init(int w, int h, bool fullscreen) {


	camera_pos = glm::vec3(0,0,-10.0);
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
	shader_list.push_back(load_shader(GL_VERTEX_SHADER, "texture.vert"));
	shader_list.push_back(load_shader(GL_FRAGMENT_SHADER, "texture.frag"));
	
	shader.program = create_program(shader_list);

	std::for_each(shader_list.begin(), shader_list.end(), glDeleteShader);


	//Init shaders

	shader.mvp = glGetUniformLocation(shader.program, "mvp");
	shader.texture = glGetUniformLocation(shader.program, "tex");

	shader.diffuse = glGetUniformLocation(shader.program, "diffuse");
	shader.specular = glGetUniformLocation(shader.program, "specular");
	shader.ambient = glGetUniformLocation(shader.program, "ambient");
	shader.emission = glGetUniformLocation(shader.program, "emission");
	shader.shininess = glGetUniformLocation(shader.program, "shininess");

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glUseProgram(shader.program);

	glUniform1i(shader.texture, 0);
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(0);

	/* setup opengl */
	glClearColor(0.2f, 0.1f, 0.2f, 0.0f);
	glViewport(0, 0, w, h);

	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);*/


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

	for(std::vector<RenderObject>::iterator it=objects.begin(); it!=objects.end(); ++it) {
		it->render(dt);
	}

	modelViewMatrix.Pop();

	SDL_GL_SwapBuffers();
}
