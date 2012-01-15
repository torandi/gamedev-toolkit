#version 330

uniform sampler2D tex;

in vec2 tex_coord;
out vec4 outputColor;

void main() {
	outputColor = texture(tex, tex_coord);
}
