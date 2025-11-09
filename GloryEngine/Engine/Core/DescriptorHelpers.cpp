#include "DescriptorHelpers.h"
#include "GraphicsDevice.h"

namespace Glory
{
	void CreateBufferDescriptorLayoutAndSet(GraphicsDevice* pDevice, bool usePushConstants, size_t numBuffers,
		const std::vector<uint32_t>& bindingIndices, const std::vector<BufferType>& bufferTypes,
		const std::vector<ShaderTypeFlag>& shaderStages, const std::vector<BufferHandle>& bufferHandles,
		const std::vector<std::pair<uint32_t, uint32_t>>& bufferOffsetsAndSizes, DescriptorSetLayoutHandle& layout,
		DescriptorSetHandle& set, BufferHandle* pConstantsBuffer, ShaderTypeFlag constantsShaderStage,
		uint32_t constantsOffset, uint32_t constantsSize)
	{
		DescriptorSetLayoutInfo setLayoutInfo = DescriptorSetLayoutInfo();
		DescriptorSetInfo setInfo = DescriptorSetInfo();
		size_t firstBufferindex = 0;
		if (pConstantsBuffer)
		{
			if (!usePushConstants)
			{
				firstBufferindex = 1;
				setLayoutInfo.m_Buffers.resize(numBuffers + 1);
				setInfo.m_Buffers.resize(numBuffers + 1);
				if (!(*pConstantsBuffer))
					*pConstantsBuffer = pDevice->CreateBuffer(constantsSize, BufferType::BT_Uniform, BF_Write);
				setInfo.m_Buffers[0].m_BufferHandle = *pConstantsBuffer;
				setInfo.m_Buffers[0].m_Offset = constantsOffset;
				setInfo.m_Buffers[0].m_Size = constantsSize;
				setLayoutInfo.m_Buffers[0].m_BindingIndex = 0;
				setLayoutInfo.m_Buffers[0].m_Type = BufferType::BT_Uniform;
				setLayoutInfo.m_Buffers[0].m_ShaderStages = constantsShaderStage;
			}
			else
			{
				setLayoutInfo.m_Buffers.resize(numBuffers);
				setInfo.m_Buffers.resize(numBuffers);
				setLayoutInfo.m_PushConstantRange.m_Offset = constantsOffset;
				setLayoutInfo.m_PushConstantRange.m_Size = constantsSize;
				setLayoutInfo.m_PushConstantRange.m_ShaderStages = constantsShaderStage;
			}
		}
		else
		{
			setLayoutInfo.m_Buffers.resize(numBuffers);
			setInfo.m_Buffers.resize(numBuffers);
		}

		for (size_t i = 0; i < numBuffers; ++i)
		{
			const uint32_t bindingIndex = i >= bindingIndices.size() ? bindingIndices.back() : bindingIndices[i];
			const BufferType bufferType = i >= bufferTypes.size() ? bufferTypes.back() : bufferTypes[i];
			const ShaderTypeFlag shaderStage = i >= shaderStages.size() ? shaderStages.back() : shaderStages[i];

			const size_t index = firstBufferindex + i;
			setLayoutInfo.m_Buffers[index].m_BindingIndex = bindingIndex;
			setLayoutInfo.m_Buffers[index].m_Type = bufferType;
			setLayoutInfo.m_Buffers[index].m_ShaderStages = shaderStage;
			setInfo.m_Buffers[index].m_BufferHandle = bufferHandles[i];
			setInfo.m_Buffers[index].m_Offset = bufferOffsetsAndSizes[i].first;
			setInfo.m_Buffers[index].m_Size = bufferOffsetsAndSizes[i].second;
		}
		layout = setInfo.m_Layout = pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
		if (usePushConstants && numBuffers == 0) return;
		set = pDevice->CreateDescriptorSet(std::move(setInfo));
	}

	DescriptorSetLayoutHandle CreateBufferDescriptorLayout(GraphicsDevice* pDevice, size_t numBuffers,
		const std::vector<uint32_t>& bindingIndices, const std::vector<BufferType>& bufferTypes,
		const std::vector<ShaderTypeFlag>& shaderStages)
	{
		DescriptorSetLayoutInfo setLayoutInfo = DescriptorSetLayoutInfo();
		setLayoutInfo.m_Buffers.resize(numBuffers);

		for (size_t i = 0; i < numBuffers; ++i)
		{
			const uint32_t bindingIndex = i >= bindingIndices.size() ? bindingIndices.back() : bindingIndices[i];
			const BufferType bufferType = i >= bufferTypes.size() ? bufferTypes.back() : bufferTypes[i];
			const ShaderTypeFlag shaderStage = i >= shaderStages.size() ? shaderStages.back() : shaderStages[i];
			setLayoutInfo.m_Buffers[i].m_BindingIndex = bindingIndex;
			setLayoutInfo.m_Buffers[i].m_Type = bufferType;
			setLayoutInfo.m_Buffers[i].m_ShaderStages = shaderStage;
		}
		return pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
	}

	DescriptorSetHandle CreateBufferDescriptorSet(GraphicsDevice* pDevice, bool usePushConstants, size_t numBuffers,
		const std::vector<BufferHandle>& bufferHandles, const std::vector<std::pair<uint32_t, uint32_t>>& bufferOffsetsAndSizes,
		DescriptorSetLayoutHandle layout, BufferHandle* pConstantsBuffer, uint32_t constantsOffset, uint32_t constantsSize)
	{
		DescriptorSetInfo setInfo = DescriptorSetInfo();
		size_t firstBufferindex = 0;
		if (pConstantsBuffer)
		{
			if (!usePushConstants)
			{
				firstBufferindex = 1;
				setInfo.m_Buffers.resize(numBuffers + 1);
				if (!(*pConstantsBuffer))
					*pConstantsBuffer = pDevice->CreateBuffer(constantsSize, BufferType::BT_Uniform, BF_Write);
				setInfo.m_Buffers[0].m_BufferHandle = *pConstantsBuffer;
				setInfo.m_Buffers[0].m_Offset = constantsOffset;
				setInfo.m_Buffers[0].m_Size = constantsSize;
			}
			else
				setInfo.m_Buffers.resize(numBuffers);
		}
		else
			setInfo.m_Buffers.resize(numBuffers);

		for (size_t i = 0; i < numBuffers; ++i)
		{
			const size_t index = firstBufferindex + i;
			setInfo.m_Buffers[index].m_BufferHandle = bufferHandles[i];
			setInfo.m_Buffers[index].m_Offset = bufferOffsetsAndSizes[i].first;
			setInfo.m_Buffers[index].m_Size = bufferOffsetsAndSizes[i].second;
		}
		setInfo.m_Layout = layout;
		return pDevice->CreateDescriptorSet(std::move(setInfo));
	}

	void CreateSamplerDescriptorLayoutAndSet(GraphicsDevice* pDevice, size_t numSamplers,
		const std::vector<uint32_t>& bindingIndices, const std::vector<ShaderTypeFlag>& shaderStages,
		std::vector<std::string>&& samplerNames, const std::vector<TextureHandle>& textureHandles,
		DescriptorSetLayoutHandle& layout, DescriptorSetHandle& set)
	{
		DescriptorSetLayoutInfo setLayoutInfo = DescriptorSetLayoutInfo();
		DescriptorSetInfo setInfo = DescriptorSetInfo();
		setLayoutInfo.m_Samplers.resize(numSamplers);
		setLayoutInfo.m_SamplerNames = std::move(samplerNames);
		setInfo.m_Samplers.resize(numSamplers);

		for (size_t i = 0; i < numSamplers; ++i)
		{
			const uint32_t bindingIndex = i >= bindingIndices.size() ? bindingIndices.back() : bindingIndices[i];
			const ShaderTypeFlag shaderStage = i >= shaderStages.size() ? shaderStages.back() : shaderStages[i];
			setLayoutInfo.m_Samplers[i].m_BindingIndex = bindingIndex;
			setLayoutInfo.m_Samplers[i].m_ShaderStages = shaderStage;
			setInfo.m_Samplers[i].m_TextureHandle = textureHandles[i];
		}
		setInfo.m_Layout = layout = pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
		if (numSamplers == 0) return;
		set = pDevice->CreateDescriptorSet(std::move(setInfo));
	}

	DescriptorSetLayoutHandle CreateSamplerDescriptorLayout(GraphicsDevice* pDevice, size_t numSamplers,
		const std::vector<uint32_t>& bindingIndices, const std::vector<ShaderTypeFlag>& shaderStages,
		std::vector<std::string>&& samplerNames)
	{
		DescriptorSetLayoutInfo setLayoutInfo = DescriptorSetLayoutInfo();
		setLayoutInfo.m_Samplers.resize(numSamplers);
		setLayoutInfo.m_SamplerNames = std::move(samplerNames);

		for (size_t i = 0; i < numSamplers; ++i)
		{
			const uint32_t bindingIndex = i >= bindingIndices.size() ? bindingIndices.back() : bindingIndices[i];
			const ShaderTypeFlag shaderStage = i >= shaderStages.size() ? shaderStages.back() : shaderStages[i];
			setLayoutInfo.m_Samplers[i].m_BindingIndex = bindingIndex;
			setLayoutInfo.m_Samplers[i].m_ShaderStages = shaderStage;
		}
		return pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
	}

	DescriptorSetHandle CreateSamplerDescriptorSet(GraphicsDevice* pDevice, size_t numSamplers,
		const std::vector<TextureHandle>& textureHandles, DescriptorSetLayoutHandle layout)
	{
		DescriptorSetInfo setInfo = DescriptorSetInfo();
		setInfo.m_Samplers.resize(numSamplers);

		for (size_t i = 0; i < numSamplers; ++i)
		{
			setInfo.m_Samplers[i].m_TextureHandle = textureHandles[i];
		}
		setInfo.m_Layout = layout;
		return pDevice->CreateDescriptorSet(std::move(setInfo));
	}
}