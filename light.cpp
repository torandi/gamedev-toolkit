#include "light.h"

#include "render_object.h"
#include <glm/glm.hpp>
#include <vector>

const Light::shader_light_t &Light::shader_light() const {
	shader_light_.intensity.x = intensity.x;
	shader_light_.intensity.y = intensity.y;
	shader_light_.intensity.z = intensity.z;
	shader_light_.position.x = position().x;
	shader_light_.position.y = position().y;
	shader_light_.position.z = position().z;
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


void Light::set_id_in_render_object(RenderObject * ro, int id, bool set_colors) const {
	for(std::vector<RenderObject::material_t>::iterator it=ro->materials.begin(); it!=ro->materials.end(); ++it) {
		if(set_colors) {
			it->attr.diffuse = glm::vec4(intensity,1.f);
			it->attr.ambient = glm::vec4(intensity,1.f);
			it->attr.specular = glm::vec4(intensity,1.f);
			it->attr.emission = glm::vec4(intensity,1.f);
		}
		it->attr.extra = id+1;
	}
}
