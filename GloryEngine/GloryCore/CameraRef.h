#pragma once
#include "UUID.h"
#include "RenderTexture.h"
#include <glm/glm.hpp>

namespace Glory
{
	struct CameraRef
	{
	public:
		CameraRef();

	public:
		void SetResolution(int width, int height);
		void SetPerspectiveProjection(int width, int height, float halfFOV, float near, float far);
		void SetView(const glm::mat4& view);
		void SetDisplayIndex(int index);
		void SetPriority(int priority);
		void SetClearColor(const glm::vec4& clearColor);

		const glm::mat4& GetView() const;
		const glm::mat4& GetProjection() const;
		int GetDisplayIndex() const;
		int GetPriority() const;
		const glm::vec4& GetClearColor() const;
		RenderTexture* GetRenderTexture() const;

		void Free();

		bool operator<(const CameraRef& other) const;

	private:
		CameraRef(UUID uuid);

	private:
		friend class CameraManager;
		UUID m_CameraID;
	};
}
