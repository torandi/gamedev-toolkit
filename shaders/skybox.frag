#version 330
#include "uniforms.glsl"

in vec3 texcoord;
out vec4 ocolor;

void main() {
	ocolor = texture2DArray(tex_array1, texcoord.xyz)*Lgt.ambient_intensity;//*(1.0f+texture(tex2, texcoord));
}
