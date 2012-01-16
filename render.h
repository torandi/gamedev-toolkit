#ifndef RENDER_H
#define RENDER_H
	#include <vector>
	#include <glload/gl_3_3.h>
	#include <glutil/MatrixStack.h>

	struct RenderObject;

	#define MAX_NUM_LIGHTS 4

	extern glutil::MatrixStack modelViewMatrix;
	extern glm::vec3 camera_pos, look_at, up_dir;
	extern float light_attenuation;
	extern glm::vec4 ambient_intensity;

	struct shader_material_t {
		unsigned int useTexture; //Set to 1 to use texture
		float shininess;
		float p2[2]; //Padding
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 ambient;
		glm::vec4 emission;
	};

	struct light_t {
		glm::vec4 intensity;
		glm::vec4 position;
	};

	struct lights_t {
		unsigned int num_lights;
		float attenuation;
		float padding[2];
		glm::vec4 ambient_intensity;
		light_t lights[MAX_NUM_LIGHTS];
	};

	struct shader_t {
		GLuint program;

		GLuint Matrices;
		GLuint camera_pos;
		GLuint LightsData;
		GLuint Material;
		GLuint texture;
	};

	enum {
		MATRICES_BLOCK_INDEX = 0,
		LIGHTS_DATA_BLOCK_INDEX = 1,
		MATERIAL_BLOCK_INDEX = 2
	};

	struct shader_globals_t {
		GLuint matricesBuffer;
		GLuint lightsBuffer;
		GLuint materialBuffer;
	};

	extern shader_globals_t sg;

	extern std::vector<light_t> lights;

	extern shader_t shader;

	extern std::vector<RenderObject> objects;

	void render_init(int w, int h, bool fullscreen);

	void render(double dt);

	
#endif
