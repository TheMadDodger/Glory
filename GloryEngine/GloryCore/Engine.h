#pragma once
#include "Object.h"
#include "WindowModule.h"
#include "GraphicsModule.h"

namespace Glory
{
	struct EngineCreateInfo
	{
		WindowModule* pWindowModule;
		GraphicsModule* pGraphicsModule;

		uint32_t OptionalModuleCount;
		Module** pOptionalModules;
	};

	/// <summary>
	/// This class describes the engine a GloryGame object will run on.
	/// It holds the required modules used to run the game, as well as optional modules.
	/// </summary>
	class Engine : public Object
	{
	public:
		static Engine* CreateEngine(const EngineCreateInfo& createInfo);

		WindowModule* GetWindowModule();
		GraphicsModule* GetGraphicsModule();

	private:
		Engine(const EngineCreateInfo& createInfo);
		virtual ~Engine();

	private:
		void Initialize();
		void Update();
		void Draw();

	private:
		friend class Game;

		// Required modules
		WindowModule* m_pWindowModule;
		GraphicsModule* m_pGraphicsModule;

		// Optional modules
		std::vector<Module*> m_pOptionalModules;

		// All modules
		std::vector<Module*> m_pAllModules;
	};
}
