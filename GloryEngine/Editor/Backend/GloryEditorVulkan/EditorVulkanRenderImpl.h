#pragma once
#include "EditorRenderImpl.h"
#include "EditorPlatform.h"
#include "imgui_impl_vulkan.h"

#include <Glory.h>
#include <EditorCreateInfo.h>

#include <vulkan/vulkan.hpp>

namespace Glory
{
    class VulkanDevice;
}

namespace Glory::Editor
{
    extern "C" GLORY_API void LoadBackend(EditorCreateInfo& editorCreateInfo);

    class EditorVulkanRenderImpl : public EditorRenderImpl
    {
    public:
        EditorVulkanRenderImpl();
        virtual ~EditorVulkanRenderImpl();

        virtual void* GetTextureID(TextureHandle texture) override;

    private:
        virtual std::string ShadingLanguage() override;
        virtual void CompileShaderForEditor(const EditorShaderData& editorShader, std::vector<char>& out) override;

        void SetupVulkanWindow(VkInstance instance, ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
        void CreateDescriptorPool(VkDevice device);
        void UploadFonts(ImGui_ImplVulkanH_Window* wd);
        void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
        void FramePresent(ImGui_ImplVulkanH_Window* wd);
        void CleanupVulkanWindow();
        void CleanupVulkan();

        virtual void SetContext(ImGuiContext* pImguiConext) override;
        virtual void Setup() override;
        virtual void SetupBackend() override;
        virtual void UploadImGUIFonts() override;
        virtual void Shutdown() override;
        virtual void Cleanup() override;
        virtual void BeforeRender() override;
        virtual void NewFrame() override;
        virtual void Clear(const ImVec4& clearColor) override;
        virtual void FrameRender(ImDrawData* pDrawData) override;
        virtual void FramePresent() override;

        void* GetTextureID_Internal(TextureHandle texture);

    private:
        VulkanDevice* m_pDevice;
        ImGui_ImplVulkanH_Window m_MainWindow;
        VkDescriptorPool m_DescriptorPool;
        bool m_SwapchainRebuild;
        std::map<UUID, vk::DescriptorSet> m_DesciptorSets;
        std::map<UUID, vk::ImageView> m_ImageViews;

        const int MINIMAGECOUNT = 2;
    };
}
