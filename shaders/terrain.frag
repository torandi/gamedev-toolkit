#version 330
#include "uniforms.glsl"

const int num_height_levels = 5;

const float height_levels[num_height_levels] = {
	0.0,
	0.3,
	0.4,
	0.5,
	0.8
};


in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texcoord;
in float height;

#include "light_calculations.glsl"

out vec4 ocolor;

void main() {
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

	//Find level:
	int prev=num_height_levels-1;
	int next=num_height_levels;
	for(int i=1; i < num_height_levels; ++i) {
		if(height < height_levels[i]) {
			prev=i-1;
			next=i;
			break;
		}
	}

	if(next != num_height_levels) {
		vec4 c1, c2;
		c1 = texture2DArray(tex_array1, vec3(texcoord, prev));
		c2 = texture2DArray(tex_array1, vec3(texcoord, next));
		float m = (height-height_levels[prev])/(height_levels[next]-height_levels[prev]);
		originalColor = c1*(1-m)+c2*m;
		//originalColor = c1;
	} else {
		originalColor = texture2DArray(tex_array1,vec3(texcoord, num_height_levels-1));
	}

	/*originalColor = vec4(height);
	originalColor.a = 1.0f;*/

	vec3 normal_map = vec3(0.0, 0.0, 1.0);

	//normal_map = normalize(texture(tex2, texcoord).xyz * 2.0 - 1.0);
	
	vec4 accumLighting = originalColor * Lgt.ambient_intensity;

	for(int light = 0; uint(light) < Lgt.num_lights; ++light) {
		vec3 light_distance = Lgt.lights[light].position.xyz - position;
		vec3 dir = normalize(light_distance);
		vec3 light_dir;
		//Convert to tangent space
		light_dir.x = dot(dir, norm_tangent);
		light_dir.y = dot(dir, norm_bitangent);
		light_dir.z = dot(dir, norm_normal);

		accumLighting += computeLighting(Lgt.lights[light], originalColor, normal_map, light_dir, camera_dir, light_distance);
	//	accumLighting += computeLighting(Lgt.lights[light], originalColor, norm_normal, dir, camera_direction, light_distance);
	}

	ocolor= clamp(accumLighting,0.0, 1.0);

//	ocolor = vec4(height);
	
	ocolor.a = 1.f;
}
