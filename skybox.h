#ifndef SKYBOX_H
#define SKYBOX_H
#include <glload/gl_3_3.h>

const float skyboxData[] = {

	//Front
	0.5f, 0.5f, 0.5f,
	0.5f, -0.5f, 0.5f,
	-0.5f, 0.5f, 0.5f,

	0.5f, -0.5f, 0.5f,
	-0.5f, -0.5f, 0.5f, 
	-0.5f, 0.5f, 0.5f, 

	//Back
	0.5f, 0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f, 

	0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f,

	//Left
	-0.5f, 0.5f, 0.5f, 
	-0.5f, 0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,

	-0.5f, 0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,

	//Right
	0.5f, 0.5f, 0.5f, 
	0.5f, 0.5f, -0.5f,
	0.5f, -0.5f, 0.5f,

	0.5f, 0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, 0.5f,

	//Top
	0.5f, 0.5f, 0.5f,
	0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, 0.5f,

	0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, 0.5f, 

	//Bottom
	0.5f, -0.5f, 0.5f,
	0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,

	0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,

	//UV Coords

	//Front
	1.f, 1.f, 0.f,
	1.f, 0.f, 0.f,
	0.f, 1.f, 0.f,
	
	1.f, 0.f, 0.f ,
	0.f, 0.f, 0.f ,
	0.f, 1.f, 0.f,

	//Back
	0.f, 1.f, 1.f,
	0.f, 0.f, 1.f,
	1.f, 1.f, 1.f,
	
	0.f, 0.0f,  1.f,
	1.f, 0.f,  1.f,
	1.f, 1.f, 1.f,

	//Left
	0.f, 1.f, 2.f,
	1.f, 1.f,  2.f,
	0.f, 0.f, 2.f,
	
	1.f, 1.f,  2.f,
	1.f, 0.f,  2.f,
	0.f, 0.f, 2.f,

	//Right
	1.f, 1.f, 3.f,
	0.f, 1.f,  3.f,
	1.f, 0.f, 3.f,
	
	0.f, 1.f,  3.f,
	0.f, 0.f,  3.f,
	1.f, 0.f, 3.f,

	//Top
	1.f, 1.f, 4.f,
	1.f, 0.f,  4.f,
	0.f, 1.f, 4.f,
	
	1.f, 0.f,  4.f,
	0.f, 0.f,  4.f,
	0.f, 1.f, 4.f,

	//Bottom
	1.f, 0.f, 5.f,
	1.f, 1.f,  5.f,
	0.f, 0.f, 5.f,
	
	1.f, 1.f,  5.f,
	0.f, 1.f,  5.f,
	0.f, 0.f, 5.f


};

const char * skybox_texture_name[] {
	"left_alpha.png",
	"right_alpha.png",
	"top_alpha.png",
	"bottom_alpha.png",
	"front_alpha.png",
	"back_alpha.png"
};

#endif
