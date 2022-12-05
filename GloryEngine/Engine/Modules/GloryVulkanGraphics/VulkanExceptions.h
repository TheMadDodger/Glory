#pragma once
#include <exception>
#include <vulkan/vulkan.hpp>

namespace Glory
{
    struct VulkanException : public std::exception
    {
        VulkanException(const vk::Result& error) : m_Error(error) {}

        const char* what() const throw ()
        {
            std::string base = "Vulkan Error Code: " + std::to_string((uint32_t)m_Error);
            return base.c_str();
        }

    private:
        vk::Result m_Error;
    };

    struct CreateVulkanInstanceException : public VulkanException
    {
        CreateVulkanInstanceException(const vk::Result& error) : VulkanException(error) {}

        const char* what() const throw ()
        {
            const char* base = VulkanException::what();
            std::string error = "Could not create a Vulkan instance! " + std::string(base);
            return error.c_str();
        }

    private:
        vk::Result m_Error;
    };
}