#include "Components.h"
#include <glm/ext/matrix_transform.hpp>

namespace Glory
{
	Transform::Transform()
		: Position(glm::vec3()), Rotation(glm::vec3()), Scale(glm::vec3(1.0f, 1.0f, 1.0f)),
		MatTransform(glm::identity<glm::mat4>()), Parent(nullptr)
	{

	}

	Transform::Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
		: Position(position), Rotation(rotation), Scale(scale),
		Parent(nullptr)
	{
		//MatTransform = glm::scale(glm::identity<glm::mat4>(), scale);
		//MatTransform = glm::rotate(MatTransform, rotation);
		//MatTransform = glm::translate(MatTransform, position);
	}
}
