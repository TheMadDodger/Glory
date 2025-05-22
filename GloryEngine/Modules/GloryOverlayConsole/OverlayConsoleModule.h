#pragma once
#include <IConsole.h>
#include <Module.h>
#include <Glory.h>
#include <IWindowInputOverrideHandler.h>

#include <glm/glm.hpp>

namespace Glory
{
	class MaterialData;
	class RenderTexture;
	class MeshData;

	/** @brief Overlay console module */
	class OverlayConsoleModule : public Module, IConsole, IWindowInputOverrideHandler
	{
	public:
		/** @brief Constructor */
		GLORY_API OverlayConsoleModule();
		/** @brief Destructor */
		GLORY_API virtual ~OverlayConsoleModule();

		/** @brief Collect references to assets used by this module */
		GLORY_API virtual void CollectReferences(std::vector<UUID>& references) override;

		/** @brief Module type OverlayConsoleModule */
		GLORY_API virtual const std::type_info& GetModuleType() override;

		/** @brief Redirect input from main window to this module when console is open */
		GLORY_API bool OnOverrideInputEvent(InputEvent& e) override;

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
		float m_Scroll;

		std::vector<std::string> m_Lines;
		std::unique_ptr<MeshData> m_pConsoleLogTextMesh;
		bool m_TextDirty;
		glm::vec4 m_CurrentColor;
	};
}
