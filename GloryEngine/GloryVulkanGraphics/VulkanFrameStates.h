#pragma once
#include <FrameStates.h>
#include "VulkanGraphicsModule.h"

namespace Glory
{
	class VulkanFrameStates : public FrameStates
	{
	public:
		VulkanFrameStates(VulkanGraphicsModule* pModule);
		virtual ~VulkanFrameStates();

		static size_t GetCurrentImageIndex();

		static const size_t MAX_FRAMES_IN_FLIGHT;

	private:
		virtual void Initialize() override;
		virtual void OnFrameStart() override;
		virtual void OnFrameEnd() override;

	private:
		VulkanGraphicsModule* m_pModule;

		static size_t m_CurrentFrame;
		static size_t m_CurrentImageIndex;

		std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
		std::vector<vk::Semaphore> m_RenderFinishedSemaphores;
		std::vector<vk::Fence> m_InFlightFences;
		std::vector<vk::Fence> m_ImagesInFlight;
	};
}
