#pragma once
#include <map>
#include "GraphicsCommandHandler.h"
#include "Commands.h"
#include "CommandData.h"
#include <typeindex>

namespace Glory
{
	class GraphicsCommands
	{
	public:

		template<class T>
		static void RegisterCommandHandler()
		{
			BaseGraphicsCommandHandler handler = (BaseGraphicsCommandHandler)(T());
			m_CommandHandlers.emplace(handler.GetCommandType(), handler);
		}

		static void RunCommand(const CommandData& commandData);

	private:
		GraphicsCommands();
		virtual ~GraphicsCommands();

	private:
		//static std::map<const std::type_info, BaseGraphicsCommandHandler*> m_CommandHandlers;
		static std::map<std::type_index, BaseGraphicsCommandHandler*> m_CommandHandlers;
	};
}
