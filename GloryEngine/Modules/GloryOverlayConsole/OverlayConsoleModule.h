#pragma once
#include <IConsole.h>
#include <Module.h>
#include <Glory.h>

#include <glm/glm.hpp>

namespace Glory
{
	class MaterialData;
	class RenderTexture;
	class MeshData;

	class OverlayConsoleModule : public Module, IConsole
	{
	public:
		GLORY_API OverlayConsoleModule();
		GLORY_API virtual ~OverlayConsoleModule();

		GLORY_API virtual void CollectReferences(std::vector<UUID>& references) override;

		GLORY_API virtual const std::type_info& GetModuleType() override;

		GLORY_API MaterialData* ConsoleBackgroundMaterial();
		GLORY_API MaterialData* ConsoleTextMaterial();

		GLORY_MODULE_VERSION_H(0,1,0);

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Update() override;
		virtual void Cleanup() override;
		virtual void OverlayPass();

		virtual void OnConsoleClose() override;
		virtual void SetNextColor(const glm::vec4& color) override;
		virtual void ResetNextColor() override;
		virtual void Write(const std::string& line) override;

		virtual void LoadSettings(ModuleSettings& settings) override;

	private:
		MaterialData* m_pConsoleBackgroundMaterial = nullptr;
		MaterialData* m_pConsoleTextMaterial = nullptr;

		std::unique_ptr<MeshData> m_pConsoleMesh;
		bool m_ConsoleButtonDown;
		bool m_ConsoleOpen;
		float m_ConsoleAnimationTime;
	};
}
