#ifndef RENDER_H
#define RENDER_H
	#include <vector>
	#include <glload/gl_3_3.h>
	#include <glutil/MatrixStack.h>

	#include "render_object.h"

	extern glutil::MatrixStack modelViewMatrix;
	extern glm::vec3 camera_pos, look_at, up_dir;
	extern float light_attenuation;
	extern glm::vec4 light_intensity, ambient_intensity, light_pos;

	struct shader_t {
		GLuint program;
		GLuint mvp;
		GLuint projection_matrix;
		GLuint texture;
		GLuint camera_pos;

		GLuint light_pos;
		GLuint light_attenuation;
		GLuint light_intensity;
		GLuint ambient_intensity;

		GLuint diffuse;
		GLuint specular;
		GLuint ambient;
		GLuint emission;
		GLuint shininess;
	};
	extern shader_t shader;

	extern std::vector<RenderObject> objects;

	void render_init(int w, int h, bool fullscreen);

	void render(double dt);

	
#endif
