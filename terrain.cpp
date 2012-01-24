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

const char * Terrain::texture_files_[] = {
	"dirt",
	"sand",
	"grass",
	"mountain",
	"snow"
};

#define NORMAL_TEXTURE ".jpg"
#define SPECULAR_MAP "_specular.jpg"
#define NORMAL_MAP "_normal.jpg"

#define TEXTURE_LEVELS 5

#define MAX_NUM_WAVES 16

void Terrain::init_terrain(Renderer * renderer) {
	renderer->load_shader_uniform_location(Renderer::TERRAIN_SHADER, "vertical_scale");
	renderer->load_shader_uniform_location(Renderer::TERRAIN_SHADER, "start_height");
	renderer->load_shader_uniform_location(Renderer::TERRAIN_SHADER, "specular_map");

	renderer->load_shader_uniform_location(Renderer::WATER_SHADER, "water_height");
	renderer->load_shader_uniform_location(Renderer::WATER_SHADER, "time");
	renderer->load_shader_uniform_location(Renderer::WATER_SHADER, "num_waves");

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
	Renderer::checkForGLErrors("Terrain::init() set num waves");

	for (int i = 0; i < MAX_NUM_WAVES; ++i) {
	  float amplitude = 0.7f / (i + 1);
	  renderer->load_shader_uniform_location(Renderer::WATER_SHADER, format("amplitude[%d]", i));
	  glUniform1f(water_shader.uniform[format("amplitude[%d]", i)], amplitude);

	  float wavelength = 15 * M_PI / (i + 1);
	  renderer->load_shader_uniform_location(Renderer::WATER_SHADER, format("wavelength[%d]", i));
	  glUniform1f(water_shader.uniform[format("wavelength[%d]", i)], wavelength);

	  float speed = 1.0f + 2*i;
	  renderer->load_shader_uniform_location(Renderer::WATER_SHADER, format("speed[%d]", i));
	  glUniform1f(water_shader.uniform[format("speed[%d]", i)], speed);

	  float angle = uniformRandomInRange(-M_PI/2, M_PI/2);
	  renderer->load_shader_uniform_location(Renderer::WATER_SHADER, format("direction[%d]", i));
	  glUniform2f(water_shader.uniform[format("direction[%d]", i)], cos(angle), sin(angle));
	  Renderer::checkForGLErrors("Terrain set wave data");
	}

	glUseProgram(0);
}

Terrain::~Terrain() {
	if(terrain_mesh_ != NULL)
		delete terrain_mesh_;
	if(water_mesh_ != NULL)
		delete water_mesh_;
	if(map_ != NULL)
		delete map_;
	if(texture_!= NULL)
		delete texture_;
	if(normal_map_!= NULL)
		delete normal_map_;
	if(specular_map_!= NULL)
		delete specular_map_;
}

Terrain::Terrain(const std::string folder, float horizontal_scale, float vertical_scale, float water_level) :
		RenderGroup(), folder_(folder+"/"),
		horizontal_scale_(horizontal_scale),
		vertical_scale_(vertical_scale),
		map_(NULL),
		terrain_mesh_(NULL),
		water_mesh_(NULL),
		texture_(NULL),
		specular_map_(NULL),
		normal_map_(NULL),
		water_level_(water_level),
		texture_scale_(128.0f) ,
		num_waves_(1),
		start_height(0.f)
		{
	heightmap_ = load_image();
	generate_terrain();	
	generate_water();
	load_textures();


	time_ = 0.0;

	position_-=glm::vec3(width_*horizontal_scale_, vertical_scale_, height_*horizontal_scale_)/2.0f;
	SDL_FreeSurface(heightmap_);
}

void  Terrain::set_num_waves(int num) {
	assert(num<=MAX_NUM_WAVES && num > 0);
	num_waves_ = num;
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
				glm::vec2 base_uv = glm::vec2(v.position.x/texture_scale_, v.position.z/texture_scale_);
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

	printf("Uploading to gfx memory\n");
	water_mesh_->generate_vbos();
}

void Terrain::load_textures() {
	std::vector<std::string> textures;
	std::vector<std::string> specular;
	std::vector<std::string> normal;
	for(unsigned int i=0;i<TEXTURE_LEVELS;++i) {
		textures.push_back((folder_+texture_files_[i])+NORMAL_TEXTURE);
		specular.push_back((folder_+texture_files_[i])+SPECULAR_MAP);
		normal.push_back((folder_+texture_files_[i])+NORMAL_MAP);
	}
	texture_ = new Texture(textures, false);
	specular_map_ = new Texture(specular, false);
	normal_map_ = new Texture(normal, false);
	texture_->bind();
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	specular_map_->bind();
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	normal_map_->bind();
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	texture_->unbind();
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


SDL_Surface * Terrain::load_image() {
	/* Load image using SDL Image */
	std::string heightmap = folder_+"heightmap.png";
	SDL_Surface* surface = IMG_Load(heightmap.c_str());
	if ( !surface ){
	  fprintf(stderr, "Failed to load heightmap at %s\n", heightmap.c_str());
	  exit(1);
	}
	SDL_Surface* rgba_surface = SDL_CreateRGBSurface(
			SDL_SWSURFACE,
			surface->w, surface->h,
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

	SDL_BlitSurface(surface, 0, rgba_surface, 0);

	/* Restore the alpha blending attributes */
	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
		SDL_SetAlpha(surface, saved_flags, saved_alpha);
	}

	width_ = rgba_surface->w;
	height_ = rgba_surface->h;

	SDL_FreeSurface(surface);
	return rgba_surface;
}

void Terrain::render(double dt, Renderer * renderer) {
	time_+=dt;

	glUseProgram(renderer->shaders[Renderer::TERRAIN_SHADER].program);

	glUniform1f(renderer->shaders[Renderer::TERRAIN_SHADER].uniform["vertical_scale"], vertical_scale_);
	glUniform1f(renderer->shaders[Renderer::TERRAIN_SHADER].uniform["start_height"], start_height);

	glActiveTexture(GL_TEXTURE0);
	texture_->bind();
	glActiveTexture(GL_TEXTURE1);
	normal_map_->bind();
	glActiveTexture(GL_TEXTURE2);
	specular_map_->bind();

	renderer->modelMatrix.Push();

	renderer->modelMatrix.ApplyMatrix(matrix());

	renderer->upload_model_matrices();

	terrain_mesh_->render();

	glUseProgram(renderer->shaders[Renderer::WATER_SHADER].program);
	glUniform1f(renderer->shaders[Renderer::WATER_SHADER].uniform["time"], time_);
	glUniform1f(renderer->shaders[Renderer::WATER_SHADER].uniform["water_height"], water_level_);
	glUniform1i(renderer->shaders[Renderer::WATER_SHADER].uniform["num_waves"], num_waves_);

	water_mesh_->render();

#if RENDER_DEBUG
	//Render debug:
	glLineWidth(2.0f);
	glUseProgram(renderer->shaders[Renderer::DEBUG_SHADER].program);


	terrain_mesh_->render();
	water_mesh_->render();
#endif

	renderer->modelMatrix.Pop();

	texture_->unbind();

	glUseProgram(0);
}
