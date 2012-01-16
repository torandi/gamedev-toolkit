#version 330

uniform sampler2D tex;
uniform vec4 diffuse; 
uniform vec4 specular; 
uniform vec4 ambient; 
uniform vec4 emission; 
uniform float shininess; 

in vec2 tex_coord;
out vec4 outputColor;

void main() {
	outputColor = texture(tex, tex_coord)+ambient+diffuse;
}
