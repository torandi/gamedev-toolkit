#ifndef TERRAIN_H
#define TERRAIN_H

#include <string>
#include <glm/glm.hpp>
#include <glload/gl_3_3.h>
#include <SDL/SDL.h>

#include "renderer.h"
#include "render_group.h"
#include "mesh.h"
#include "texture.h"

class Terrain : public RenderGroup {
	std::string folder_;
	float horizontal_scale_;
	float vertical_scale_;
	SDL_Surface * heightmap_;
	int width_, height_;

	SDL_Surface * load_image();
	void generate_terrain();
	void load_textures();

	glm::vec4 get_pixel_color(int x, int y);
	float height_from_color(const glm::vec4 &color);

	Mesh * mesh_;

	Texture * texture_;

	//Hide these functions:
	RenderGroup::operator[];
	RenderGroup::add_object;

	static const char * texture_files_[];

	public:
		//Start height (relative this object) used when selecting terrain
		float start_height;

		float height() { return height_; };
		float width() { return width_; };
		Terrain(const std::string folder, float horizontal_scale, float vertical_scale);
		~Terrain();

		virtual void render(double dt, Renderer * renderer);

		static void init_terrain(Renderer * renderer);
};

#endif
