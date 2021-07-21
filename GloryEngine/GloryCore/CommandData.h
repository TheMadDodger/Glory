#pragma once
#include "GraphicsCommands.h"
#include <typeindex>

namespace Glory
{
	struct CommandData
	{
		CommandData(const std::type_index& type, const BaseGraphicsCommand& command) : Type(type), Command(command) {}

		const std::type_index Type;
		BaseGraphicsCommand Command;
	};
}