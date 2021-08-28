#pragma once
#include "GraphicsCommands.h"
#include <typeindex>
#include <any>

namespace Glory
{
	struct CommandData
	{
		CommandData(const std::type_index& type, std::any command) : Type(type), Command(command) {}

		const std::type_index Type;
		const std::any Command;
	};
}