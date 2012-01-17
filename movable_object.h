#ifndef MOVABLE_OBJECT_H
#define MOVABLE_OBJECT_H
	#include <glm/glm.hpp>
	#include <glm/gtc/quaternion.hpp>
	class MovableObject {
	protected:
		glm::vec3 position_;
		glm::fquat orientation_;

		virtual const glm::mat4 rotation_matrix() const;
		virtual const glm::mat4 translation_matrix() const;

		glm::vec3 orient_vector(const glm::vec3 &vec) const;

	public:
		MovableObject() : orientation_(1.f, 0.f, 0.f,0.f) {};
		MovableObject(glm::vec3 position) : position_(position), orientation_(1.f, 0.f, 0.f,0.f) {};
		virtual ~MovableObject();

		const glm::vec3 &position() const { return position_; };
		virtual const glm::mat4 matrix() const;

		void relative_move(const glm::vec3 &move);
		void relative_rotate(const glm::vec3 &_axis, const float &angle);

		void absolute_rotate(const glm::vec3 &_axis, const float &angle);
		void absolute_move(const glm::vec3 &move);

	};
#endif
