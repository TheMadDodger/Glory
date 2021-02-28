#pragma once
#include <FileData.h>
#include <GraphicsEnums.h>
#include <vulkan/vulkan.hpp>

namespace Glory
{
	class VulkanShader
	{
	public:
		VulkanShader(FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function);
		virtual ~VulkanShader();

	private:
		void Initialize();

	private:
		friend class VulkanGraphicsModule;
		FileData* m_pShaderFileData;
		const ShaderType m_ShaderType;
		const std::string m_Function;

		vk::ShaderModule m_ShaderModule;
		vk::PipelineShaderStageCreateInfo m_PipelineShaderStageInfo;
	};
}
