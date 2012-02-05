#include "terrain.h"
#include "texture.h"
#include "renderer.h"
#include "mesh.h"
#include "util.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#define RENDER_DEBUG 0

#define NORMAL_TEXTURE ".jpg"
#define SPECULAR_MAP "_specular.jpg"
#define NORMAL_MAP "_normal.jpg"

#define TEXTURE_LEVELS 5

void Terrain::init_terrain(Renderer * renderer) {
	renderer->load_shader_uniform_location(Renderer::TERRAIN_SHADER, "vertical_scale");
	renderer->load_shader_uniform_location(Renderer::TERRAIN_SHADER, "start_height");
	renderer->load_shader_uniform_location(Renderer::TERRAIN_SHADER, "specular_map");

	renderer->load_shader_uniform_location(Renderer::WATER_SHADER, "water_height");
	renderer->load_shader_uniform_location(Renderer::WATER_SHADER, "time");
	renderer->load_shader_uniform_location(Renderer::WATER_SHADER, "wave1");
	renderer->load_shader_uniform_location(Renderer::WATER_SHADER, "wave2");

	Renderer::checkForGLErrors("Terrain::init() load shader uniforms");
	
	glUseProgram(renderer->shaders[Renderer::TERRAIN_SHADER].program);
	glUniform1i(renderer->shaders[Renderer::TERRAIN_SHADER].uniform["specular_map"], 2);

	glSamplerParameteri(renderer->shaders[Renderer::TERRAIN_SHADER].texture_array1, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(renderer->shaders[Renderer::TERRAIN_SHADER].texture_array1, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glSamplerParameteri(renderer->shaders[Renderer::TERRAIN_SHADER].texture_array1, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameterf(renderer->shaders[Renderer::TERRAIN_SHADER].texture_array1, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);

	Renderer::checkForGLErrors("Terrain::init() set terrain data");
	//Set wave data:
	Shader &water_shader = renderer->shaders[Renderer::WATER_SHADER];

	glUseProgram(water_shader.program);

	glSamplerParameteri(water_shader.texture1, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(water_shader.texture1, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glSamplerParameterf(water_shader.texture1, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);

	glUseProgram(0);
}

Terrain::~Terrain() {
	if(terrain_mesh_ != NULL)
		delete terrain_mesh_;
	if(water_mesh_ != NULL)
		delete water_mesh_;
	if(map_ != NULL)
		delete map_;
}

Terrain::Terrain(const std::string folder, float horizontal_scale, float vertical_scale, float water_level, texture_pack_t * textures,  Texture * water_nm, glm::vec2 chunk_pos, glm::vec2 size) :
		RenderGroup(), folder_(folder+"/"),
		horizontal_scale_(horizontal_scale),
		vertical_scale_(vertical_scale),
		map_(NULL),
		terrain_mesh_(NULL),
		water_mesh_(NULL),
		water_level_(water_level*vertical_scale_),
		texture_scale_(128.0f) ,
		num_waves_(1),
		textures_(textures),
		water_normal_map_(water_nm),
		start_height(0.f),
		chunk_position(chunk_pos)
		{
	heightmap_ = load_image(size);
	generate_terrain();	
	generate_water();
	SDL_FreeSurface(heightmap_);

	time_ = 0.0;

	position_-=glm::vec3(width_*horizontal_scale_, 0, height_*horizontal_scale_)/2.0f;
}

void Terrain::generate_terrain() {
	unsigned long numVertices = width_*height_;

	printf("Generating terrain...\n");
	printf("Word size: %dx%d\n", width_, height_);

	map_ = new float[numVertices];
	std::vector<Mesh::vertex_t> vertices(numVertices);
	for(int y=0; y<height_; ++y) {
		for(int x=0; x<width_; ++x) {
			Mesh::vertex_t v;
			int i = y * width_ + x;
			glm::vec4 color = get_pixel_color(x, y);
			float h = height_from_color(color);
			map_[i] =  h*vertical_scale_;
			v.position = glm::vec3(horizontal_scale_*x, h*vertical_scale_, horizontal_scale_*y); 
			v.texCoord = glm::vec2(v.position.x/texture_scale_, v.position.z/texture_scale_);
			vertices[i] = v;
		}
	}
	unsigned long indexCount = (height_ - 1 ) * (width_ -1) * 6;

	std::vector<unsigned int> indices(indexCount);
	//build indices
	for(int x=0; x<width_- 1; ++x) {
		for(int y=0; y<height_- 1; ++y) {
			int i = y * (width_-1) + x;
			indices[i*6 + 0] = x + y*width_;
			indices[i*6 + 1] = x + (y+1)*width_;
			indices[i*6 + 2] = (x + 1) + y*width_;
			indices[i*6 + 3] = x + (y+1)*width_;
			indices[i*6 + 4] = (x+1) + (y+1)*width_;
			indices[i*6 + 5] = (x + 1) + y*width_;
		}
	}
	printf("Terrain generated, creating mesh\n");

	terrain_mesh_ = new Mesh(vertices, indices);

	printf("Generating normals\n");
	terrain_mesh_->generate_normals();
	printf("Generating tangents\n");
	terrain_mesh_->generate_tangents_and_bitangents();
	printf("Ortonormalizing tangent space\n");
	terrain_mesh_->ortonormalize_tangent_space();
	printf("Uploading to gfx memory\n");
	terrain_mesh_->generate_vbos();
}

void Terrain::generate_water() {
	std::vector<unsigned int> indices;
	std::vector<Mesh::vertex_t> vertices;

	unsigned long vi=0;
	for(int y=0; y<height_-1; ++y) {
		for(int x=0; x<width_-1; ++x) {
			if(is_square_below_water(x, y)) {
				Mesh::vertex_t v;
				//Note that the y component is not the water height. It is used to calculate water depth in shader
				//Water height is set with uniform 
				glm::vec3 base_pos = glm::vec3(horizontal_scale_*x,get_height_at(x, y) , horizontal_scale_*y);
				glm::vec2 base_uv = glm::vec2(base_pos.x/texture_scale_, base_pos.z/texture_scale_);
				for(int i=0;i<2;++i) {
					v.position = base_pos;
					v.texCoord = base_uv;
					v.position.x+=i*horizontal_scale_;
					v.texCoord.x+=i*horizontal_scale_/texture_scale_; 
					for(int j=0;j<2;++j) {
						v.position.z+=j*horizontal_scale_;
						v.texCoord.y+=j*horizontal_scale_/texture_scale_; 
						vertices.push_back(v);
					}
				}
				indices.push_back(vi);
				indices.push_back(vi+1);
				indices.push_back(vi+2);
				indices.push_back(vi+1);
				indices.push_back(vi+3);
				indices.push_back(vi+2);
				vi+=4;
			}
		}
	}

	printf("Water generated, %d squares was under water, creating mesh\n", (int)(vertices.size()/4));

	water_mesh_ = new Mesh(vertices, indices);

	printf("Generating normals\n");
	water_mesh_->generate_normals();
	printf("Generating tangents\n");
	water_mesh_->generate_tangents_and_bitangents();
	printf("Ortonormalizing tangent space\n");
	water_mesh_->ortonormalize_tangent_space();
	printf("Uploading to gfx memory\n");
	water_mesh_->generate_vbos();
}

texture_pack_t  * Terrain::generate_texture_pack(std::string folder, std::vector<std::string> texture_files) {
	std::vector<std::string> textures;
	std::vector<std::string> normal;
	std::vector<std::string> specular;
	texture_pack_t * tp = new texture_pack_t();

	for(std::vector<std::string>::iterator it=texture_files.begin(); it!=texture_files.end(); ++it) {
		textures.push_back((folder+"/"+(*it))+NORMAL_TEXTURE);
		normal.push_back((folder+"/"+(*it))+NORMAL_MAP);
		specular.push_back((folder+"/"+(*it))+SPECULAR_MAP);
	}
	tp->texture = new Texture(textures, false);
	tp->normal_map = new Texture(normal, false);
	tp->specular_map =  new Texture(specular, false);
	tp->texture->bind();
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	tp->normal_map->bind();
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	tp->specular_map->bind();
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	tp->specular_map->unbind();
	return tp;
}

float Terrain::height_from_color(const glm::vec4 &color) {
	return (color.r+color.g+color.b+color.a)/4.0;
}

glm::vec4 Terrain::get_pixel_color(int x, int y) {
	glm::vec4 color;

	Uint32 temp, pixel;
	Uint8 red, green, blue, alpha;
	pixel = ((Uint32*)heightmap_->pixels)[(height_-(y+1))*(width_)+(width_-(x+1))];

	SDL_PixelFormat * fmt=heightmap_->format;

	/* Get Red component */
	temp = pixel & fmt->Rmask;  /* Isolate red component */
	temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
	temp = temp << fmt->Rloss;  /* Expand to a full 8-bit number */
	red = (Uint8)temp;

	/* Get Green component */
	temp = pixel & fmt->Gmask;  /* Isolate green component */
	temp = temp >> fmt->Gshift; /* Shift it down to 8-bit */
	temp = temp << fmt->Gloss;  /* Expand to a full 8-bit number */
	green = (Uint8)temp;

	/* Get Blue component */
	temp = pixel & fmt->Bmask;  /* Isolate blue component */
	temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
	temp = temp << fmt->Bloss;  /* Expand to a full 8-bit number */
	blue = (Uint8)temp;

	/* Get Alpha component */
	temp = pixel & fmt->Amask;  /* Isolate alpha component */
	temp = temp >> fmt->Ashift; /* Shift it down to 8-bit */
	temp = temp << fmt->Aloss;  /* Expand to a full 8-bit number */
	alpha = (Uint8)temp;

	color.r = (float)red/0xFF;
	color.g = (float)green/0xFF;
	color.b = (float)blue/0xFF;
	color.a = (float)alpha/0xFF;

	return color;	
}

float Terrain::get_height_at(int x, int y) {
	return map_[y*width_ + x];
}

float Terrain::get_height_at(float x_, float y_) {
	int x = (int) (x_/horizontal_scale_);
	int y = (int) (y_/horizontal_scale_);
	float dx = (x_/horizontal_scale_) - x;
	float dy = (y_/horizontal_scale_) - y;
	float height=0;
	height += (1.0-dx) * (1.0-dy) * map_[y*width_ + x];
	height += dx * (1.0-dy) * map_[y*width_ + x+1];
	height += (1.0-dx) * dy * map_[(y+1)*width_ + x];
	height += dx * dy * map_[(y+1)*width_ + x+1];
	return height;
}

bool Terrain::is_square_below_water(int x, int y) {
	return ((get_height_at(x, y) < water_level_) ||
		(get_height_at(x+1, y) < water_level_) ||
		(get_height_at(x, y+1) < water_level_) ||
		(get_height_at(x+1, y+1) < water_level_));
}


SDL_Surface * Terrain::load_image(glm::vec2 size) {
	/* Load image using SDL Image */
	std::string heightmap = folder_+"heightmap.png";
	SDL_Surface* surface = IMG_Load(heightmap.c_str());
	if ( !surface ){
	  fprintf(stderr, "Failed to load heightmap at %s\n", heightmap.c_str());
	  exit(1);
	}
	if(size.x == 0 && size.y == 0) {
		width_ = surface->w;
		height_ = surface->h;
	} else {
		width_ = size.x;
		height_ = size.y;
	}
	SDL_Surface* rgba_surface = SDL_CreateRGBSurface(
			SDL_SWSURFACE,
			width_, height_,
			32,
			0xFF000000,
			0x00FF0000,
			0x0000FF00,
			0x000000FF
	);

	if ( !rgba_surface ) {
	  fprintf(stderr, "Failed to create RGBA surface\n");
	  exit(1);
	}

	/* Save the alpha blending attributes */
	Uint32 saved_flags = surface->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
	Uint8 saved_alpha = surface->format->alpha;
	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
		SDL_SetAlpha(surface, 0, 0);
	}

	SDL_Rect srcrect;
	srcrect.x = chunk_position.x;
	srcrect.y = chunk_position.y;
	srcrect.w = width_;
	srcrect.h = height_;

	SDL_BlitSurface(surface, &srcrect, rgba_surface, 0);

	/* Restore the alpha blending attributes */
	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
		SDL_SetAlpha(surface, saved_flags, saved_alpha);
	}


	SDL_FreeSurface(surface);
	return rgba_surface;
}

void Terrain::render(double dt, Renderer * renderer) {
	time_+=dt;

	glUseProgram(renderer->shaders[Renderer::TERRAIN_SHADER].program);

	glUniform1f(renderer->shaders[Renderer::TERRAIN_SHADER].uniform["vertical_scale"], vertical_scale_);
	glUniform1f(renderer->shaders[Renderer::TERRAIN_SHADER].uniform["start_height"], start_height);


	renderer->modelMatrix.Push();

	renderer->modelMatrix.ApplyMatrix(matrix());

	renderer->upload_model_matrices();

	textures_->bind();
	terrain_mesh_->render();
	textures_->unbind();

	glUseProgram(renderer->shaders[Renderer::WATER_SHADER].program);
	glUniform1f(renderer->shaders[Renderer::WATER_SHADER].uniform["time"], time_);
	glUniform1f(renderer->shaders[Renderer::WATER_SHADER].uniform["water_height"], water_level_);
	glUniform2fv(renderer->shaders[Renderer::WATER_SHADER].uniform["wave1"], 1, glm::value_ptr(wave1));
	glUniform2fv(renderer->shaders[Renderer::WATER_SHADER].uniform["wave2"], 1, glm::value_ptr(wave2));


	glActiveTexture(GL_TEXTURE0);
	water_normal_map_->bind();

	water_mesh_->render();

	Renderer::checkForGLErrors("Render water ");

	water_normal_map_->unbind();

#if RENDER_DEBUG
	//Render debug:
	glLineWidth(2.0f);
	glUseProgram(renderer->shaders[Renderer::DEBUG_SHADER].program);


	terrain_mesh_->render();
#endif

	renderer->modelMatrix.Pop();


	glUseProgram(0);
}
