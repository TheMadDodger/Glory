#pragma once
#include <Engine.h>
#include <string>
#include <filesystem>
#include <windows.h>
#include <RendererModule.h>

namespace Glory
{
    class NullRenderer : public RendererModule
    {
    public:
        NullRenderer() {}
        virtual ~NullRenderer() {}
    
    protected:
        friend class GraphicsThread;

		virtual void Initialize() override {}
		virtual void Cleanup() override {}
		virtual void OnRender(CameraRef camera, const RenderData& renderData, const std::vector<PointLight>& lights = std::vector<PointLight>()) override {}
		virtual void OnDoScreenRender(CameraRef camera, const std::vector<PointLight>& lights, size_t width, size_t height, RenderTexture* pRenderTexture) override {}

		virtual void OnStartCameraRender(CameraRef camera, const std::vector<PointLight>& lights) override {}
		virtual void OnEndCameraRender(CameraRef camera, const std::vector<PointLight>& lights) override {}
    };

	class EngineLoader
	{
	public:
		EngineLoader(const std::filesystem::path& cfgPath);
		virtual ~EngineLoader();

		Engine* LoadEngine(const Glory::WindowCreateInfo& defaultWindow);
		void Unload();

	private:
		void LoadModules(YAML::Node& modules);
		void LoadModule(const std::string& moduleName);
		void PopulateEngineInfo(YAML::Node& engineInfo, EngineCreateInfo& engineCreateInfo, const Glory::WindowCreateInfo& defaultWindow);

		template<class T>
		T* LoadRequiredModule(YAML::Node& node, const std::string& key, T** value)
		{
			YAML::Node indexNode = node[key];
			int index = indexNode.as<int>();
			Module* pModule = m_pModules[index];
			T* pT = (T*)pModule;
			*value = pT;
			return pT;
		}

	private:
		std::filesystem::path m_CFGPath;
		std::vector<Module*> m_pModules;
		std::vector<HMODULE> m_Libs;
		std::vector<Module*> m_pOptionalModules;
	};
}
