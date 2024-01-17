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
		RenderTexture* GetRenderTextureForCamera(CameraRef camera, Engine* pEngine, bool createIfNotExist = true);
		Camera* GetCamera(UUID uuid);

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
