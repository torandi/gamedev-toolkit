#ifndef TERRAIN_H
#define TERRAIN_H

#include <string>
#include <vector>
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
	float * map_;

	SDL_Surface * load_image(glm::vec2 size);
	void generate_terrain();
	void generate_water();

	glm::vec4 get_pixel_color(int x, int y);
	float height_from_color(const glm::vec4 &color);

	float get_height_at(int x, int y);
	float get_height_at(float x, float y);
	bool is_square_below_water(int x, int y);

	Mesh * terrain_mesh_;
	Mesh * water_mesh_;

	//Hide these functions:
	RenderGroup::operator[];
	RenderGroup::add_object;

	const float water_level_;
	const float texture_scale_;
	float time_;
	int num_waves_;
	texture_pack_t * textures_;

	public:
		static texture_pack_t * generate_texture_pack(std::string folder, std::vector<std::string> texture_files);
		//Start height (relative this object) used when selecting terrain
		float start_height;
		glm::vec2 position;
		//Water level

		int num_waves() { return num_waves_; };
		void set_num_waves(int num);
		float height() { return height_; };
		float width() { return width_; };
		float water_level() { return water_level_; };
		float vertical_scale() { return vertical_scale_; };
		Terrain(const std::string folder, float horizontal_scale, float vertical_scale, float water_level, texture_pack_t *textures, glm::vec2 pos=glm::vec2(0,0), glm::vec2 size=glm::vec2(0,0));
		~Terrain();

		virtual void render(double dt, Renderer * renderer);

		static void init_terrain(Renderer * renderer);
};

#endif
