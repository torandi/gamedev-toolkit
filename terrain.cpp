#include "terrain.h"
#include "texture.h"
#include "renderer.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <glm/glm.hpp>
#include <string>

Terrain::Terrain(const std::string folder) : RenderGroup(), folder_(folder) {
	horizontal_scale_ = 1.0f;
	vertical_scale_ = 100.f;
	heightmap_ = load_image();
	generate_terrain();	

	position_-=glm::vec3(0.0, 100.0, 0.0);
	SDL_FreeSurface(heightmap_);
}

void Terrain::generate_terrain() {
	int numVertices = width_*height_;

	printf("Generating terrain...\n");
	
	//Build heightmap:

	map_ = new float[numVertices];

	for(int y=0; y<height_; ++y) {
		for(int x=0; x<width_; ++x) {
			int i = y * width_ + x;
			glm::vec4 color = get_pixel_color(x, y);
			map_[i] = vertical_scale_*height_from_color(color);
		}
	}


	
	vertex_t * vertex_data = new vertex_t[numVertices];
	for(int y=0; y<height_; ++y) {
		for(int x=0; x<width_; ++x) {
			vertex_t v;
			int i = y * width_ + x;
			glm::vec4 color = get_pixel_color(x, y);
			v.position = glm::vec3(horizontal_scale_*x, vertical_scale_*height_from_color(color), horizontal_scale_*y);
			v.texCoord = glm::vec2(v.position.x/2.0f, v.position.z/2.0f);
			vertex_data[i] = v;
		}
	}


	int indexCount = (height_ - 1 ) * (width_ -1) * 6;

	GLuint * indices  = new GLuint[indexCount];
	//build indices
	for(int x=0; x<width_- 1; ++x) {
		for(int y=0; y<height_- 1; ++y) {
			int i = y * (width_-1) + x;
			indices[i*6 + 0] = x + y*width_;
			indices[i*6 + 1] = (x + 1) + y*width_;
			indices[i*6 + 2] = x + (y+1)*width_;
			indices[i*6 + 3] = x + (y+1)*width_;
			indices[i*6 + 4] = (x+1) + (y+1)*width_;
			indices[i*6 + 5] = (x + 1) + y*width_;
		}
	}
	printf("Terrain generated, uploading to graphic memory.\n");

	num_triangles_ = indexCount/3;

	//Upload data:
	glGenBuffers(1, &vb_);
	glGenBuffers(1, &ib_);

	glBindBuffer(GL_ARRAY_BUFFER, vb_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t)*numVertices, vertex_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*indexCount, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete vertex_data;
	delete indices;
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
	std::string heightmap = folder_+"/heightmap.png";
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

	renderer->modelMatrix.Push();

	renderer->modelMatrix.ApplyMatrix(matrix());

	renderer->upload_model_matrices();
	
	glBindBuffer(GL_ARRAY_BUFFER, vb_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib_);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (sizeof(glm::vec3)));

	glDrawElements(GL_TRIANGLES, num_triangles_, GL_UNSIGNED_INT, 0);	
	
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	renderer->modelMatrix.Pop();

	glUseProgram(0);
}
