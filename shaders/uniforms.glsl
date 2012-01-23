#extension GL_EXT_gpu_shader4 : enable

const int maxNumberOfLights = 4;
const uint true_uint = uint(1);

uniform sampler2D tex1;
uniform sampler2D tex2;

uniform sampler2DArray tex_array1;
uniform sampler2DArray tex_array2;

uniform samplerCube skybox;

layout(std140) uniform Matrices {
	mat4 projectionViewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
};

layout(std140) uniform Camera {
	vec3 camera_pos; //The cameras position in world space
};


layout(std140) uniform Material {
	uint use_texture;
	uint use_normal_map;
	int extra;
	float shininess; 
	vec4 diffuse; 
	vec4 specular; 
	vec4 ambient; 
	vec4 emission; 
} Mtl;

struct light_data {
	float attenuation;
	vec4 intensity;
	vec4 position;
};

layout(std140) uniform LightsData {
	uint num_lights;
	vec4 ambient_intensity;
	light_data lights[maxNumberOfLights];
} Lgt;

