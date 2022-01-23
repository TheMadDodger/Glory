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
		void SetResolution(size_t width, size_t height);
		void SetPerspectiveProjection(size_t width, size_t height, float halfFOV, float near, float far);
		void SetView(const glm::mat4& view);
		void SetDisplayIndex(int index);
		void SetPriority(int priority);
		void SetLayerMask(const LayerMask& layerMask);
		void SetClearColor(const glm::vec4& clearColor);
		void SetOutputTexture(RenderTexture* pTexture);
		void EnableOutput(bool enable);

		const glm::uvec2& GetResolution() const;
		const glm::mat4& GetView() const;
		const glm::mat4& GetProjection() const;
		int GetDisplayIndex() const;
		int GetPriority() const;
		const glm::vec4& GetClearColor() const;
		const LayerMask& GetLayerMask() const;
		RenderTexture* GetRenderTexture() const;
		RenderTexture* GetOutputTexture() const;
		bool HasOutput() const;

		float GetNear() const;
		float GetFar() const;

	private:
		Camera(size_t width, size_t height);

	private:
		friend class CameraManager;
		bool m_IsInUse;
		bool m_TextureIsDirty;
		int m_DisplayIndex;
		int m_Priority;
		float m_Near;
		float m_Far;
		LayerMask m_LayerMask;

		glm::mat4 m_View;
		glm::mat4 m_Projection;
		glm::uvec2 m_Resolution;

		glm::vec4 m_ClearColor;

		RenderTexture* m_pRenderTexture;
		RenderTexture* m_pOutputTexture;
		bool m_OutputEnabled;
	};
}
