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
		CameraRef GetNewOrUnusedCamera();
		void SetUnused(Camera* pCamera);
		RenderTexture* GetRenderTextureForCamera(CameraRef camera, Engine* pEngine, bool createIfNotExist = true);
		Camera* GetCamera(UUID uuid);

	private:
		friend class GloryContext;
		CameraManager();
		virtual ~CameraManager();

	private:
		std::vector<Camera> m_Cameras;
		std::vector<size_t> m_UnusedCameraIndices;
		std::unordered_map<UUID, size_t> m_IDToCamera;
	};
}
