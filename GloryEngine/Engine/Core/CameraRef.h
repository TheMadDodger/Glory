#pragma once
#include "UUID.h"
#include "RenderTexture.h"
#include "LayerMask.h"

#include <glm/glm.hpp>

namespace Glory
{
	class RenderTexture;
	class CameraManager;

	struct CameraRef
	{
	public:
		CameraRef();

	public:
		void SetResolution(uint32_t width, uint32_t height);
		void SetPerspectiveProjection(uint32_t width, uint32_t height, float halfFOV, float near, float far);
		void SetOrthographicProjection(float width, float height, float near, float far);
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
		glm::mat4 GetViewInverse() const;
		const glm::mat4& GetProjection() const;
		glm::mat4 GetProjectionInverse() const;
		glm::mat4 GetFinalView() const;

		float* GetViewPointer();
		float* GetProjectionPointer();

		int GetDisplayIndex() const;
		int GetPriority() const;
		const glm::vec4& GetClearColor() const;
		const LayerMask& GetLayerMask() const;
		size_t GetRenderTextureCount() const;
		RenderTexture* GetRenderTexture(size_t index) const;
		RenderTexture* GetOutputTexture() const;
		bool HasOutput() const;

		template<typename T>
		bool GetUserData(const std::string& name, T*& data)
		{
			T* userData = (T*)GetUserDataVoid(name);
			if (!userData) return false;
			data = userData;
			return true;
		}

		void* GetUserDataVoid(const std::string& name);

		float GetNear() const;
		float GetFar() const;
		UUID GetUUID() const;

		void Free();

	private:
		CameraRef(CameraManager* pManager, UUID uuid);

	private:
		friend class CameraManager;
		CameraManager* m_pManager;
		UUID m_CameraID;
	};
}
