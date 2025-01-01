#pragma once
#include "CameraRef.h"

#include <unordered_map>

namespace Glory
{
	class Camera;
	class Engine;

	class CameraManager
	{
	public:
		virtual ~CameraManager();

		CameraRef GetNewOrUnusedCamera();
		void SetUnused(Camera* pCamera);
		RenderTexture* GetRenderTextureForCamera(CameraRef camera, Engine* pEngine, size_t index = 0, bool createIfNotExist = true);
		size_t CameraRenderTextureCount(CameraRef camera, Engine* pEngine);
		Camera* GetCamera(UUID uuid);
		void ResizeAllCameras(glm::uvec2 resolution);

	private:
		friend class Engine;
		void Cleanup();

		CameraManager(Engine* pEngine);

	private:
		Engine* m_pEngine;
		std::vector<Camera> m_Cameras;
		std::vector<size_t> m_UnusedCameraIndices;
		std::unordered_map<UUID, size_t> m_IDToCamera;
	};
}
