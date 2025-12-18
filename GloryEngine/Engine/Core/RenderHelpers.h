#pragma once

#include <glm/fwd.hpp>

namespace Glory
{
	class GraphicsDevice;

	void FixViewport(glm::vec4& viewport, const glm::uvec2& resolution, GraphicsDevice* pDevice);
	void InvertViewport(glm::vec4& viewport, GraphicsDevice* pDevice);
	void FixShadowCoords(glm::vec4& coords, GraphicsDevice* pDevice);

	//const glm::vec4 temp = coords;
	//const float height = temp.w - temp.y;
	//coords.y = 1.0f - temp.y - temp.w;
	//coords.w = coords.y + height;

	void FixProjection(glm::mat4& projection, GraphicsDevice* pDevice);
}