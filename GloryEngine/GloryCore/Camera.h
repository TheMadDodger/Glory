#pragma once
#include <glm/glm.hpp>
#include "Object.h"
#include "RenderTexture.h"

namespace Glory
{
	class Camera : public Object
	{
	public:
		Camera();

		void SetPerspectiveProjection(int width, int height, float halfFOV, float near, float far);
		void SetView(const glm::mat4& view);

		const glm::mat4& GetView() const;
		const glm::mat4& GetProjection() const;
		RenderTexture* GetRenderTexture();

	private:
		friend class CameraManager;
		bool m_IsInUse;

		glm::mat4 m_View;
		glm::mat4 m_Projection;
		RenderTexture* m_pRenderTexture;
	};
}
