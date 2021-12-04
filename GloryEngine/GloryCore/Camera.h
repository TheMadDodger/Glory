#pragma once
#include "Object.h"
#include <glm/glm.hpp>

namespace Glory
{
	struct CoreCamera
	{
		CoreCamera();

		glm::mat4 m_View;
		glm::mat4 m_Projection;
	};
}
