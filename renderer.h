#ifndef RENDERER_H
#define RENDERER_H
	#include <vector>
	#include <string>
	#include <glload/gl_3_3.h>
	#include <glutil/MatrixStack.h>

	#include "camera.h"
	#include "light.h"
	#include "render_group.h"
	#include "shader.h"
	#include "texture.h"	



class Renderer {

	GLuint vao;

	void render_skybox();

	Shader::lights_data_t lightData;

	GLuint skybox_buffer_;

	void init_shader(Shader &shader);

	int width_, height_;

	static std::string shader_files_[];
public:
	Texture * skybox_texture;

	Renderer(int w, int h, bool fullscreen);
	~Renderer();
	
	float zNear;
	float zFar;
	bool cull_face;

	enum shader_program_t {
		NORMAL_SHADER=0,
		SKYBOX_SHADER,
		TERRAIN_SHADER,
		WATER_SHADER,
		PARTICLES_SHADER,
		DEBUG_SHADER,
		NUM_SHADERS
	};

	static int checkForGLErrors( const char *s );

	Shader shaders[NUM_SHADERS];

	void load_shader_uniform_location(shader_program_t shader, std::string uniform_name);

	//Uploads model and normal matrices
	void upload_model_matrices();

	//Load skybox
	void load_skybox(std::string skybox_path);

	void enable_face_culling();
	void disabel_face_culling();

	glutil::MatrixStack modelMatrix;
	glutil::MatrixStack projectionViewMatrix;
	glm::vec3 ambient_intensity;
	Camera camera;

	std::vector<RenderGroup*> render_objects;
	std::vector<Light*> lights;

	void render(double dt);

};
#endif
