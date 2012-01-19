#version 330
#include "uniforms.vert"

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec4 normal;
layout (location = 3) in vec4 tangent;
layout (location = 3) in vec4 binormal;

out vec2 tex_coord;
out vec3 frag_normal;
out vec3 world_pos;

void main() {
	vec4 w_pos = modelMatrix * position;
	world_pos = w_pos.xyz;
	gl_Position = projectionViewMatrix *  w_pos;
	tex_coord = texCoord;
	frag_normal = (normalMatrix * normal).xyz;
}

