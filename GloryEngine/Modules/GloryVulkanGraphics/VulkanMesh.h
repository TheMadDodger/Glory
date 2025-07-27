#pragma once
#include <Mesh.h>
#include <vulkan/vulkan.hpp>

namespace Glory
{
    class VulkanMesh : public Mesh
    {
    public:
        VulkanMesh(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding, uint32_t stride, const std::vector<AttributeType>& attributeTypes);
        VulkanMesh(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding, uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes);
        virtual ~VulkanMesh();

        const vk::VertexInputBindingDescription* GetVertexInputBindingDescription() const;
        uint32_t GetVertexInputAttributeDescriptionsCount();
        const vk::VertexInputAttributeDescription* GetVertexInputAttributeDescriptions();

        virtual void CreateBindingAndAttributeData() override;
        virtual void BindForDraw() override;

    private:
        vk::Format GetFormat(const AttributeType& atributeType);

    private:
        vk::VertexInputBindingDescription m_BindingDescription;
        std::vector<vk::VertexInputAttributeDescription> m_VertexInputAttributeDescriptions;
    };
}