#version 330
#include "uniforms.frag"

in vec2 tex_coord;
in vec3 frag_normal;
in vec3 world_pos; //my position in world space

#include "light_calculations.frag"

out vec4 outputColor;

void main() {
	vec4 originalColor; 
	vec3 surfaceNormal = normalize(frag_normal);

	uint light_id = Mtl.extra >> 1;

	if((Mtl.extra & 1) == 1) {
		originalColor = texture(tex, tex_coord);
	} else {
		originalColor = Mtl.diffuse;
	}
	vec4 accumLighting = originalColor * Lgt.ambient_intensity;

	for(uint light = 0; light < Lgt.num_lights; ++light) {
		if(light != light_id)
			accumLighting += computeLighting(Lgt.lights[light], originalColor, surfaceNormal);
		else
			accumLighting += originalColor;
	}
	outputColor = accumLighting;
}
