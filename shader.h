#ifndef SHADER_H
#define SHADER_H
#include <glload/gll.hpp>
#include <glload/gl_3_3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "light.h"

#define SHADER_PATH "shaders/"
#define VERT_SHADER_EXTENTION ".vert"
#define FRAG_SHADER_EXTENTION ".frag"

#define MAX_NUM_LIGHTS 4

class Shader {
	static GLuint load_shader(GLenum eShaderType, const std::string &strFilename);
	static GLuint create_program(const std::vector<GLuint> &shaderList);
	
	static void load_file(const std::string &filename, std::stringstream &shaderData, std::string included_from);
	static std::string parse_shader(const std::string &filename, std::set<std::string> included_files=std::set<std::string>(), std::string included_from="");

public:

	std::string name;

	struct lights_data_t {
		unsigned int num_lights;
		float attenuation;
		float padding[2];
		glm::vec4 ambient_intensity;
		Light::shader_light_t lights[MAX_NUM_LIGHTS];
	};

	struct material_t {
		/*
			The extra parameter is used for different things in different shaders:
			NORMAL_SHADER: No use
			LIGHT_SHADER: [0..MAX_NUM_LIGHTS] My light id
		*/
		unsigned int use_texture;
		unsigned int use_normal_map;
		unsigned int extra;
		float shininess;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 ambient;
		glm::vec4 emission;
	};


	struct globals_t {
		GLuint matricesBuffer;
		GLuint lightsBuffer;
		GLuint materialBuffer;
		GLuint cameraBuffer;
	};

	enum {
		MATRICES_BLOCK_INDEX = 0,
		LIGHTS_DATA_BLOCK_INDEX = 1,
		MATERIAL_BLOCK_INDEX = 2,
		CAMERA_BLOCK_INDEX = 3
	};

	static globals_t globals;

	GLuint program;

	GLint Matrices;
	GLint camera_pos;
	GLint LightsData;
	GLint Material;
	GLint Camera;
	GLint texture;

	std::map<std::string, GLint> uniform; //For shader specific uniforms

	static Shader create_shader(std::string base_name);
};
#endif
