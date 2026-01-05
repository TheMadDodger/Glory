#pragma once
#include <IConsole.h>
#include <Module.h>
#include <Glory.h>
#include <KeyEnums.h>
#include <GraphicsHandles.h>
#include <IWindowInputOverrideHandler.h>

#include <glm/glm.hpp>

namespace Glory
{
	class MaterialData;
	class RenderTexture;
	class MeshData;
	class Console;
	class Window;
	class GraphicsDevice;

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
		virtual void OverlayPass(GraphicsDevice* pDevice, RenderPassHandle renderPass, CommandBufferHandle commandBuffer);

		virtual void OnConsoleClose() override;
		virtual void SetNextColor(const glm::vec4& color) override;
		virtual void ResetNextColor() override;
		virtual void Write(const std::string& line) override;

		virtual void LoadSettings(ModuleSettings& settings) override;

		void HandleKeyboardInput(Window* pWindow, Console& pConsole, KeyboardKey key);

	private:
		std::unique_ptr<MeshData> m_pConsoleMesh;
		MeshHandle m_ConsoleMesh = 0;
		MeshHandle m_ConsoleTextMesh = 0;
		MeshHandle m_InputTextBracketMesh = 0;
		MeshHandle m_InputTextMesh = 0;
		MeshHandle m_CursorTextMesh = 0;
		bool m_ConsoleButtonDown;
		bool m_ConsoleOpen;
		bool m_ConsoleOpenedThisFrame;
		bool m_CursorBlink;
		float m_ConsoleAnimationTime;
		float m_Scroll;
		float m_InputTextWidth;

		std::unique_ptr<MeshData> m_pConsoleLogTextMesh;
		bool m_TextDirty;
		std::unique_ptr<MeshData> m_pInputTextMesh;
		std::unique_ptr<MeshData> m_pInputTextBracketMesh;
		std::unique_ptr<MeshData> m_pInputTextCursorMesh;
		bool m_InputTextDirty;
		static const size_t MAX_CONSOLE_INPUT = 1024;
		size_t m_CursorPos;
		char m_ConsoleInput[MAX_CONSOLE_INPUT];
		int m_HistoryRewindIndex;
		std::string m_BackedUpInput;

		DescriptorSetLayoutHandle m_RenderConstantsSetLayout = 0;
		DescriptorSetLayoutHandle m_TextRenderSetLayout = 0;
		DescriptorSetHandle m_TextRenderSet = 0;
		UUID m_LastFontID = 0;
	};
}
