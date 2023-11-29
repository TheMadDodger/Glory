#include "Components.h"
#include <glm/ext/matrix_transform.hpp>

namespace Glory
{
	Transform::Transform()
		: Position(glm::vec3()), Rotation(glm::vec3()), Scale(glm::vec3(1.0f, 1.0f, 1.0f)),
		MatTransform(glm::identity<glm::mat4>())
	{
	}

	Transform::Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
		: Position(position), Rotation(rotation), Scale(scale),
		MatTransform(glm::translate(glm::identity<glm::mat4>(), position) * glm::inverse(glm::mat4_cast(glm::quat(rotation))) * glm::scale(glm::identity<glm::mat4>(), scale))
	{
	}
}
