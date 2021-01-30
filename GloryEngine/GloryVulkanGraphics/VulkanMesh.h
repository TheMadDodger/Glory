#pragma once
#include <Mesh.h>
#include <vulkan/vulkan.hpp>

namespace Glory
{
    class VulkanMesh : public Mesh
    {
    public:
        VulkanMesh(size_t vertexCount, size_t indexCount, InputRate inputRate, size_t binding, size_t stride, const std::vector<AttributeType>& attributeTypes);
        virtual ~VulkanMesh();

        const vk::VertexInputBindingDescription* GetVertexInputBindingDescription() const;
        uint32_t GetVertexInputAttributeDescriptionsCount();
        const vk::VertexInputAttributeDescription* GetVertexInputAttributeDescriptions();

        virtual void CreateBindingAndAttributeData() override;

    private:
        void CreateBindingDescription();
        void CreateVertexInputAttributeDescriptions();
        vk::Format GetFormat(const AttributeType& atributeType);

    private:
        vk::VertexInputBindingDescription m_BindingDescription;
        std::vector<vk::VertexInputAttributeDescription> m_VertexInputAttributeDescriptions;
    };
}