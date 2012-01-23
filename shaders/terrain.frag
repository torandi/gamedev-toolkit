#version 330
#include "uniforms.glsl"

uniform sampler2DArray specular_map;

const int num_height_levels = 5;

const float height_levels[num_height_levels] = {
	0.0,
	0.1,
	0.2,
	0.5,
	0.8
};

const float height_shininess[num_height_levels] = {
	0.0,
	2.0,
	0.0,
	0.0,
	5.0
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
	vec4 specular_color = vec4(1.0);
	float shininess = 1.0;

	//Find level:
	int prev=num_height_levels-1;
	int next=num_height_levels-1;
	for(int i=1; i < num_height_levels; ++i) {
		if(height < height_levels[i]) {
			prev=i-1;
			next=i;
			break;
		}
	}

	vec4 normal_map = vec4(0.0, 0.0, 1.0,1.0);

	vec4 c1, c2, s1, s2, n1, n2;
	float sh1, sh2;
	c1 = texture2DArray(tex_array1, vec3(texcoord, prev));
	c2 = texture2DArray(tex_array1, vec3(texcoord, next));
	s1 = texture2DArray(specular_map, vec3(texcoord, prev));
	s2 = texture2DArray(specular_map, vec3(texcoord, next));
	n1 = texture2DArray(tex_array2, vec3(texcoord, prev));
	n2 = texture2DArray(tex_array2, vec3(texcoord, next));
	sh1 = height_shininess[prev];
	sh2 = height_shininess[next];
	float m = (height-height_levels[prev])/(height_levels[next]-height_levels[prev]);
	if(next==prev)
		m=1.0;
	originalColor = c1*(1-m)+c2*m;
	specular_color = s1*(1-m)+s2*m;
	shininess = sh1*(1-m)+sh2*m;
	normal_map = n1*(1-m)+n2*m;

	normal_map.xyz = normalize(normal_map.xyz * 2.0 - 1.0);
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
				Lgt.lights[light], originalColor, normal_map.xyz,
				light_dir, camera_dir, light_distance, 
				shininess, specular_color, 0.5,
				true, true);
	}

	ocolor= clamp(accumLighting,0.0, 1.0);

//	ocolor = vec4(height);
	
	ocolor.a = 1.f;
}
