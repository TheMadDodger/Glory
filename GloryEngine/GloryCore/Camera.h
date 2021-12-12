#pragma once
#include <glm/glm.hpp>
#include "Object.h"
#include "RenderTexture.h"

namespace Glory
{
	class Camera : public Object
	{
	public:
		void SetResolution(int width, int height);
		void SetPerspectiveProjection(int width, int height, float halfFOV, float near, float far);
		void SetView(const glm::mat4& view);
		void SetDisplayIndex(int index);

		const glm::mat4& GetView() const;
		const glm::mat4& GetProjection() const;
		int GetDisplayIndex() const;
		RenderTexture* GetRenderTexture() const;

	private:
		Camera(int width, int height);

	private:
		friend class CameraManager;
		bool m_IsInUse;
		bool m_TextureIsDirty;

		glm::mat4 m_View;
		glm::mat4 m_Projection;
		glm::ivec2 m_Resolution;

		RenderTexture* m_pRenderTexture;

		int m_DisplayIndex;
	};
}
