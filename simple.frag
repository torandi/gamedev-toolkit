#version 330

out vec4 outputColor;

in vec4 vert_color;

void main() {
	outputColor = vert_color;
}
