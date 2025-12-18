#include "RenderHelpers.h"
#include "GraphicsDevice.h"

#include <glm/glm.hpp>

namespace Glory
{
	void FixViewport(glm::vec4& viewport, const glm::uvec2& resolution, GraphicsDevice* pDevice)
	{
		const ViewportOrigin origin = pDevice->GetViewportOrigin();
		switch (origin)
		{
		case Glory::TopLeft:
			/* Transform the Y origin */
			glm::vec4 temp = viewport;
			viewport.y = float(resolution.y) - temp.y - temp.w;
			break;
		case Glory::BottomLeft:
			/* No need to do anything */
			break;
		default:
			break;
		}
	}

	void InvertViewport(glm::vec4& viewport, GraphicsDevice* pDevice)
	{
		const ViewportOrigin origin = pDevice->GetViewportOrigin();
		switch (origin)
		{
		case Glory::TopLeft:
			/* Transform the Y origin */
			glm::vec4 temp = viewport;
			viewport.y = temp.w - temp.y;
			viewport.w = -temp.w;
			break;
		case Glory::BottomLeft:
			/* No need to do anything */
			break;
		default:
			break;
		}
	}

	void FixShadowCoords(glm::vec4& coords, GraphicsDevice* pDevice)
	{
		const ViewportOrigin origin = pDevice->GetViewportOrigin();
		switch (origin)
		{
		case Glory::TopLeft:
			/* Invert Y axis */
			coords.y = 1.0f - coords.y;
			coords.w = 1.0f - coords.w;
			//coords.y = temp.w;
			//coords.w = temp.y;
			break;
		case Glory::BottomLeft:
			/* No need to do anything */
			break;
		default:
			break;
		}
	}

	//const glm::vec4 temp = coords;
	//const float height = temp.w - temp.y;
	//coords.y = 1.0f - temp.y - temp.w;
	//coords.w = coords.y + height;

	void FixProjection(glm::mat4& projection, GraphicsDevice* pDevice)
	{
		const ViewportOrigin origin = pDevice->GetViewportOrigin();
		switch (origin)
		{
		case Glory::TopLeft:
			/* Flip Y */
			projection[1][1] *= -1.0f;
			break;
		case Glory::BottomLeft:
			/* No need to do anything */
			break;
		default:
			break;
		}
	}
}