#include "VulkanShader.h"
#include "VulkanGraphicsModule.h"
#include "VulkanDeviceManager.h"
#include "Device.h"
#include "VulkanStructsConverter.h"

#include <Engine.h>

namespace Glory
{
	VulkanShader::VulkanShader(FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function) : Shader(pShaderFileData, shaderType, function)
	{

	}

	VulkanShader::~VulkanShader()
	{
		VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
		VulkanDeviceManager& deviceManager = pGraphics->GetDeviceManager();
		Device* pDevice = deviceManager.GetSelectedDevice();
		LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

		deviceData.LogicalDevice.destroyShaderModule(m_ShaderModule);
	}

	void VulkanShader::Initialize()
	{
		VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
		VulkanDeviceManager& deviceManager = pGraphics->GetDeviceManager();
		Device* pDevice = deviceManager.GetSelectedDevice();
		LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

		vk::ShaderModuleCreateInfo shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(m_CompiledShader.size())
			.setPCode(reinterpret_cast<const uint32_t*>(m_CompiledShader.data()));
		m_ShaderModule = deviceData.LogicalDevice.createShaderModule(shaderModuleCreateInfo, nullptr);

		vk::ShaderStageFlagBits shaderFlag = VKConverter::GetShaderStageFlag(m_ShaderType);

		m_PipelineShaderStageInfo = vk::PipelineShaderStageCreateInfo()
			.setStage(shaderFlag)
			.setModule(m_ShaderModule)
			.setPName(m_Function.data());
	}
}
