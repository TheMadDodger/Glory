#pragma once
#include "EditorRenderImpl.h"
#include "EditorPlatform.h"
#include "imgui_impl_vulkan.h"
#include <Device.h>

namespace Glory::Editor
{
    class EditorVulkanRenderImpl : public EditorRenderImpl
    {
    public:
        EditorVulkanRenderImpl();
        virtual ~EditorVulkanRenderImpl();

        virtual void* GetTextureID(Texture* pTexture) override;

    private:
        void SetupVulkanWindow(VkInstance instance, ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
        void CreateDescriptorPool(VkDevice device);
        void UploadFonts(ImGui_ImplVulkanH_Window* wd);
        void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
        void FramePresent(ImGui_ImplVulkanH_Window* wd);
        void CleanupVulkanWindow();
        void CleanupVulkan();

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

    private:
        Glory::Device* m_pDevice;
        ImGui_ImplVulkanH_Window m_MainWindow;
        VkDescriptorPool m_DescriptorPool;
        bool m_SwapChainRebuild;

        const int MINIMAGECOUNT = 2;
    };
}
