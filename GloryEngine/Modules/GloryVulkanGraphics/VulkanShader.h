#pragma once
#include <Shader.h>
#include <FileData.h>
#include <GraphicsEnums.h>

#include <vulkan/vulkan.hpp>

namespace Glory
{
	class VulkanShader : public Shader
	{
	public:
		VulkanShader(FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function);
		virtual ~VulkanShader();

	private:
		virtual void Initialize() override;

	private:
		friend class VulkanGraphicsModule;
		friend class VulkanPipeline;
		friend class DeferredPipelineTest;

		vk::ShaderModule m_ShaderModule;
		vk::PipelineShaderStageCreateInfo m_PipelineShaderStageInfo;
	};
}
