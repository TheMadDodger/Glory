#include "VulkanShader.h"
#include "VulkanGraphicsModule.h"
#include "VulkanDeviceManager.h"
#include "Device.h"
#include <Game.h>
#include "VulkanStructsConverter.h"

namespace Glory
{
	VulkanShader::VulkanShader(FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function) : m_pShaderFileData(pShaderFileData), m_ShaderType(shaderType), m_Function(function)
	{

	}

	VulkanShader::~VulkanShader()
	{
		VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
		VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
		Device* pDevice = pDeviceManager->GetSelectedDevice();
		LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

		deviceData.LogicalDevice.destroyShaderModule(m_ShaderModule);
	}

	void VulkanShader::Initialize()
	{
		VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
		VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
		Device* pDevice = pDeviceManager->GetSelectedDevice();
		LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

		vk::ShaderModuleCreateInfo shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(m_pShaderFileData->Size())
			.setPCode(reinterpret_cast<const uint32_t*>(m_pShaderFileData->Data()));
		m_ShaderModule = deviceData.LogicalDevice.createShaderModule(shaderModuleCreateInfo, nullptr);

		vk::ShaderStageFlagBits shaderFlag = VKConverter::GetShaderStageFlag(m_ShaderType);

		m_PipelineShaderStageInfo = vk::PipelineShaderStageCreateInfo()
			.setStage(shaderFlag)
			.setModule(m_ShaderModule)
			.setPName(m_Function.data());
	}
}
