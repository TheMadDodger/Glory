#include "GraphicsCommandHandler.h"

namespace Glory
{
	BaseGraphicsCommandHandler::BaseGraphicsCommandHandler()
	{
	}

	BaseGraphicsCommandHandler::~BaseGraphicsCommandHandler()
	{
	}

	const std::type_info& BaseGraphicsCommandHandler::GetCommandType()
	{
		return typeid(void);
	}

	void BaseGraphicsCommandHandler::Invoke(const std::any& commandData) {}
}
