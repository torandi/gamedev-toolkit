#version 330

uniform mat4 mvp;
uniform mat4 projection_matrix;

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

out vec2 tex_coord;
out vec3 frag_normal;
out vec3 world_pos;

void main() {
	vec4 w_pos = mvp * position;
	world_pos = vec3(w_pos);
	gl_Position = projection_matrix *  w_pos;
	tex_coord = texCoord;
	frag_normal = normal;
}

