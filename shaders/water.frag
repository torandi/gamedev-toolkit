#version 330
#include "uniforms.glsl"

const vec3 water_tint = vec3(0.8, 1.0, 1.0);

in vec3 position;
in vec3 normal;
in float depth;

#include "skybox_color.glsl"
#include "light_calculations.glsl"

out vec4 ocolor;

void main() {
	vec4 specular=vec4(1.0);

	vec3 norm_normal;
	norm_normal = normalize(normal);

	vec3 camera_dir = normalize(camera_pos - position);

	vec3 r = reflect(-camera_dir, norm_normal);
	vec4 originalColor;
	originalColor.rgb = skybox_color(r)*water_tint;
	
	vec4 accumLighting = originalColor *2.0* Lgt.ambient_intensity;

	for(int light = 0; uint(light) < Lgt.num_lights; ++light) {
		vec3 light_distance = Lgt.lights[light].position.xyz - position;
		vec3 light_dir = normalize(light_distance);
		accumLighting += computeLighting(
				Lgt.lights[light], originalColor, norm_normal,
				light_dir, camera_dir, light_distance, 
				64.0, specular, 1.0,
				true, true);
	}

	ocolor= clamp(accumLighting,0.0, 1.0);
	ocolor.a = 0.5+0.4*(depth);
}
