#include "TumbnailGenerator.h"

namespace Glory::Editor
{
	BaseTumbnailGenerator::BaseTumbnailGenerator()
	{
	}

	BaseTumbnailGenerator::~BaseTumbnailGenerator()
	{
	}

	const std::string& BaseTumbnailGenerator::GetExtension()
	{
		return "";
	}
}