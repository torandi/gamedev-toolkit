#version 330
#include "uniforms.glsl"

in vec2 texcoord;
out vec4 ocolor;

void main() {
	ocolor = texture(tex1, texcoord)*Lgt.ambient_intensity*(1.0f+texture(tex2, texcoord));
}
