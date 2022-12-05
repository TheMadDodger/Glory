#pragma once
#include <vector>

namespace Glory
{
	class VulkanGraphicsModule;
	class Device;

	class VulkanDeviceManager
	{
	public:
		size_t GetDeviceCount();
		Device* GetDevice(size_t index);
		Device* GetSelectedDevice();

	private:
		VulkanDeviceManager();
		virtual ~VulkanDeviceManager();

	private:
		void Initialize(VulkanGraphicsModule* pGrahpicsModule);
		void CheckDeviceSupport(VulkanGraphicsModule* pGraphicsModule, std::vector<const char*> extensions);
		void SelectDevice(size_t index);

	private:
		friend class VulkanGraphicsModule;
		std::vector<Device*> m_Devices;
		std::vector<Device*> m_SupportedDevices;
		Device* m_pSelectedPhysicalDevice;
	};
}