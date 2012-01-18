#version 330
#include "uniforms.frag"

uniform sampler2D skymap;

in vec2 tex_coord;
out vec4 outputColor;

void main() {
	outputColor = texture(tex, tex_coord)*Lgt.ambient_intensity*(1.0f+texture(skymap, tex_coord));
}
