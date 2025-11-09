#pragma once
#include "GraphicsEnums.h"
#include "GraphicsDevice.h"

namespace Glory
{
	class GraphicsDevice;

	void CreateBufferDescriptorLayoutAndSet(GraphicsDevice* pDevice, bool usePushConstants, size_t numBuffers,
		const std::vector<uint32_t>& bindingIndices, const std::vector<BufferType>& bufferTypes,
		const std::vector<ShaderTypeFlag>& shaderStages, const std::vector<BufferHandle>& bufferHandles,
		const std::vector<std::pair<uint32_t, uint32_t>>& bufferOffsetsAndSizes, DescriptorSetLayoutHandle& layout,
		DescriptorSetHandle& set, BufferHandle* pConstantsBuffer=nullptr, ShaderTypeFlag constantsShaderStage=ShaderTypeFlag(0),
		uint32_t constantsOffset=0, uint32_t constantsSize=0);

	DescriptorSetLayoutHandle CreateBufferDescriptorLayout(GraphicsDevice* pDevice, size_t numBuffers,
		const std::vector<uint32_t>& bindingIndices, const std::vector<BufferType>& bufferTypes,
		const std::vector<ShaderTypeFlag>& shaderStages);

	DescriptorSetHandle CreateBufferDescriptorSet(GraphicsDevice* pDevice, bool usePushConstants, size_t numBuffers,
		const std::vector<BufferHandle>& bufferHandles, const std::vector<std::pair<uint32_t, uint32_t>>& bufferOffsetsAndSizes,
		DescriptorSetLayoutHandle layout, BufferHandle* pConstantsBuffer=nullptr, uint32_t constantsOffset=0, uint32_t constantsSize=0);

	void CreateSamplerDescriptorLayoutAndSet(GraphicsDevice* pDevice, size_t numSamplers,
		const std::vector<uint32_t>& bindingIndices, const std::vector<ShaderTypeFlag>& shaderStages,
		std::vector<std::string>&& samplerNames, const std::vector<TextureHandle>& textureHandles,
		DescriptorSetLayoutHandle& layout, DescriptorSetHandle& set);

	DescriptorSetLayoutHandle CreateSamplerDescriptorLayout(GraphicsDevice* pDevice, size_t numSamplers,
		const std::vector<uint32_t>& bindingIndices, const std::vector<ShaderTypeFlag>& shaderStages,
		std::vector<std::string>&& samplerNames);

	DescriptorSetHandle CreateSamplerDescriptorSet(GraphicsDevice* pDevice, size_t numSamplers,
		const std::vector<TextureHandle>& textureHandles, DescriptorSetLayoutHandle layout);
}