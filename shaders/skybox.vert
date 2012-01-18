#version 330
#pragma include "common.vert"

layout(std140) uniform Matrices {
	mat4 projectionViewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
};

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 texCoord;

out vec2 tex_coord;

void main() {
	gl_Position = projectionViewMatrix * position;
	tex_coord = texCoord;
}

