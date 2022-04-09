#pragma once
#include "Camera.h"
#include "Engine.h"
#include "CameraRef.h"
#include <unordered_map>

namespace Glory
{
	class CameraManager
	{
	public:
		static CameraRef GetNewOrUnusedCamera();
		static void SetUnused(Camera* pCamera);
		static RenderTexture* GetRenderTextureForCamera(CameraRef camera, Engine* pEngine, bool createIfNotExist = true);
		static Camera* GetCamera(UUID uuid);

	private:
		CameraManager();
		virtual ~CameraManager();

	private:
		static std::vector<Camera> m_Cameras;
		static std::vector<size_t> m_UnusedCameraIndices;
		static std::unordered_map<UUID, size_t> m_IDToCamera;
	};
}
