#pragma once
#include <map>
#include "GraphicsCommandHandler.h"
#include "CommandData.h"
#include <typeindex>
#include "GraphicsModule.h"

namespace Glory
{
	class GraphicsCommands
	{
	public:
		template<class T>
		static void RegisterCommandHandler(GraphicsModule* pModule)
		{
			BaseGraphicsCommandHandler* handler = (BaseGraphicsCommandHandler*)(new T(pModule));
			m_CommandHandlers.emplace(handler->GetCommandType(), handler);
		}

		static void Destroy();
		static void RunCommand(const std::any& commandData);

	private:
		GraphicsCommands();
		virtual ~GraphicsCommands();

	private:
		static std::map<std::type_index, BaseGraphicsCommandHandler*> m_CommandHandlers;
	};
}
