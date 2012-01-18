#version 330

const int maxNumberOfLights = 4;

uniform sampler2D tex;

layout(std140) uniform Matrices {
	mat4 projection_matrix;
	mat4 mvp;
};

layout(std140) uniform Material {
	uint extra; //Used to inform about light id in this shader
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

in vec2 tex_coord;
in vec3 frag_normal;
in vec3 world_pos; //my position in world space

out vec4 outputColor;

float calcAttenuation(in vec3 world_pos,in vec3 light_pos, out vec3 light_dir) {
	vec3 lightDifference =  light_pos - world_pos;
	float lightDistanceSqr = dot(lightDifference, lightDifference);
	light_dir = lightDifference * inversesqrt(lightDistanceSqr);
	
	return (1 / ( 1.0 + Lgt.light_attenuation * sqrt(lightDistanceSqr)));
}


vec4 computeLighting(in light_data light, in vec4 originalColor, in vec3 surfaceNormal) {
	vec3 light_dir = vec3(0.0);
	vec4 lightIntensity;
	//Turn off light attenuatin if w == 0.0
	if(light.position.w == 0.0) {
		light_dir = world_pos - vec3(light.position);
		lightIntensity = light.intensity;	
	} else { 
		float atten = calcAttenuation(world_pos, vec3(light.position), light_dir);
		lightIntensity =  atten * light.intensity;
	}

	float cosAngIncidence = dot(surfaceNormal, light_dir);
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);

	vec3 viewDirection = normalize(camera_pos-world_pos);
	vec3 halfAngle = normalize(light_dir + viewDirection);
	float angleNormalHalf = acos(dot(halfAngle, surfaceNormal));
	float exponent = angleNormalHalf / Mtl.shininess;
	exponent = -(exponent * exponent);
	float gaussianTerm = exp(exponent);

	gaussianTerm = cosAngIncidence != 0.0 ? gaussianTerm : 0.0;
	
	return (originalColor*lightIntensity*cosAngIncidence) + (Mtl.specular * lightIntensity * gaussianTerm);
}

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
