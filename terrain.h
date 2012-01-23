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
	float * map_;

	SDL_Surface * load_image();
	void generate_terrain();
	void generate_water();
	void load_textures();

	glm::vec4 get_pixel_color(int x, int y);
	float height_from_color(const glm::vec4 &color);

	float get_height_at(int x, int y);
	float get_height_at(float x, float y);
	bool is_square_below_water(int x, int y);

	Mesh * terrain_mesh_;
	Mesh * water_mesh_;

	Texture * texture_;
	Texture * specular_map_;
	Texture * normal_map_;

	//Hide these functions:
	RenderGroup::operator[];
	RenderGroup::add_object;

	static const char * texture_files_[];
	const float water_level_;
	const float texture_scale_;
	float time_;
	int num_waves_;

	public:
		//Start height (relative this object) used when selecting terrain
		float start_height;
		//Water level

		int num_waves() { return num_waves_; };
		void set_num_waves(int num);
		float height() { return height_; };
		float width() { return width_; };
		Terrain(const std::string folder, float horizontal_scale, float vertical_scale, float water_level);
		~Terrain();

		virtual void render(double dt, Renderer * renderer);

		static void init_terrain(Renderer * renderer);
};

#endif
