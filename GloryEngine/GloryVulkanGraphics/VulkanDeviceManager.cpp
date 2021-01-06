#include "VulkanDeviceManager.h"
#include <iostream>
#include "Device.h"

namespace Glory
{
    size_t VulkanDeviceManager::GetDeviceCount()
    {
        return m_Devices.size();
    }

    Device* VulkanDeviceManager::GetDevice(size_t index)
    {
        index = std::clamp(index, (size_t)0, m_Devices.size() - 1);
        return m_Devices[index];
    }

    Device* VulkanDeviceManager::GetSelectedDevice()
    {
        return m_pSelectedPhysicalDevice;
    }

    VulkanDeviceManager::VulkanDeviceManager() : m_pSelectedPhysicalDevice(nullptr)
	{
	}

	VulkanDeviceManager::~VulkanDeviceManager()
	{
        for (size_t i = 0; i < m_Devices.size(); i++)
        {
            delete m_Devices[i];
        }
        m_Devices.clear();
        m_SupportedDevices.clear();
	}

	void VulkanDeviceManager::Initialize(VulkanGraphicsModule* pGrahpicsModule)
	{
		// Get the physical devices
        VkInstance instance = pGrahpicsModule->GetCInstance();
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
        std::cout << "Available physical devices:" << deviceCount << std::endl;

        m_Devices.resize(deviceCount);

        int index = 0;
        for (const auto& device : physicalDevices)
        {
            Device* pDevice = new Device((vk::PhysicalDevice)device);
            pDevice->LoadData(pGrahpicsModule);
            m_Devices[index] = pDevice;
            std::cout << '\t' << index << ": " << device << std::endl;
            ++index;
        }
        std::cout << std::endl;

        if (physicalDevices.size() <= 0)
        {
            std::cerr << "No physical devices found!" << std::endl;
            return;
        }
	}

    void VulkanDeviceManager::CheckDeviceSupport(VulkanGraphicsModule* pGraphicsModule, std::vector<const char*> extensions)
    {
        m_SupportedDevices.clear();
        for (size_t i = 0; i < m_Devices.size(); i++)
        {
            if (!m_Devices[i]->CheckSupport(pGraphicsModule, extensions)) continue;
            m_SupportedDevices.push_back(m_Devices[i]);
        }
    }

    void VulkanDeviceManager::SelectDevice(size_t index)
    {
        m_pSelectedPhysicalDevice = GetDevice(index);
    }
}
