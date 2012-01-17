#include "light.h"
#include <glm/glm.hpp>

const Light::shader_light_t &Light::shader_light() const {
	shader_light_.intensity.xyz = intensity.xyz;
	shader_light_.position.xyz = position().xyz;
	shader_light_.position.w = 1.0f;
	//the w component turns on and of light attenuation
	switch(light_type) {
		case DIRECTIONAL_LIGHT:
			shader_light_.position.w = 0.0;
			break;
		case POINT_LIGHT:
			shader_light_.position.w = 1.0;
			break;
	}
	return shader_light_;
}
