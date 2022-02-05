#pragma once
#include "UUID.h"
#include "RenderTexture.h"
#include "LayerMask.h"
#include <glm/glm.hpp>

namespace Glory
{
	struct CameraRef
	{
	public:
		CameraRef();

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
		void SetUserData(const std::string& name, void* data);

		const glm::uvec2& GetResolution() const;
		const glm::mat4& GetView() const;
		const glm::mat4& GetViewInverse() const;
		const glm::mat4& GetProjection() const;
		const glm::mat4& GetProjectionInverse() const;
		int GetDisplayIndex() const;
		int GetPriority() const;
		const glm::vec4& GetClearColor() const;
		const LayerMask& GetLayerMask() const;
		RenderTexture* GetRenderTexture() const;
		RenderTexture* GetOutputTexture() const;
		bool HasOutput() const;

		template<typename T>
		bool GetUserData(const std::string& name, T*& data)
		{
			Camera* pCamera = CameraManager::GetCamera(m_CameraID);
			if (pCamera == nullptr) return false;
			return pCamera->GetUserData(name, data);
		}

		float GetNear() const;
		float GetFar() const;

		void Free();

		bool operator<(const CameraRef& other) const;

	private:
		CameraRef(UUID uuid);

	private:
		friend class CameraManager;
		UUID m_CameraID;
	};
}
