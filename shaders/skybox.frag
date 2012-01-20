#version 330
#include "uniforms.glsl"

in vec3 texcoord;
out vec4 ocolor;

void main() {
	vec4 tex_color = texture2DArray(tex_array1, texcoord.xyz);
	ocolor.rgb = tex_color.rgb*Lgt.ambient_intensity.rgb*(1.0f+tex_color.a);
}
