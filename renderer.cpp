#include "renderer.h"
#include "render_group.h"
#include "render_object.h"
#include "camera.h"
#include "light.h"

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
#include <GL/glu.h>

std::string Renderer::shader_files[] = {
	"normal",
	"light_source"
};



GLuint Renderer::load_shader(GLenum eShaderType, const std::string &strFilename)
{
	std::ifstream shaderFile(strFilename.c_str());
	std::stringstream shaderData;
	shaderData << shaderFile.rdbuf();
	shaderFile.close();
	printf("Compiling shader %s\n", strFilename.c_str());
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

GLuint Renderer::create_program(const std::vector<GLuint> &shaderList)
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

Renderer::shader_t Renderer::load_shader_program(std::string file) {
	shader_t shader;

	std::vector<GLuint> shader_list;
	//Load shaders:
	shader_list.push_back(load_shader(GL_VERTEX_SHADER, "shaders/"+file+".vert"));
	shader_list.push_back(load_shader(GL_FRAGMENT_SHADER, "shaders/"+file+".frag"));
	
	shader.program = create_program(shader_list);

	std::for_each(shader_list.begin(), shader_list.end(), glDeleteShader);


	//Init shaders

	//Local uniforms
	shader.texture = glGetUniformLocation(shader.program, "tex");
	shader.camera_pos= glGetUniformLocation(shader.program, "camera_pos");

	//Global uniforms
	shader.Matrices = glGetUniformBlockIndex(shader.program, "Matrices");
	shader.LightsData = glGetUniformBlockIndex(shader.program, "LightsData");
	shader.Material = glGetUniformBlockIndex(shader.program, "Material");

	//Bind to blocks
	glUniformBlockBinding(shader.program, shader.Matrices, MATRICES_BLOCK_INDEX);
	glUniformBlockBinding(shader.program, shader.LightsData, LIGHTS_DATA_BLOCK_INDEX);
	glUniformBlockBinding(shader.program, shader.Material, MATERIAL_BLOCK_INDEX);

	return shader;
}

Renderer::Renderer(int w, int h, bool fullscreen) {
	float zNear = 1.0f;
	float zFar = 1000.0f;
	light_attenuation = 1.f/pow(HALF_LIGHT_DISTANCE,2);
	ambient_intensity = glm::vec4(0.1f,0.1f,0.1f,1.0f);

	camera.set_position(glm::vec3(0.0, 0.0, -2.5));

  	/* create window */
  	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	int flags = SDL_OPENGL | SDL_DOUBLEBUF;
	if ( fullscreen ) flags |= SDL_FULLSCREEN;
	SDL_SetVideoMode(w, h, 0, flags);
	SDL_WM_SetCaption("Game menu","Game menu");

	glload::LoadFunctions();

	//Load shaders:
	for(int i=0;i<NUM_SHADERS; ++i) {
		shaders[i] = load_shader_program(shader_files[i]);
	}

	//Setup uniform buffers
	glGenBuffers(sizeof(shader_globals_t)/sizeof(GLuint), (GLuint*)&shader_globals);
	glBindBuffer(GL_UNIFORM_BUFFER, shader_globals.matricesBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4)*3, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, shader_globals.lightsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(shader_lights_t), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, shader_globals.materialBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(shader_material_t), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//Bind buffers to blocks:
	glBindBufferRange(GL_UNIFORM_BUFFER, MATRICES_BLOCK_INDEX, shader_globals.matricesBuffer, 0, sizeof(glm::mat4)*2);
	glBindBufferRange(GL_UNIFORM_BUFFER, LIGHTS_DATA_BLOCK_INDEX, shader_globals.lightsBuffer, 0, sizeof(shader_lights_t));
	glBindBufferRange(GL_UNIFORM_BUFFER, MATERIAL_BLOCK_INDEX, shader_globals.materialBuffer, 0, sizeof(shader_material_t));

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glUseProgram(shaders[NORMAL_SHADER].program);

	glUniform1i(shaders[NORMAL_SHADER].texture, 0);
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(0);

	/* setup opengl */
	glClearColor(0.2f, 0.1f, 0.2f, 0.0f);

	//Setup view (this may be moved to reshape)
	projectionViewMatrix.Perspective(45.0f, w/(float)h, zNear, zFar);
	glViewport(0, 0, w, h);

	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);*/


	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_CLAMP);
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

	glUseProgram(shaders[NORMAL_SHADER].program);

	projectionViewMatrix.Push();

	projectionViewMatrix.LookAt(camera.position(), camera.look_at(), camera.up());

	//Upload projection matrix:
	glBindBuffer(GL_UNIFORM_BUFFER, shader_globals.matricesBuffer);
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
	glBindBuffer(GL_UNIFORM_BUFFER, shader_globals.lightsBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(shader_lights_t), &lightData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	modelMatrix.Push();

	for(std::vector<RenderGroup*>::iterator it=render_objects.begin(); it!=render_objects.end(); ++it) {
		(*it)->render(dt, this);
	}

	modelMatrix.Pop();
	projectionViewMatrix.Pop();

	SDL_GL_SwapBuffers();

	checkForGLErrors("render(): ");
}


void Renderer::upload_model_matrices() {
	glBindBuffer(GL_UNIFORM_BUFFER, shader_globals.matricesBuffer);
	//Model matrix:
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(modelMatrix.Top()));
	//Normal matrix:
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4)*2, sizeof(glm::mat4), glm::value_ptr(glm::transpose(glm::inverse(modelMatrix.Top()))));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
