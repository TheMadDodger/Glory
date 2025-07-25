#include "VulkanMesh.h"

namespace Glory
{
    VulkanMesh::VulkanMesh(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding, uint32_t stride, const std::vector<AttributeType>& attributeTypes):
        Mesh(vertexCount, indexCount, inputRate, binding, stride, attributeTypes)
    {
    }

    VulkanMesh::VulkanMesh(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding,
        uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes):
        Mesh(vertexCount, indexCount, inputRate, binding, stride, primitiveType, attributeTypes)
    {
    }

    VulkanMesh::~VulkanMesh()
	{
	}

    const vk::VertexInputBindingDescription* VulkanMesh::GetVertexInputBindingDescription() const
    {
        return &m_BindingDescription;
    }

    uint32_t VulkanMesh::GetVertexInputAttributeDescriptionsCount()
    {
        return m_VertexInputAttributeDescriptions.size();
    }

    const vk::VertexInputAttributeDescription* VulkanMesh::GetVertexInputAttributeDescriptions()
    {
        return m_VertexInputAttributeDescriptions.data();
    }

    void VulkanMesh::CreateBindingAndAttributeData()
    {
        // Vertex binding and attributes
        m_BindingDescription = vk::VertexInputBindingDescription();
        m_BindingDescription.binding = m_Binding;
        m_BindingDescription.stride = m_Stride;

        switch (m_InputRate)
        {
        case InputRate::Vertex:
            m_BindingDescription.inputRate = vk::VertexInputRate::eVertex;
            break;

        case InputRate::Instance:
            m_BindingDescription.inputRate = vk::VertexInputRate::eInstance;
            break;
        default:
            break;
        }

        m_VertexInputAttributeDescriptions.resize(m_AttributeTypes.size());
        uint32_t currentOffset = 0;
        for (size_t i = 0; i < m_VertexInputAttributeDescriptions.size(); i++)
        {
            m_VertexInputAttributeDescriptions[i].binding = m_Binding;
            m_VertexInputAttributeDescriptions[i].location = i;
            m_VertexInputAttributeDescriptions[i].format = GetFormat(m_AttributeTypes[i]);
            m_VertexInputAttributeDescriptions[i].offset = currentOffset;
            GetNextOffset(m_AttributeTypes[i], currentOffset);
        }
    }

    void VulkanMesh::BindForDraw()
    {
        throw new std::exception("VulkanMesh::BindForDraw() not yet implemented!");
    }

    vk::Format VulkanMesh::GetFormat(const AttributeType& atributeType)
    {
        vk::Format format;

        switch (atributeType)
        {
        case AttributeType::Float:
            format = vk::Format::eR32Sfloat;
            break;
        case AttributeType::Float2:
            format = vk::Format::eR32G32Sfloat;
            break;
        case AttributeType::Float3:
            format = vk::Format::eR32G32B32Sfloat;
            break;
        case AttributeType::Float4:
            format = vk::Format::eR32G32B32A32Sfloat;
            break;
        case AttributeType::UINT:
            format = vk::Format::eR32Uint;
            break;
        case AttributeType::UINT2:
            format = vk::Format::eR32G32Uint;
            break;
        case AttributeType::UINT3:
            format = vk::Format::eR32G32B32Uint;
            break;
        case AttributeType::UINT4:
            format = vk::Format::eR32G32B32A32Uint;
            break;
        case AttributeType::SINT:
            format = vk::Format::eR32Sint;
            break;
        case AttributeType::SINT2:
            format = vk::Format::eR32G32Sint;
            break;
        case AttributeType::SINT3:
            format = vk::Format::eR32G32B32Sint;
            break;
        case AttributeType::SINT4:
            format = vk::Format::eR32G32B32A32Sint;
            break;
        }

        return format;
    }
}