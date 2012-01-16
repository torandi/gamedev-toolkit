#ifndef RENDER_H
#define RENDER_H
	#include <vector>
	#include <glload/gl_3_3.h>
	#include <glutil/MatrixStack.h>

	#include "render_object.h"

	extern glutil::MatrixStack modelViewMatrix;
	extern glm::vec3 camera_pos, look_at, up_dir;

	struct shader_t {
		GLuint program;
		GLuint mvp;
		GLuint texture;

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
