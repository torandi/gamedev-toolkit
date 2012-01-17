#ifndef RENDERER_H
#define RENDERER_H
	#include <vector>
	#include <string>
	#include <glload/gl_3_3.h>
	#include <glutil/MatrixStack.h>

	#include "camera.h"
	#include "light.h"
	#include "render_group.h"

	#define MAX_NUM_LIGHTS 4

class Renderer {
public:
	struct shader_t {
		GLuint program;

		GLuint Matrices;
		GLuint camera_pos;
		GLuint LightsData;
		GLuint Material;
		GLuint texture;
	};

	struct shader_lights_t {
		unsigned int num_lights;
		float attenuation;
		float padding[2];
		glm::vec4 ambient_intensity;
		Light::shader_light_t lights[MAX_NUM_LIGHTS];
	};

private:

	static std::string shader_files[];

	int checkForGLErrors( const char *s );
	GLuint vao;
	GLuint load_shader(GLenum eShaderType, const std::string &strFilename);
	GLuint create_program(const std::vector<GLuint> &shaderList);
	shader_t load_shader_program(std::string file);

	shader_lights_t lightData;
public:

	Renderer(int w, int h, bool fullscreen);
	~Renderer();
	
	float zNear;
	float zFar;

	enum shader_program_t {
		NORMAL_SHADER=0,
		LIGHT_SOURCE_SHADER=1,
		NUM_SHADERS=2
	};

	struct shader_globals_t {
		GLuint matricesBuffer;
		GLuint lightsBuffer;
		GLuint materialBuffer;
	};

	shader_t shaders[NUM_SHADERS];

	glutil::MatrixStack modelViewMatrix;
	glutil::MatrixStack projectionMatrix;
	float light_attenuation;
	glm::vec4 ambient_intensity;
	Camera camera;
	shader_globals_t shader_globals;
	std::vector<RenderGroup*> render_objects;
	std::vector<Light*> lights;

	struct shader_material_t {
		unsigned int useTexture; //Set to 1 to use texture
		float shininess;
		float p2[2]; //Padding
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 ambient;
		glm::vec4 emission;
	};


	enum {
		MATRICES_BLOCK_INDEX = 0,
		LIGHTS_DATA_BLOCK_INDEX = 1,
		MATERIAL_BLOCK_INDEX = 2
	};


	void render(double dt);

};
#endif
