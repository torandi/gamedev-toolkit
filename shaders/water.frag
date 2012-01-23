#version 330
#include "uniforms.glsl"

const vec3 water_tint = vec3(0.8, 1.0, 1.0);

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texcoord;
in float height;

#include "skybox_color.glsl"
#include "light_calculations.glsl"

out vec4 ocolor;

void main() {
	vec4 specular=vec4(1.0);

	vec3 norm_normal, norm_tangent, norm_bitangent;
	norm_normal = normalize(normal);
	norm_tangent = normalize(tangent);
	norm_bitangent = normalize(bitangent);

	vec3 camera_direction = normalize(camera_pos - position);
	//Convert to tangent space:
	vec3 camera_dir;
	camera_dir.x = dot(camera_direction, norm_tangent); 
	camera_dir.y = dot(camera_direction, norm_bitangent); 
	camera_dir.z = dot(camera_direction, norm_normal); 

	vec3 normal_map = vec3(0.0, 0.0, 1.0);

	//normal_map = normalize(texture(tex2, texcoord).xyz * 2.0 - 1.0);

	vec3 r = reflect(-camera_direction, norm_normal);
	vec4 originalColor;
	originalColor.rgb = skybox_color(r)*water_tint;
	
	vec4 accumLighting = originalColor * Lgt.ambient_intensity;

	for(int light = 0; uint(light) < Lgt.num_lights; ++light) {
		vec3 light_distance = Lgt.lights[light].position.xyz - position;
		vec3 dir = normalize(light_distance);
		vec3 light_dir;
		//Convert to tangent space
		light_dir.x = dot(dir, norm_tangent);
		light_dir.y = dot(dir, norm_bitangent);
		light_dir.z = dot(dir, norm_normal);
		accumLighting += computeLighting(
				Lgt.lights[light], originalColor, normal_map,
				light_dir, camera_dir, light_distance, 
				64.0, specular, 1.0,
				true, true);
	}

	ocolor= clamp(accumLighting,0.0, 1.0);
	ocolor.a = 0.5;
	
}
