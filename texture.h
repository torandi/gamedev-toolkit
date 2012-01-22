#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <vector>
#include <glload/gl_3_3.h>
#include <glimg/glimg.h>

class Texture  {
	public:
		//Load a single texture as GL_TEXTURE_2D
		Texture(const std::string & path);
		/*
		 * Load an array of textures, set cube_map to true to use it as an cube map
		 * The cubemap textures will be loaded in the following order:
		 *	 GL_TEXTURE_CUBE_MAP_POSITIVE_X
		 *	 GL_TEXTURE_CUBE_MAP_NEGATIVE_X
		 *	 GL_TEXTURE_CUBE_MAP_POSITIVE_Y
		 *	 GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
		 *	 GL_TEXTURE_CUBE_MAP_POSITIVE_Z
		 *	 GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 
		 */
		Texture(const std::vector<std::string> &paths, bool cube_map=false);
		~Texture();

		int width() const;
		int height() const;

		void bind() const;
		void unbind() const;

		//Get texture number on open gl
		GLuint texture() const; 

		static glimg::ImageSet * load_image(const std::string &path);

		/* 
		 * Requires the texture to be bound!
		 *
		 * Sets parameters to CLAMP_EDGE and GL_NEAREST (skybox etc)
		 */
		void set_clamp_params();

	private:
		//Copy not allowed (no body implemented, intentional!)
		Texture(const Texture &other);

		void load_texture();
		void free_texture();

		std::string * _filenames;
		GLuint _texture;
		int _width;
		int _height;
		unsigned int _num_textures;
		GLuint _texture_type;

		static GLuint cube_map_index_[6];
};

#endif /* TEXTURE_H */
