#pragma once
#include "CameraRef.h"
#include "LayerMask.h"

#include <glm/glm.hpp>
#include <unordered_map>

namespace Glory
{
	class Camera
	{
	public:
		bool SetBaseResolution(uint32_t width, uint32_t height);
		const glm::uvec2& GetBaseResolution() const;
		bool SetResolutionScale(float width, float height);
		void SetPerspectiveProjection(float halfFOV, float near, float far, bool force=false);
		void SetOutput(bool output, int x, int y);
		bool IsOutput() const;
		void SetOrthographicProjection(float near, float far, bool force=false);
		void SetView(const glm::mat4& view);
		void SetPriority(int priority);
		void SetLayerMask(const LayerMask& layerMask);
		void SetClearColor(const glm::vec4& clearColor);
		void SetUserData(const std::string& name, void* data);
		bool IsResolutionDirty();
		bool IsPerspectiveDirty();
		void SetResolutionDirty(bool dirty=true);
		void SetPerspectiveDirty(bool dirty=true);
		void Focus(const BoundingSphere& boundingSphere);

		const glm::uvec2& GetResolution() const;
		const glm::mat4& GetView() const;
		const glm::mat4& GetViewOffset() const;
		const glm::mat4& GetProjection() const;
		glm::mat4 GetFinalView() const;

		float* GetViewPointer();
		float* GetProjectionPointer();

		int GetPriority() const;
		const glm::vec4& GetClearColor() const;
		const LayerMask& GetLayerMask() const;

		uint64_t& GetUserHandle(const std::string& name);

		float GetNear() const;
		float GetFar() const;
		UUID GetUUID() const;

	private:
		Camera(uint32_t width, uint32_t height);

	private:
		friend class CameraManager;
		bool m_IsInUse;
		bool m_IsOrtho;
		bool m_Output;
		bool m_ResolutionDirty;
		bool m_PerspectiveDirty;
		int m_Priority;
		float m_Near;
		float m_Far;
		float m_HalfFOV;
		LayerMask m_LayerMask;

		glm::mat4 m_View;
		glm::mat4 m_ViewOffset;
		glm::mat4 m_Projection;
		glm::uvec2 m_BaseResolution;
		glm::vec2 m_ResolutionScale;
		glm::uvec2 m_Resolution;
		glm::ivec2 m_OutputOffset;

		glm::vec4 m_ClearColor;

		std::unordered_map<std::string, void*> m_UserDatas;
		std::unordered_map<std::string, uint64_t> m_UserHandles;

		UUID m_UUID;
	};
}
