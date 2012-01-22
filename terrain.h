#ifndef TERRAIN_H
#define TERRAIN_H

#include <string>
#include <glm/glm.hpp>
#include <glload/gl_3_3.h>
#include <SDL/SDL.h>

#include "renderer.h"
#include "render_group.h"
#include "mesh.h"

class Terrain : public RenderGroup {
	std::string folder_;
	float horizontal_scale_;
	float vertical_scale_;
	SDL_Surface * heightmap_;
	int width_, height_;

	SDL_Surface * load_image();
	void generate_terrain();

	glm::vec4 get_pixel_color(int x, int y);
	float height_from_color(const glm::vec4 &color);

	Mesh * mesh_;

	//Hide these functions:
	RenderGroup::operator[];
	RenderGroup::add_object;

	public:
		float height() { return height_; };
		float width() { return width_; };
		Terrain(const std::string folder);
		~Terrain();

		virtual void render(double dt, Renderer * renderer);
};

#endif
