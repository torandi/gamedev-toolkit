#version 330
#include "uniforms.frag"

in vec2 tex_coord;
out vec4 outputColor;

void main() {
	outputColor = texture(tex1, tex_coord)*Lgt.ambient_intensity*(1.0f+texture(tex2, tex_coord));
}
