#version 330
#pragma include "uniforms.frag"


in vec2 tex_coord;
in vec3 frag_normal;
in vec3 world_pos; //my position in world space

#pragma include "light_calculations.frag"

out vec4 outputColor;

void main() {
	vec4 originalColor; 
	vec3 surfaceNormal = normalize(frag_normal);
	if(Mtl.extra == 1) {
		originalColor = texture(tex, tex_coord);
	} else {
		originalColor = Mtl.diffuse;
	}
	vec4 accumLighting = originalColor * Lgt.ambient_intensity;

	for(uint light = 0; light < Lgt.num_lights; ++light) {
		accumLighting += computeLighting(Lgt.lights[light], originalColor, surfaceNormal);
	}
	outputColor = accumLighting;
}
