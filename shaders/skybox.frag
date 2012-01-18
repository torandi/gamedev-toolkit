#version 330
#include "uniforms.frag"

in vec2 tex_coord;
out vec4 outputColor;

void main() {
	outputColor = texture(tex, tex_coord)*Lgt.ambient_intensity*2.0;
}
