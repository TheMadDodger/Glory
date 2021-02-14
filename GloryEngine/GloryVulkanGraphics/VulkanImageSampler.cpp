#include "VulkanImageSampler.h"
#include "VulkanGraphicsModule.h"
#include "Device.h"
#include <Game.h>
#include "VulkanStructsConverter.h"

namespace Glory
{
    std::map<SamplerSettings, VulkanImageSampler*, SamplerSettingsComparer> VulkanImageSampler::m_pSamplers
        = std::map<SamplerSettings, VulkanImageSampler*, SamplerSettingsComparer>();
    std::vector<VulkanImageSampler*> VulkanImageSampler::m_pSamplersArray
        = std::vector<VulkanImageSampler*>();

    void VulkanImageSampler::Destroy()
    {
        for (size_t i = 0; i < m_pSamplersArray.size(); i++)
        {
            delete m_pSamplersArray[i];
        }

        m_pSamplersArray.clear();
        m_pSamplers.clear();
    }

    VulkanImageSampler* VulkanImageSampler::GetSampler(const SamplerSettings& settings)
    {
        auto it = m_pSamplers.find(settings);
        if (it != m_pSamplers.end())
        {
            auto pSampler = *it;
            return pSampler.second;
        }

        return CreateNewSampler(settings);
    }

    const vk::Sampler& VulkanImageSampler::GetSampler() const
    {
        return m_TextureSampler;
    }

    VulkanImageSampler::VulkanImageSampler() {}

    VulkanImageSampler::~VulkanImageSampler()
    {
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        deviceData.LogicalDevice.destroySampler(m_TextureSampler, nullptr);
    }

    VulkanImageSampler* VulkanImageSampler::CreateNewSampler(const SamplerSettings& settings)
    {
        // Create texture sampler
        auto samplerCreateInfo = VKConverter::GetVulkanSamplerInfo(settings);

        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        VulkanImageSampler* pImageSampler = new VulkanImageSampler();
        if (deviceData.LogicalDevice.createSampler(&samplerCreateInfo, nullptr, &pImageSampler->m_TextureSampler) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create texture sampler!");

        m_pSamplers[settings] = pImageSampler;
        m_pSamplersArray.push_back(pImageSampler);
        return pImageSampler;
    }

    bool SamplerSettingsComparer::operator()(const SamplerSettings& a, const SamplerSettings& b) const
    {
        if (a.AddressModeU != b.AddressModeU) return false;
        if (a.AddressModeV != b.AddressModeV) return false;
        if (a.AddressModeW != b.AddressModeW) return false;
        if (a.AnisotropyEnable != b.AnisotropyEnable) return false;
        if (a.MaxAnisotropy != b.MaxAnisotropy) return false;
        if (a.CompareEnable != b.CompareEnable) return false;
        if (a.CompareOp != b.CompareOp) return false;
        if (a.MagFilter != b.MagFilter) return false;
        if (a.MinFilter != b.MinFilter) return false;
        if (a.MinLOD != b.MinLOD) return false;
        if (a.MaxLOD != b.MaxLOD) return false;
        if (a.MipLODBias != b.MipLODBias) return false;
        if (a.MipmapMode != b.MipmapMode) return false;
        if (a.UnnormalizedCoordinates != b.UnnormalizedCoordinates) return false;
        return true;
    }
}
