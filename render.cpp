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

#define RENDER_LIGHT 1

#define HALF_LIGHT_DISTANCE 20.f

RenderObject *light;

glutil::MatrixStack modelViewMatrix;
glutil::MatrixStack projectionMatrix;
std::vector<RenderObject> objects;
glm::vec3 camera_pos, look_at, up_dir, light_pos;
float light_attenuation;
glm::vec4 light_intensity, ambient_intensity;

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
	light_pos = glm::vec3(2.0, 2.0, 2.0);

	light_attenuation = 1.f/pow(HALF_LIGHT_DISTANCE,2);
	light_intensity = glm::vec4(0.8f,0.8f, 0.8f, 1.0f);
	ambient_intensity = glm::vec4(0.2f,0.2f,0.2f,1.0f);

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
	shader.projection_matrix = glGetUniformLocation(shader.program, "projection_matrix");
	shader.texture = glGetUniformLocation(shader.program, "tex");
	shader.camera_pos= glGetUniformLocation(shader.program, "camera_pos");

	shader.light_pos= glGetUniformLocation(shader.program, "light_pos");
	shader.light_attenuation= glGetUniformLocation(shader.program, "light_attenuation");
	shader.light_intensity= glGetUniformLocation(shader.program, "light_intensity");
	shader.ambient_intensity= glGetUniformLocation(shader.program, "ambient_intensity");

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


	projectionMatrix.Perspective(45.0f, w/(float)h, zNear, zFar);

	if(RENDER_LIGHT) {
		light = new RenderObject("models/cube.obj");	
		light->scale = 0.25f/2.0f;
	}
}

float rotation = 0;

void render(double dt){
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader.program);

	projectionMatrix.Push();

	projectionMatrix.LookAt(camera_pos, look_at, up_dir);

	glUniformMatrix4fv(shader.projection_matrix, 1, GL_FALSE,  glm::value_ptr(projectionMatrix.Top()));
	glUniform3fv(shader.camera_pos,3,  glm::value_ptr(camera_pos));

	glUniform3fv(shader.light_pos, 3, glm::value_ptr(light_pos));
	glUniform1f(shader.light_attenuation, light_attenuation);
	glUniform4fv(shader.light_intensity, 4,  glm::value_ptr(light_intensity));
	glUniform4fv(shader.ambient_intensity,4,  glm::value_ptr(ambient_intensity));

	for(std::vector<RenderObject>::iterator it=objects.begin(); it!=objects.end(); ++it) {
		it->render(dt);
	}
	if(RENDER_LIGHT) {
		light->position = light_pos;
		light->render(dt);
	}

	projectionMatrix.Pop();

	SDL_GL_SwapBuffers();
}
