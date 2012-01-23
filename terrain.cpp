#include "terrain.h"
#include "texture.h"
#include "renderer.h"
#include "mesh.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#define RENDER_DEBUG 0

const char * Terrain::texture_files_[] = {
	"dirt.dds",
	"sand.dds",
	"grass.dds",
	"mountain.dds",
	"snow.dds"
};

#define TEXTURE_LEVELS 5

void Terrain::init_terrain(Renderer * renderer) {
	renderer->load_shader_uniform_location(Renderer::TERRAIN_SHADER, "vertical_scale");
	renderer->load_shader_uniform_location(Renderer::TERRAIN_SHADER, "start_height");
	
	glUseProgram(renderer->shaders[Renderer::TERRAIN_SHADER].program);

	glSamplerParameteri(renderer->shaders[Renderer::TERRAIN_SHADER].texture_array1, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(renderer->shaders[Renderer::TERRAIN_SHADER].texture_array1, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	glUseProgram(0);
}

Terrain::~Terrain() {
	if(mesh_ != NULL)
		delete mesh_;
}

Terrain::Terrain(const std::string folder, float horizontal_scale, float vertical_scale) :
		RenderGroup(), folder_(folder+"/"),
		horizontal_scale_(horizontal_scale),
		vertical_scale_(vertical_scale),
		mesh_(NULL),
		start_height(0.f)
		{
	heightmap_ = load_image();
	generate_terrain();	
	load_textures();

	position_-=glm::vec3(width_*horizontal_scale_, vertical_scale_, height_*horizontal_scale_/2.0)/2.0f;
	SDL_FreeSurface(heightmap_);
}

void Terrain::generate_terrain() {
	int numVertices = width_*height_;

	printf("Generating terrain...\n");


	std::vector<Mesh::vertex_t> vertices(numVertices);
	for(int y=0; y<height_; ++y) {
		for(int x=0; x<width_; ++x) {
			Mesh::vertex_t v;
			int i = y * width_ + x;
			glm::vec4 color = get_pixel_color(x, y);
			v.position = glm::vec3(horizontal_scale_*x, vertical_scale_*height_from_color(color), horizontal_scale_*y);
			v.texCoord = glm::vec2(v.position.x/2.0, v.position.z/2.0);
			vertices[i] = v;
		}
	}

	int indexCount = (height_ - 1 ) * (width_ -1) * 6;

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

	mesh_ = new Mesh(vertices, indices);

	printf("Generating normals\n");
	mesh_->generate_normals();
	printf("Generating tangents\n");
	mesh_->generate_tangents_and_bitangents();
	printf("Ortonormalizing tangent space\n");
	mesh_->ortonormalize_tangent_space();
	printf("Uploading to gfx memory\n");
	mesh_->generate_vbos();
}

void Terrain::load_textures() {
	std::vector<std::string> textures;
	for(unsigned int i=0;i<TEXTURE_LEVELS;++i) {
		textures.push_back(folder_+texture_files_[i]);
	}
	texture_ = new Texture(textures, false);
	texture_->bind();
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, texture_->mipmap_count());
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 6);
	printf("Mipmap count: %d\n", texture_->mipmap_count());
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
	glUseProgram(renderer->shaders[Renderer::TERRAIN_SHADER].program);

	glUniform1f(renderer->shaders[Renderer::TERRAIN_SHADER].uniform["vertical_scale"], vertical_scale_);
	glUniform1f(renderer->shaders[Renderer::TERRAIN_SHADER].uniform["start_height"], start_height);

	glActiveTexture(GL_TEXTURE0);

	texture_->bind();

	renderer->modelMatrix.Push();

	renderer->modelMatrix.ApplyMatrix(matrix());

	renderer->upload_model_matrices();

	mesh_->render();

#if RENDER_DEBUG
	//Render debug:
	glLineWidth(2.0f);
	glUseProgram(renderer->shaders[Renderer::DEBUG_SHADER].program);


	mesh_->render();
#endif

	renderer->modelMatrix.Pop();

	texture_->unbind();

	glUseProgram(0);
}
