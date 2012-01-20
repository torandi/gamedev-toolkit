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

	#define HALF_LIGHT_DISTANCE 1.5f


class Renderer {

	int checkForGLErrors( const char *s );
	GLuint vao;

	void render_skybox();

	Shader::lights_data_t lightData;

	GLuint skybox_buffer_;
	GLuint skybox_texture_;
	GLuint skybox_skymap_;

	void init_shader(Shader &shader);

	bool skybox_loaded_;

	int width_, height_;

	static std::string shader_files_[];
public:

	Renderer(int w, int h, bool fullscreen);
	~Renderer();
	
	float zNear;
	float zFar;
	bool cull_face;

	enum shader_program_t {
		NORMAL_SHADER=0,
		SKYBOX_SHADER,
		NUM_SHADERS
	};

	Shader shaders[NUM_SHADERS];

	//Uploads model and normal matrices
	void upload_model_matrices();

	//Load skybox
	void load_skybox(std::string skybox_path);

	void enable_face_culling();
	void disabel_face_culling();

	glutil::MatrixStack modelMatrix;
	glutil::MatrixStack projectionViewMatrix;
	float light_attenuation;
	glm::vec3 ambient_intensity;
	Camera camera;

	std::vector<RenderGroup*> render_objects;
	std::vector<Light*> lights;

	static GLuint load_texture(std::string file);

	void render(double dt);

};
#endif
