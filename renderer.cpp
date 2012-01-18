#include "renderer.h"
#include "render_group.h"
#include "render_object.h"
#include "camera.h"
#include "light.h"
#include "shader.h"

#include "skybox.h"

#include <glload/gll.hpp>
#include <glload/gl_3_3.h>
#include <glm/glm.hpp>
#include <glutil/MatrixStack.h>
#include <glimg/glimg.h>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <SDL/SDL.h>
#include <GL/glu.h>

std::string Renderer::shader_files_[] = {
	"normal",
	"light_source",
	"skybox"
};

void Renderer::init_shader(Shader &shader) {

	//Local uniforms
	shader.texture = glGetUniformLocation(shader.program, "tex");
	shader.camera_pos= glGetUniformLocation(shader.program, "camera_pos");

	//Global uniforms
	shader.Matrices = glGetUniformBlockIndex(shader.program, "Matrices");
	shader.LightsData = glGetUniformBlockIndex(shader.program, "LightsData");
	shader.Material = glGetUniformBlockIndex(shader.program, "Material");

	//Bind to blocks
	glUniformBlockBinding(shader.program, shader.Matrices, Shader::MATRICES_BLOCK_INDEX);
	glUniformBlockBinding(shader.program, shader.LightsData, Shader::LIGHTS_DATA_BLOCK_INDEX);
	glUniformBlockBinding(shader.program, shader.Material, Shader::MATERIAL_BLOCK_INDEX);

	//Bind texture
	glUseProgram(shader.program);
	glUniform1i(shader.texture, 0);
	glUseProgram(0);

	checkForGLErrors((std::string("init shader ")+shader.name).c_str());
}

Renderer::Renderer(int w, int h, bool fullscreen) {
	float zNear = 1.0f;
	float zFar = 1000.0f;
	light_attenuation = 1.f/pow(HALF_LIGHT_DISTANCE,2);
	ambient_intensity = glm::vec4(0.1f,0.1f,0.1f,1.0f);

	skybox_loaded_ = false;

	width_ = w;
	height_ = h;

	camera.set_position(glm::vec3(0.0, 0.0, -2.5));

  	/* create window */
  	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	int flags = SDL_OPENGL | SDL_DOUBLEBUF;
	if ( fullscreen ) flags |= SDL_FULLSCREEN;
	SDL_SetVideoMode(w, h, 0, flags);
	SDL_WM_SetCaption("Game menu","Game menu");

	glload::LoadFunctions();

	checkForGLErrors("render init");

	//Load shaders:
	for(int i=0;i<NUM_SHADERS; ++i) {
		shaders[i] = Shader::create_shader(shader_files_[i]);
		init_shader(shaders[i]);
	}

	glActiveTexture(GL_TEXTURE0);

	//Setup uniform buffers
	glGenBuffers(sizeof(Shader::globals_t)/sizeof(GLuint), (GLuint*)&Shader::globals);
	glBindBuffer(GL_UNIFORM_BUFFER, Shader::globals.matricesBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4)*3, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, Shader::globals.lightsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Shader::lights_data_t), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, Shader::globals.materialBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Shader::material_t), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//Bind buffers to blocks:
	glBindBufferRange(GL_UNIFORM_BUFFER, Shader::MATRICES_BLOCK_INDEX, Shader::globals.matricesBuffer, 0, sizeof(glm::mat4)*2);
	glBindBufferRange(GL_UNIFORM_BUFFER, Shader::LIGHTS_DATA_BLOCK_INDEX, Shader::globals.lightsBuffer, 0, sizeof(Shader::lights_data_t));
	glBindBufferRange(GL_UNIFORM_BUFFER, Shader::MATERIAL_BLOCK_INDEX, Shader::globals.materialBuffer, 0, sizeof(Shader::material_t));

	//Generate skybox buffers:

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &skybox_buffer_);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_buffer_);

	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxData), skyboxData, GL_STATIC_DRAW);

	glUseProgram(shaders[NORMAL_SHADER].program);


	glUseProgram(0);

	/* setup opengl */
	glClearColor(0.2f, 0.1f, 0.2f, 0.0f);

	//Setup view (this may be moved to reshape)
	projectionViewMatrix.Perspective(45.0f, w/(float)h, zNear, zFar);
	glViewport(0, 0, w, h);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);


	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_CLAMP);
}

/**
 * skybox_path is the path to the folder with the skybox textures
 */
void Renderer::load_skybox(std::string skybox_path) {
	//Load skybox texture:
	skybox_path+="/";
	for(int i=0; i < 6; ++i) {
		skybox_texture_[i] = glimg::CreateTexture(glimg::loaders::stb::LoadFromFile((skybox_path+skybox_texture_name[i]).c_str()),0);
	}
	skybox_loaded_ = true;
}

Renderer::~Renderer() { }

int Renderer::checkForGLErrors( const char *s )
{
 int errors = 0 ;

 while ( true )
 {
	GLenum x = glGetError() ;

	if ( x == GL_NO_ERROR )
	  return errors ;

	fprintf( stderr, "%s: OpenGL error: %s\n", s, gluErrorString ( x )) ;
	errors++ ;
 }
}

void Renderer::render(double dt){
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	if(skybox_loaded_)
		render_skybox();

	glDisable(GL_CULL_FACE);

	projectionViewMatrix.Push();

	projectionViewMatrix.LookAt(camera.position(), camera.look_at(), camera.up());

	//Upload projection matrix:
	glBindBuffer(GL_UNIFORM_BUFFER, Shader::globals.matricesBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionViewMatrix.Top()));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	for(int i=0; i < NUM_SHADERS; ++i) {
		glUniform3fv(shaders[i].camera_pos,3,  glm::value_ptr(camera.position()));
	}
	//Build lights object:
	if (lights.size() <= MAX_NUM_LIGHTS) {
		lightData.num_lights	= lights.size();
	} else {
		lightData.num_lights = MAX_NUM_LIGHTS;
		fprintf(stderr, "Warning! There are more than %d lights. Only the %d first ligths will be used!\n", MAX_NUM_LIGHTS, MAX_NUM_LIGHTS);
	}
	lightData.attenuation = light_attenuation;
	lightData.ambient_intensity =  ambient_intensity;
	for(unsigned int i=0; i < lightData.num_lights; ++i) {
		lightData.lights[i] = lights[i]->shader_light();
	}
	//Upload light data:
	glBindBuffer(GL_UNIFORM_BUFFER, Shader::globals.lightsBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Shader::lights_data_t), &lightData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


	for(std::vector<RenderGroup*>::iterator it=render_objects.begin(); it!=render_objects.end(); ++it) {
		(*it)->render(dt, this);
	}


	projectionViewMatrix.Pop();

	glUseProgram(0);

	SDL_GL_SwapBuffers();

	checkForGLErrors("render(): ");
}

void Renderer::render_skybox() {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	glUseProgram(shaders[SKYBOX_SHADER].program);

	projectionViewMatrix.Push();
	projectionViewMatrix.SetIdentity();
	projectionViewMatrix.Perspective(45.0f, width_/(float)height_, -0.5, 0.5);
	projectionViewMatrix.LookAt(glm::vec3(0.0), camera.look_at(), glm::vec3(0.0, 1.0, 0.0));

	//Upload projection matrix:
	glBindBuffer(GL_UNIFORM_BUFFER, Shader::globals.matricesBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionViewMatrix.Top()));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, skybox_buffer_);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*3*36) );

	for(int i =0;i<6; ++i) {
		glBindTexture(GL_TEXTURE_2D, skybox_texture_[i]);
		glDrawArrays(GL_TRIANGLES, 6*i, 6);
	}


	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glUseProgram(shaders[NORMAL_SHADER].program);

	projectionViewMatrix.Pop();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(0);
}

void Renderer::upload_model_matrices() {
	glBindBuffer(GL_UNIFORM_BUFFER, Shader::globals.matricesBuffer);
	//Model matrix:
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(modelMatrix.Top()));
	//Normal matrix:
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4)*2, sizeof(glm::mat4), glm::value_ptr(glm::transpose(glm::inverse(modelMatrix.Top()))));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
