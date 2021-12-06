#pragma once
#include "Camera.h"
#include "Engine.h"
#include <unordered_map>

namespace Glory
{
	class CameraManager
	{
	public:
		static Camera* GetNewOrUnusedCamera();
		static void SetUnused(Camera* pCamera);
		static RenderTexture* GetRenderTextureForCamera(Camera* pCamera, Engine* pEngine);

	private:
		CameraManager();
		virtual ~CameraManager();

	private:
		static std::vector<Camera> m_Cameras;
		static std::vector<size_t> m_UnusedCameraIndices;
		static std::unordered_map<UUID, size_t> m_IDToCamera;
	};
}
