#version 330
#include "uniforms.glsl"

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) in vec4 in_color;

out vec2 texcoord;
out vec4 color;

void main() {
	vec4 w_pos = modelMatrix * in_position;
	gl_Position = projectionViewMatrix *  w_pos;
	texcoord = in_texcoord;
	color = in_color;
/*
	vec4 originalColor = in_color;
	vec3 accumLighting = originalColor.rgb * Lgt.ambient_intensity.rgb;

	for(int light = 0; uint(light) < Lgt.num_lights; ++light) {
		vec3 light_distance = Lgt.lights[light].position.xyz - w_pos.xyz;
		vec3 lightIntensity;

		//Turn off light attenuation if w == 0.0
		if(Lgt.lights[light].position.w == 0.0) {
			lightIntensity = Lgt.lights[light].intensity.rgb;	
		} else { 
			float lightAttenuation = (1 / ( 1.0 + Lgt.lights[light].attenuation * length(light_distance)));
			lightIntensity =  lightAttenuation * Lgt.lights[light].intensity.rgb;
		}

		accumLighting += lightIntensity*originalColor.rgb;
	}
	color.rgb= clamp(accumLighting,0.0, 1.0);
	color.a = originalColor.a;*/
}

