#ifndef MOVABLE_OBJECT_H
#define MOVABLE_OBJECT_H
	#include <glm/glm.hpp>
	#include <glm/gtc/quaternion.hpp>
	class MovableObject {
	private:
		mutable glm::mat4 rot_matrix_, trans_mat_, matrix_;
	protected:
		mutable bool rotation_matrix_dirty_, translation_matrix_dirty_, matrix_dirty_;
		glm::vec3 position_;
		glm::fquat orientation_;

		virtual const glm::mat4 rotation_matrix() const;
		virtual const glm::mat4 translation_matrix() const;

		glm::vec3 orient_vector(const glm::vec3 &vec) const;

	public:
		MovableObject();
		MovableObject(glm::vec3 position);

		virtual ~MovableObject();

		virtual const glm::vec3 &position() const { return position_; };
		virtual const glm::mat4 matrix() const;

		void relative_move(const glm::vec3 &move);
		void relative_rotate(const glm::vec3 &_axis, const float &angle);

		void absolute_rotate(const glm::vec3 &_axis, const float &angle);
		void absolute_move(const glm::vec3 &move);

	};
#endif
