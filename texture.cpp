#include "renderer.h"
#include "texture.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <vector>
#include <string>
#include <cassert>

GLuint Texture::cube_map_index_[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 
};

Texture::Texture(const std::string &path) :
	_texture(-1),
	_width(0),
	_height(0),
	_num_textures(1)
	{
	_filenames = new std::string[_num_textures];
	_filenames[0] = path;
	_texture_type = GL_TEXTURE_2D;
	load_texture();
}

Texture::Texture(const std::vector<std::string> &paths, bool cube_map) :
	_texture(-1),
	_width(0),
	_height(0),
	_num_textures(paths.size())
	{
	if(cube_map) {
		assert(_num_textures == 6);
		_texture_type = GL_TEXTURE_CUBE_MAP;
	} else
		_texture_type = GL_TEXTURE_2D_ARRAY;
	_filenames = new std::string[_num_textures];
	int i=0;
	for(std::vector<std::string>::const_iterator it=paths.begin(); it!=paths.end(); ++it) {
		_filenames[i++] = (*it);
	}
	load_texture();
}

Texture::~Texture(){
	delete[] _filenames;
	free_texture();
}

int Texture::width() const {
	return _width;
}

int Texture::height() const {
	return _height;
}

void Texture::bind() const {
	assert(_texture != (unsigned int)-1);
	glBindTexture(_texture_type, _texture);
	Renderer::checkForGLErrors("Texture::bind()");
}

void Texture::unbind() const {
	glBindTexture(_texture_type, 0);
}

GLuint Texture::texture() const {
	return _texture;
}

void Texture::load_texture() {
	assert(_texture == (unsigned int)-1);
	//Load textures:
	SDL_Surface ** images = new SDL_Surface*[_num_textures];
	for(unsigned int i=0; i < _num_textures; ++i) {
		images[i] = load_image(_filenames[i]);
		printf("Loaded SDL image %s, size %dx%d\n", _filenames[i].c_str(), images[0]->w, images[0]->h);
	}
	_width = images[0]->w;
	_height = images[0]->h;

	//Generate texture:
	glGenTextures(1, &_texture);
	bind();	
	Renderer::checkForGLErrors("load_texture(): gen buffer");


	switch(_texture_type) {
		case GL_TEXTURE_2D:
			//One texture only:
			glTexParameteri(_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, images[0]->pixels );
			Renderer::checkForGLErrors("load_texture(): write GL_TEXTURE_2D data");
			break;
		case GL_TEXTURE_2D_ARRAY:
			//Generate the array:
			glTexParameteri(_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, _width, _height, _num_textures, 0,   GL_RGBA,  GL_UNSIGNED_BYTE, NULL);
			Renderer::checkForGLErrors("load_texture(): gen 2d array buffer");

			//Fill the array with data:
			for(unsigned int i=0; i < _num_textures; ++i) {
				//											, lvl, x, y, z, width, height, depth
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, _width, _height, 1, GL_RGBA,  GL_UNSIGNED_BYTE, images[i]->pixels);
			}
			break;
		case GL_TEXTURE_CUBE_MAP:
			set_clamp_params();
			for(int i=0; i < 6; ++i) {
				glTexImage2D(cube_map_index_[i], 0, GL_RGBA, _width,_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, images[i]->pixels );
				Renderer::checkForGLErrors("load_texture(): Fill cube map");
			}
			break;
		default:
			fprintf(stderr, "Error! Invalid texture type encountered when loading textures, exiting (Texture::load_texture()");
			exit(3);
	}

	unbind();

	//Free images:
	for(unsigned int i=0; i<_num_textures; ++i) {
		SDL_FreeSurface(images[i]);
	}
	delete[] images;
}

//Requires the texture to be bound!
void Texture::set_clamp_params() {
	glTexParameteri(_texture_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(_texture_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(_texture_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(_texture_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(_texture_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

SDL_Surface * Texture::load_image(const std::string &path) {
	/* Load image using SDL Image */
	SDL_Surface* surface = IMG_Load(path.c_str());
	if ( !surface ){
		fprintf(stderr, "Failed to load texture at %s\n", path.c_str());
		exit(1);
	}

	/* To properly support all formats the surface must be copied to a new
	 * surface with a prespecified pixel format suitable for opengl.
	 *
	 * This snippet is a slightly modified version of code posted by
	 * Sam Lantinga to the SDL mailinglist at Sep 11 2002.
	 */
	SDL_Surface* rgba_surface = SDL_CreateRGBSurface(
			SDL_SWSURFACE,
			surface->w, surface->h,
			32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
			0xFF000000
#else
			0xFF000000,
			0x00FF0000,
			0x0000FF00,
			0x000000FF
#endif
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

	return rgba_surface;
}

void Texture::free_texture(){
	if(_texture != (unsigned int)-1) {
		glDeleteTextures(1, &_texture);
		_texture = -1;
	}
}
