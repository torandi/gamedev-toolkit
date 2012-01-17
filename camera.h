#ifndef CAMERA_H
#define CAMERA_H
	#include <glm/glm.hpp>
	#include <glm/gtc/quaternion.hpp>
	class camera_t {
		glm::vec3 position_;
		glm::fquat orientation_;

		const glm::mat4 rotation_matrix() const;
		const glm::mat4 translation_matrix() const;

	public:
		camera_t() : orientation_(1.f, 0.f, 0.f,0.f) {};
		camera_t(glm::vec3 position) : position_(position), orientation_(1.f, 0.f, 0.f,0.f) {};

		const glm::vec3 &position() const { return position_; };
		const glm::vec3 look_at() const;
		const glm::vec3 up() const;
		const glm::mat4 matrix() const;

		void relative_move(const glm::vec3 &move);
		void relative_rotate(const glm::vec3 &_axis, const float &angle);
	};

	extern camera_t camera;
#endif
