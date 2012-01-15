#version 330

uniform mat4 mvp;

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 texCoord;

out vec2 tex_coord;

void main() {
	gl_Position = mvp * position;
	tex_coord = texCoord;
}

