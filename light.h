#ifndef LIGHT_H
#define LIGHT_H

#define GLM_SWIZZLE 
#include <glm/glm.hpp>
#include "movable_object.h"

class Light : public MovableObject {
public:
	enum light_type_t {
		DIRECTIONAL_LIGHT, //No attenuation
		POINT_LIGHT
	} light_type;

	glm::vec3 intensity;

	Light(glm::vec3 _intensity, light_type_t lt ) : MovableObject(), light_type(lt) , intensity(_intensity){};
	Light(glm::vec3 _intensity, glm::vec3 position, light_type_t lt) : MovableObject(position), light_type(lt) , intensity(_intensity) {};
	virtual ~Light() {};

	struct shader_light_t {
		glm::vec4 intensity;
		glm::vec4 position;
	};

private:
	mutable shader_light_t shader_light_;

public:
	const shader_light_t &shader_light() const;
};
#endif
