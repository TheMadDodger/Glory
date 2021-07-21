#include "GraphicsCommandLibrary.h"
#include <string>

namespace Glory
{
	std::map<std::type_index, BaseGraphicsCommandHandler*> GraphicsCommands::m_CommandHandlers;

	void GraphicsCommands::RunCommand(const CommandData& commandData)
	{
		auto it = m_CommandHandlers.find(commandData.Type);
		if (it == m_CommandHandlers.end())
		{
			std::string error = "No command handler found for the graphics command " + std::string(commandData.Type.name());
			throw new std::exception(error.c_str());
		}
		m_CommandHandlers[commandData.Type]->Invoke(&commandData.Command);
	}

	GraphicsCommands::GraphicsCommands() {}
	GraphicsCommands::~GraphicsCommands() {}
}