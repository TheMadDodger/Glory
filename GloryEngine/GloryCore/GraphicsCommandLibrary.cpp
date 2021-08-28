#include "GraphicsCommandLibrary.h"
#include <string>
#include <algorithm>

namespace Glory
{
	std::map<std::type_index, BaseGraphicsCommandHandler*> GraphicsCommands::m_CommandHandlers;

	void GraphicsCommands::Destroy()
	{
		std::for_each(m_CommandHandlers.begin(), m_CommandHandlers.end(), [](std::pair<std::type_index, BaseGraphicsCommandHandler*> element) { delete element.second; });
		m_CommandHandlers.clear();
	}

	void GraphicsCommands::RunCommand(const std::any& commandData)
	{
		const std::type_info& type = commandData.type();
		auto it = m_CommandHandlers.find(type);
		if (it == m_CommandHandlers.end())
		{
			std::string error = "No command handler found for the graphics command " + std::string(type.name());
			throw new std::exception(error.c_str());
		}
		m_CommandHandlers[type]->Invoke(commandData);
	}

	GraphicsCommands::GraphicsCommands() {}
	GraphicsCommands::~GraphicsCommands() {}
}