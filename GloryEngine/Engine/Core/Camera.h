#pragma once
#include <glm/glm.hpp>
#include <unordered_map>

#include "CameraRef.h"
#include "RenderTexture.h"
#include "LayerMask.h"

namespace Glory
{
	class Camera
	{
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
		void SetSecondaryOutputTexture(RenderTexture* pTexture);
		void Swap();
		void SetUserData(const std::string& name, void* data);

		const glm::uvec2& GetResolution() const;
		const glm::mat4& GetView() const;
		const glm::mat4& GetViewOffset() const;
		const glm::mat4& GetProjection() const;
		glm::mat4 GetFinalView() const;

		float* GetViewPointer();
		float* GetProjectionPointer();

		int GetDisplayIndex() const;
		int GetPriority() const;
		const glm::vec4& GetClearColor() const;
		const LayerMask& GetLayerMask() const;
		size_t RenderTextureCount() const;
		RenderTexture* GetRenderTexture(size_t index=0) const;
		RenderTexture* GetOutputTexture() const;
		RenderTexture* GetSecondaryOutputTexture() const;
		template<typename T>
		bool GetUserData(const std::string& name, T*& data)
		{
			if (m_UserDatas.find(name) == m_UserDatas.end()) return false;
			data = (T*)m_UserDatas[name];
			return true;
		}

		uint64_t& GetUserHandle(const std::string& name);

		float GetNear() const;
		float GetFar() const;
		UUID GetUUID() const;

	private:
		Camera(uint32_t width, uint32_t height);

	private:
		friend class CameraManager;
		bool m_IsInUse;
		bool m_TextureIsDirty;
		bool m_IsOrtho;
		int m_DisplayIndex;
		int m_Priority;
		float m_Near;
		float m_Far;
		float m_HalfFOV;
		LayerMask m_LayerMask;

		glm::mat4 m_View;
		glm::mat4 m_ViewOffset;
		glm::mat4 m_Projection;
		glm::uvec2 m_Resolution;

		glm::vec4 m_ClearColor;

		std::vector<RenderTexture*> m_pRenderTextures;
		RenderTexture* m_pOutputTexture;
		RenderTexture* m_pSecondaryOutputTexture;

		bool m_PerspectiveDirty;

		std::unordered_map<std::string, void*> m_UserDatas;
		std::unordered_map<std::string, uint64_t> m_UserHandles;

		UUID m_UUID;
	};
}
