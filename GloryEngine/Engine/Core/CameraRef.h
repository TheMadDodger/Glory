#pragma once
#include "UUID.h"
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
		CameraRef(std::nullptr_t);

		bool operator==(const CameraRef& other) const;

	public:
		void SetBaseResolution(uint32_t width, uint32_t height);
		const glm::uvec2& GetBaseResolution() const;
		void SetResolutionScale(float width, float height);
		void SetPerspectiveProjection(float halfFOV, float near, float far);
		void SetOutput(bool output, int x, int y);
		bool IsOutput() const;
		void SetOrthographicProjection(float near, float far);
		void SetView(const glm::mat4& view);
		void SetPriority(int priority);
		void SetLayerMask(const LayerMask& layerMask);
		void SetClearColor(const glm::vec4& clearColor);
		void SetUserData(const std::string& name, void* data);
		bool IsResolutionDirty();
		bool IsPerspectiveDirty();
		void SetResolutionDirty(bool dirty=true);
		void SetPerspectiveDirty(bool dirty=true);

		const glm::uvec2& GetResolution() const;
		const glm::mat4& GetView() const;
		glm::mat4 GetViewInverse() const;
		const glm::mat4& GetProjection() const;
		glm::mat4 GetProjectionInverse() const;
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

		void Free();

	private:
		CameraRef(CameraManager* pManager, UUID uuid);

	private:
		friend class CameraManager;
		CameraManager* m_pManager;
		UUID m_CameraID;
	};
}
