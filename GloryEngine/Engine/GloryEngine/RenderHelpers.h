#pragma once
#include <engine_visibility.h>

#include <glm/fwd.hpp>

namespace Glory
{
	class GraphicsDevice;

	GLORY_ENGINE_API void FixViewport(glm::vec4& viewport, const glm::uvec2& resolution, GraphicsDevice* pDevice);
	GLORY_ENGINE_API void InvertViewport(glm::vec4& viewport, GraphicsDevice* pDevice);
	GLORY_ENGINE_API void FixShadowCoords(glm::vec4& coords, GraphicsDevice* pDevice);
	GLORY_ENGINE_API void FixScreenCoords(glm::ivec2& coords, const glm::uvec2& dimensions, GraphicsDevice* pDevice);
	GLORY_ENGINE_API void FixProjection(glm::mat4& projection, GraphicsDevice* pDevice);
}