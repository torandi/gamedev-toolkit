#version 330
#include "uniforms.glsl"

in vec2 texcoord;
in vec4 color;

out vec4 ocolor;

void main() {
	vec4 tex_color = texture(tex1, texcoord);
	vec4 originalColor = tex_color*color;

	ocolor = originalColor;
}
