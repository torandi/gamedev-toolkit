const int maxNumberOfLights = 4;

uniform sampler2D tex;

layout(std140) uniform Material {
	uint extra; //Used to toggle texture in normal shader
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

uniform vec3 camera_pos; //The cameras position in world space
