#pragma once
#include <IConsole.h>
#include <Module.h>
#include <Glory.h>
#include <KeyEnums.h>
#include <IWindowInputOverrideHandler.h>

#include <glm/glm.hpp>

namespace Glory
{
	class MaterialData;
	class RenderTexture;
	class MeshData;
	class Console;
	class Window;

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
		GLORY_API bool OnOverrideTextEvent(TextEvent& e) override;

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

		void HandleKeyboardInput(Window* pWindow, Console& pConsole, KeyboardKey key);

	private:
		MaterialData* m_pConsoleBackgroundMaterial = nullptr;
		MaterialData* m_pConsoleTextMaterial = nullptr;

		std::unique_ptr<MeshData> m_pConsoleMesh;
		bool m_ConsoleButtonDown;
		bool m_ConsoleOpen;
		bool m_ConsoleOpenedThisFrame;
		float m_ConsoleAnimationTime;
		float m_Scroll;

		std::unique_ptr<MeshData> m_pConsoleLogTextMesh;
		bool m_TextDirty;
		std::unique_ptr<MeshData> m_pInputTextMesh;
		std::unique_ptr<MeshData> m_pInputTextBracketMesh;
		std::unique_ptr<MeshData> m_pInputTextCursorMesh;
		bool m_InputTextDirty;
		static const size_t MAX_CONSOLE_INPUT = 1024;
		size_t m_CursorPos;
		char m_ConsoleInput[MAX_CONSOLE_INPUT];
	};
}
