#include "render.h"
#include "render_object.h"
#include "camera.h"

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

#define RENDER_LIGHT 0

#define HALF_LIGHT_DISTANCE 10.f

int errcnt = 0;

RenderObject *light;

glutil::MatrixStack modelViewMatrix;
glutil::MatrixStack projectionMatrix;
std::vector<RenderObject> objects;
glm::vec3 camera_pos, look_at, up_dir;
float light_attenuation;
glm::vec4 ambient_intensity;
std::vector<light_t> lights;
shader_globals_t sg;

lights_t lightData;

camera_t camera;

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

	light_attenuation = 1.f/pow(HALF_LIGHT_DISTANCE,2);
	ambient_intensity = glm::vec4(0.1f,0.1f,0.1f,1.0f);

	light_t l;

	l.intensity = glm::vec4(0.5f,0.5f, 0.5f, 1.0f);
	l.position = glm::vec4(2.0, 2.0, 2.0, 0.0);
	
	lights.push_back(l);


	projectionMatrix.Perspective(45.0f, w/(float)h, zNear, zFar);

  	/* create window */
  	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	int flags = SDL_OPENGL | SDL_DOUBLEBUF;
	if ( fullscreen ) flags |= SDL_FULLSCREEN;
	SDL_SetVideoMode(w, h, 0, flags);
	SDL_WM_SetCaption("Game menu","Game menu");

	glload::LoadFunctions();

	std::vector<GLuint> shader_list;
	//Load shader:
	shader_list.push_back(load_shader(GL_VERTEX_SHADER, "shaders/normal.vert"));
	shader_list.push_back(load_shader(GL_FRAGMENT_SHADER, "shaders/normal.frag"));
	
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

	//Setup uniform buffers
	glGenBuffers(sizeof(shader_globals_t)/sizeof(GLuint), (GLuint*)&sg);
	glBindBuffer(GL_UNIFORM_BUFFER, sg.matricesBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4)*2, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, sg.lightsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(lights_t), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, sg.materialBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(shader_material_t), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//Bind buffers to blocks:
	glBindBufferRange(GL_UNIFORM_BUFFER, MATRICES_BLOCK_INDEX, sg.matricesBuffer, 0, sizeof(glm::mat4)*2);
	glBindBufferRange(GL_UNIFORM_BUFFER, LIGHTS_DATA_BLOCK_INDEX, sg.lightsBuffer, 0, sizeof(lights_t));
	glBindBufferRange(GL_UNIFORM_BUFFER, MATERIAL_BLOCK_INDEX, sg.materialBuffer, 0, sizeof(shader_material_t));

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glUseProgram(shader.program);

	glUniform1i(shader.texture, 0);
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(0);

	/* setup opengl */
	glClearColor(0.2f, 0.1f, 0.2f, 0.0f);

	//Setup view (this may be moved to reshape
	glViewport(0, 0, w, h);

	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);*/


	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_CLAMP);

	light = new RenderObject("models/cube.obj");
	light->scale = 0.25f/2.0f;
}

float rotation = 0;

int checkForGLErrors( const char *s )
{
 int errors = 0 ;

 while ( true )
 {
	GLenum x = glGetError() ;

	if ( x == GL_NO_ERROR )
	  return errors ;

	fprintf( stderr, "%s: OpenGL error: %s [%08x]\n", s ? s : "", gluErrorString ( x ), errcnt++ ) ;
	errors++ ;
 }
}

void render(double dt){
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader.program);

	projectionMatrix.Push();

	projectionMatrix.LookAt(camera_pos, look_at, up_dir);

	//Upload projection matrix:
	glBindBuffer(GL_UNIFORM_BUFFER, sg.matricesBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionMatrix.Top()));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glUniform3fv(shader.camera_pos,3,  glm::value_ptr(camera_pos));

	//Build lights object:
	assert(lights.size() <= MAX_NUM_LIGHTS);
	lightData.num_lights	= lights.size();
	lightData.attenuation = light_attenuation;
	lightData.ambient_intensity =  ambient_intensity;
	memcpy(lightData.lights, &lights.front(), lightData.num_lights*sizeof(light_t));
	//Upload light data:
	glBindBuffer(GL_UNIFORM_BUFFER, sg.lightsBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lights_t), &lightData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	modelViewMatrix.Push();

	modelViewMatrix*= camera.matrix();

	for(std::vector<RenderObject>::iterator it=objects.begin(); it!=objects.end(); ++it) {
		it->render(dt);
	}


	light->position = glm::vec3(lights.front().position);
	light->render(dt);

	modelViewMatrix.Pop();
	projectionMatrix.Pop();

	SDL_GL_SwapBuffers();

	checkForGLErrors("Render(): ");
}
