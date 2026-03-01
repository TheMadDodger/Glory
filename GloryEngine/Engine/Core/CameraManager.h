#pragma once
#include "CameraRef.h"

#include <unordered_map>

namespace Glory
{
	class Camera;
	class IEngine;

	class CameraManager
	{
	public:
		CameraManager(IEngine* pEngine);
		virtual ~CameraManager();

		CameraRef GetNewOrUnusedCamera();
		void SetUnused(Camera* pCamera);
		Camera* GetCamera(UUID uuid);

	private:
		friend class IEngine;
		void Cleanup();

	private:
		IEngine* m_pEngine;
		std::vector<Camera> m_Cameras;
		std::vector<size_t> m_UnusedCameraIndices;
		std::unordered_map<UUID, size_t> m_IDToCamera;
	};
}
