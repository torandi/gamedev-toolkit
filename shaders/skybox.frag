#version 330
#pragma include "uniforms.frag"

in vec2 tex_coord;
out vec4 outputColor;

void main() {
	outputColor = texture(tex, tex_coord);
}
