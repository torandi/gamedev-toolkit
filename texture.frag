#version 330

uniform sampler2D tex;
uniform vec4 diffuse; 
uniform vec4 specular; 
uniform vec4 ambient; 
uniform vec4 emission; 
uniform float shininess; 

uniform vec3 camera_pos;

uniform float light_attenuation;
uniform vec4 light_intensity;
uniform vec4 ambient_intensity;

uniform vec3 light_pos;

in vec2 tex_coord;
in vec3 frag_normal;
in vec3 world_pos;

out vec4 outputColor;

float CalcAttenuation(in vec3 world_pos, out vec3 light_dir)
{
	vec3 lightDifference =  world_pos - light_pos;
	float lightDistanceSqr = dot(lightDifference, lightDifference);
	light_dir = lightDifference * inversesqrt(lightDistanceSqr);
	
	return (1 / ( 1.0 + light_attenuation * sqrt(lightDistanceSqr)));
}

void main() {
	vec3 light_dir = vec3(0.0);
	float atten = CalcAttenuation(world_pos, light_dir);
	vec4 attenIntensity = atten * light_intensity;

	vec3 surfaceNormal = normalize(frag_normal);
	float cosAngIncidence = dot(surfaceNormal, light_dir);
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);

	vec3 viewDirection = normalize(camera_pos-world_pos);
	vec3 halfAngle = normalize(light_dir + viewDirection);
	float angleNormalHalf = acos(dot(halfAngle, surfaceNormal));
	float exponent = angleNormalHalf / shininess;
	exponent = -(exponent * exponent);
	float gaussianTerm = exp(exponent);

	gaussianTerm = cosAngIncidence != 0.0 ? gaussianTerm : 0.0;
	
	outputColor = texture(tex, tex_coord)*((diffuse*attenIntensity*cosAngIncidence) + (specular * attenIntensity * gaussianTerm) + ( ambient*ambient_intensity));
}
