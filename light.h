#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "movable_object.h"

#define HALF_LIGHT_DISTANCE 1.5f

class RenderObject;

class Light : public MovableObject {
public:
	enum light_type_t {
		DIRECTIONAL_LIGHT, //No attenuation
		POINT_LIGHT
	} light_type;

	glm::vec3 intensity;

	Light(glm::vec3 _intensity, light_type_t lt ) : MovableObject(), light_type(lt) , intensity(_intensity) { 
		shader_light_.attenuation = 1.f/pow(HALF_LIGHT_DISTANCE,2);
	};
	Light(glm::vec3 _intensity, glm::vec3 position, light_type_t lt) : MovableObject(position), light_type(lt) , intensity(_intensity) {
		shader_light_.attenuation = 1.f/pow(HALF_LIGHT_DISTANCE,2);
	}
	virtual ~Light() {};

	struct shader_light_t {
		float attenuation;
		float padding[3];
		glm::vec4 intensity;
		glm::vec4 position;
	};

private:
	mutable shader_light_t shader_light_;

public:
	const shader_light_t &shader_light() const;

	void set_half_light_distance(float hld);

	void set_id_in_render_object(RenderObject * ro, int id, bool set_colors=false) const;
};
#endif
