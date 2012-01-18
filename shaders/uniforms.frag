const int maxNumberOfLights = 4;

uniform sampler2D tex1;
uniform sampler2D tex2;

layout(std140) uniform Material {
	uint use_texture;
	uint use_normal_map;
	uint extra;
	float shininess; 
	vec4 diffuse; 
	vec4 specular; 
	vec4 ambient; 
	vec4 emission; 
} Mtl;

struct light_data {
	vec4 intensity;
	vec4 position;
};

layout(std140) uniform LightsData {
	uint num_lights;
	float light_attenuation;
	vec4 ambient_intensity;
	light_data lights[maxNumberOfLights];
} Lgt;

layout(std140) uniform Camera{
	vec3 camera_pos; //The cameras position in world space
};
