#pragma once
#include <glm/glm.hpp>
#include "Object.h"
#include "RenderTexture.h"
#include "LayerMask.h"

namespace Glory
{
	class Camera : public Object
	{
	public:
		void SetResolution(int width, int height);
		void SetPerspectiveProjection(int width, int height, float halfFOV, float near, float far);
		void SetView(const glm::mat4& view);
		void SetDisplayIndex(int index);
		void SetPriority(int priority);
		void SetLayerMask(const LayerMask& layerMask);
		void SetClearColor(const glm::vec4& clearColor);

		const glm::mat4& GetView() const;
		const glm::mat4& GetProjection() const;
		int GetDisplayIndex() const;
		int GetPriority() const;
		const glm::vec4& GetClearColor() const;
		const LayerMask& GetLayerMask() const;
		RenderTexture* GetRenderTexture() const;

	private:
		Camera(int width, int height);

	private:
		friend class CameraManager;
		bool m_IsInUse;
		bool m_TextureIsDirty;
		int m_DisplayIndex;
		int m_Priority;
		LayerMask m_LayerMask;

		glm::mat4 m_View;
		glm::mat4 m_Projection;
		glm::ivec2 m_Resolution;

		glm::vec4 m_ClearColor;

		RenderTexture* m_pRenderTexture;
	};
}
