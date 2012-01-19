#version 330
#include "uniforms.glsl"


in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texcoord;

#include "light_calculations.glsl"

out vec4 ocolor;

void main() {

	//If the extra parameter is >= 1 it indicates we are rendering a light with id (extra-1)
	int my_light_id = Mtl.extra -1;

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

	vec4 originalColor; 
	vec3 normal_map = vec3(0.0, 0.0, 1.0);
	if(Mtl.use_normal_map==1) {
		normal_map = normalize(texture(tex2, texcoord).xyz * 2.0 - 1.0);
	}
	
	if(Mtl.use_texture == 1) {
		originalColor = texture(tex1, texcoord);
	} else {
		originalColor = Mtl.diffuse;
	}
	vec4 accumLighting = originalColor * Lgt.ambient_intensity;

	for(int light = 0; light < Lgt.num_lights; ++light) {
		if(light != my_light_id) {
			vec3 light_distance = Lgt.lights[light].position.xyz - position;
			vec3 dir = normalize(light_distance);
			vec3 light_dir;
			//Convert to tangent space
			light_dir.x = dot(dir, norm_tangent);
			light_dir.y = dot(dir, norm_bitangent);
			light_dir.z = dot(dir, norm_normal);

			accumLighting += computeLighting(Lgt.lights[light], originalColor, normal_map, light_dir, camera_dir, light_distance);
		} else {
			accumLighting += originalColor; //This is a light, add full color
		}
	}

	ocolor= clamp(accumLighting,0.0, 1.0);
}
