#pragma once
#include <glm/glm.hpp>

namespace Glory
{
	struct Transform
	{
		Transform();
		Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);

		glm::vec3 Position;
		glm::vec3 Rotation;
		glm::vec3 Scale;

		glm::mat4 MatTransform;
	};

	struct Triangle
	{
		Triangle() {}
	};
}
